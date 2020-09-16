/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios 	savetty;

void	term_initio()
{
	struct termios tty;

	tcgetattr(0, &savetty);
	tcgetattr(0, &tty);

	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;

	tcsetattr(0, TCSADRAIN, &tty);
}

void	term_exitio()
{
	tcsetattr(0, TCSADRAIN, &savetty);
}

void	term_puts(char *s)
{
	fprintf(stderr,"%s",s);
}

void	term_putchar(char c)
{
	putc(c,stderr);
}

int	term_getchar_nb()
{
        static unsigned char 	line [2];

        if (read(0,line,1)) // note: destructive read
				printf("%c \n", (char) line[0]);
        		return (int) line[0];

        return -1;
}

int	term_getchar()
{
        int    c;

        while ((c = term_getchar_nb()) == -1)
                ;
        return c;
}

/*------------------------------------------------------------
 * Serial I/O
 * 8 bits, 1 stopbit, no parity,
 * 115,200 baud
 *------------------------------------------------------------
 */
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

int serial_device = 0;
int fd_RS232;

void rs232_open(void)
{
  	char 		*name;
  	int 		result;
  	struct termios	tty;

       	fd_RS232 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);  // Hardcode your serial port here, or request it as an argument at runtime

	assert(fd_RS232>=0);

  	result = isatty(fd_RS232);
  	assert(result == 1);

  	name = ttyname(fd_RS232);
  	assert(name != 0);

  	result = tcgetattr(fd_RS232, &tty);
	assert(result == 0);

	tty.c_iflag = IGNBRK; /* ignore break condition */
	tty.c_oflag = 0;
	tty.c_lflag = 0;

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
	tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */

	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 1; // added timeout

	tty.c_iflag &= ~(IXON|IXOFF|IXANY);

	result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */

	tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}


void 	rs232_close(void)
{
  	int 	result;

  	result = close(fd_RS232);
  	assert (result==0);
}


int	rs232_getchar_nb()
{
	int 		result;
	unsigned char 	c;

	result = read(fd_RS232, &c, 1);

	if (result == 0)
		return -1;

	else
	{
		assert(result == 1);
		return (int) c;
	}
}


int 	rs232_getchar()
{
	int 	c;

	while ((c = rs232_getchar_nb()) == -1)
		;
	return c;
}


int 	rs232_putchar(char c)
{
	int result;

	do {
		result = (int) write(fd_RS232, &c, 1);
	} while (result == 0);

	assert(result == 1);
	return result;
}

/*------------------------------------------------------------
 * joystick I/O
 *------------------------------------------------------------
 */
#include "joystick.h"
#define JS_DEV	"/dev/input/js0"

int	axis[6];
int	button[12];

void get_joystick(int fd)
{
	struct js_event js;
	while (read(fd, &js, sizeof(struct js_event)) == 
	       			sizeof(struct js_event))  {

		/* register data
		 */
		 fprintf(stderr,".");
		fprintf(stderr,"%6d ",js.number);
		switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
		}
		//for (int i = 0; i < 6; i++) {
		//	fprintf(stderr,"%6d ",axis[i]);
		//}
	}
	return;
}

/*------------------------------------------------------------
 * control variables
 *------------------------------------------------------------
 */
#define SAFE		0
#define PANIC		1
#define MANUAL		2
#define CALIBRATION	3
#define YAW		4
#define FULL		5
#define RAW		6

int8_t k_throttle = 0, k_roll = 0, k_pitch = 0, k_yaw = 0;
int8_t j_throttle = 0, j_roll = 0, j_pitch = 0, j_yaw = 0;
uint8_t mode = 0;

typedef struct
{
	uint8_t mode;
	uint8_t throttle;
	int8_t roll, pitch, yaw;
	uint8_t CRC;
	//....
}command;

void send_command(command c)
{
	rs232_putchar(0xFF);
	rs232_putchar(c.mode);
	rs232_putchar(c.throttle);
	rs232_putchar(c.roll);
	rs232_putchar(c.pitch);
	rs232_putchar(c.yaw);
	rs232_putchar(c.CRC);
}

/*------------------------------------------------------------
 * time
 *------------------------------------------------------------
 */

#include <time.h>
#include <assert.h>
unsigned int    mon_time_ms(void)
{
        unsigned int    ms;
        struct timeval  tv;
        struct timezone tz;

        gettimeofday(&tv, &tz);
        ms = 1000 * (tv.tv_sec % 65); // 65 sec wrap around
        ms = ms + tv.tv_usec / 1000;
        return ms;
}

void    mon_delay_ms(unsigned int ms)
{
        struct timespec req, rem;

        req.tv_sec = ms / 1000;
        req.tv_nsec = 1000000 * (ms % 1000);
        assert(nanosleep(&req,&rem) == 0);
}

/*----------------------------------------------------------------
 * CRC calculation
 *----------------------------------------------------------------
 */
#include "crc.h"
//verifies if correct values have been sent
uint8_t get_crc(uint8_t crc, void const *msg, uint8_t bufferSize)
{
	uint8_t const *buffer = msg;

	if (buffer == NULL) return 0xff;

	crc &= 0xff;
	printf("null \n");

	for (int i = 0; i < bufferSize; i++)
	{
		printf("crc_now %d buffer_i %d \n", crc, buffer[i]);
		crc = crc8_table[ crc ^ buffer[i] ];
		
	}

	return crc;
}


/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
#include <stdbool.h>

int main(int argc, char **argv)
{
	int	c;
	//int	fd;
	//struct js_event js;

	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

//	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
//		perror("jstest");
//		exit(1);
//	}

//	fcntl(fd, F_SETFL, O_NONBLOCK);

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	/* send & receive
	 */
	for (;;)
	{
		//get_joystick(fd);
		//if ((c = term_getchar_nb()) != -1)
		//	rs232_putchar(c);

		//if ((c = rs232_getchar_nb()) != -1)
		//	term_putchar(c);
		mon_delay_ms(300);
		if ((c = term_getchar_nb()) != -1)//still need to limit the values
		{
			switch (c)
			{
				case '0':
					mode = SAFE;
					break;
				case '1':
					mode = PANIC;
					break;
				case '2':
					mode = MANUAL;
					break;
				case '3':
					mode = CALIBRATION;
					break;
				case '4':
					mode = YAW;
					break;
				case '5':
					mode = FULL;
					break;
				case '6':
					mode = RAW;
					break;
			}
			//if (mode == SAFE) 
			//{
			//	printf("safe mode!\n");
			//	if(c == 27)
			//	{
			//		demo_done = true;
			//		break;
			//	}
			//	continue;
			//}
			switch (c)
			{
				//still need to limit the values
				case 'r'://lift up
					k_throttle++;
					break;
				case 'f'://lift down
					k_throttle--;
					if (k_throttle < 0) k_throttle = 0;
					break;
				case 'a'://left, roll up
					k_roll++;
					break;
				case 'd'://right, roll down
					k_roll--;
					break;
				case 'w'://up, pitch down
					k_pitch--;
					break;
				case 's'://down, pitch up
					k_pitch++;
					break;
				case 'q'://yaw down
					k_yaw--;
					break;
				case 'e'://yaw up
					k_yaw++;
					break;
			}
		}

		//fprintf(stderr,"mode = %d\n",mode);
		//fprintf(stderr,"from keyboard: throttle = %d roll = %d pitch = %d yaw = %d\n",k_throttle, k_roll, k_pitch, k_yaw);
		//fprintf(stderr,"from joystick: throttle = %d roll = %d pitch = %d yaw = %d\n",j_throttle, j_roll, j_pitch, j_yaw);
		int8_t CRC=0;
		command Command = {mode,k_throttle+j_throttle,k_roll+j_roll,k_pitch+j_roll,k_yaw+j_yaw, CRC};

		
		while((c = rs232_getchar_nb()) != -1)
			term_putchar(c); 
		//char data[8] = [Command.mode, Command.throttle, Command.roll, Command.pitch, Command.yaw]
		const void *val[3];
		val[0] = 1;
		val[1] = 2;
		val[2] = 1;


		//int crc = get_crc(255, &val, 7);
		int check = get_crc(0, &val, 7);
		printf("crc %d val %d\n",check, val);
		Command.CRC = check;
		printf("command ");
		printf("%d ", Command.mode);
		printf("%d ", Command.throttle);
		printf("%d ", Command.roll);
		printf("%d ", Command.pitch);
		printf("%d ", Command.yaw);
		printf("%d \n", Command.CRC);
		send_command(Command);
	}


	//...
	//packetize data and send via rs232

	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

