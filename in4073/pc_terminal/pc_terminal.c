/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "math.h"

#define LOG_SIZE 29
int count1 = 0;
/*-------------------------
-----------------------------------
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
 * time
 *------------------------------------------------------------
 */
unsigned int previous_time;//last checking time
unsigned int interval=3000;//sending interval

#include <time.h>
#include <sys/time.h>
#include <assert.h>
unsigned int mon_time_ms(void)
{
        unsigned int    ms;
        struct timeval  tv;
        struct timezone tz;

        gettimeofday(&tv, &tz);
        ms = 1000 * (tv.tv_sec % 65); // 65 sec wrap around
        ms = ms + tv.tv_usec / 1000;
        return ms;
}

void mon_delay_ms(unsigned int ms)
{
        struct timespec req, rem;

        req.tv_sec = ms / 1000;
        req.tv_nsec = 1000000 * (ms % 1000);



        assert(nanosleep(&req,&rem) == 0);
}

bool sending_timer()//send command periodically to check the connection
{
	unsigned int current_time = mon_time_ms();
	if(current_time-previous_time > interval)
	{
		previous_time = current_time;
		return true;
	}
	return false;
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
#define EXIT		7

int16_t k_throttle = 0, k_roll = 0, k_pitch = 0, k_yaw = 0;
int16_t j_throttle = 0, j_roll = 0, j_pitch = 0, j_yaw = 0;
int16_t throttle = 0, roll = 0, pitch = 0, yaw = 0;
uint8_t mode = 0;
uint8_t frame = 0;
int last_sending_time;

typedef struct
{
	uint8_t frame;
	uint8_t mode;
	uint8_t throttle;
	int8_t roll, pitch, yaw;
	//....
}command;

/*------------------------------------------------------------
 * joystick and keyboard I/O
 *------------------------------------------------------------
 */
#include "joystick.h"
#define JS_DEV	"/dev/input/js0"

int	axis[6];
int	button[12];

bool get_joystick(int fd)
{
	struct js_event js;
	bool get_values = false;
	while (read(fd, &js, sizeof(struct js_event)) == 
	       			sizeof(struct js_event))  {

		/* register data
		 */
		//fprintf(stderr,".");
		//fprintf(stderr,"%6d ",js.number);
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
		//fprintf(stderr,"\n");
		get_values = true;
	}
	//if(change large enough)
	//{
	//	assign values
	//	return true;
	//}
	if(get_values && mode != SAFE)
	{
		j_throttle = (-axis[3]+32767)/256; 
		j_roll = axis[0]/256; 
		j_pitch = axis[1]/256; 
		j_yaw = axis[2]/256;
		return true;
	}

	return false;
}

bool get_keyboard()
{
	int c;
	if ((c = term_getchar_nb()) != -1)//still need to limit the values
	{
		switch (c)
		{
			case '0':
				mode = SAFE;
				return true;
			case '1':
				mode = PANIC;
				return true;
			case '2':
				mode = MANUAL;
				return true;
			case '3':
				mode = CALIBRATION;
				return true;
			case '4':
				mode = YAW;
				return true;
			case '5':
				mode = FULL;
				return true;
			case '6':
				mode = RAW;
				return true;
			case '7':
				mode = EXIT;
				return true;
		}
		if (mode == SAFE) 
		{
			printf("Invalid command! It is safe mode.\n");
			return false;
		}
		switch (c)
		{
			//still need to limit the values
			case 'r'://lift up
				k_throttle+=5;
				return true;
			case 'f'://lift down
				k_throttle-=5;
			if (k_throttle < 0) k_throttle = 0;
				return true;
			case 'a'://left, roll up
				k_roll+=5;
				return true;
			case 'd'://right, roll down
				k_roll-=5;
				return true;
			case 'w'://up, pitch down
				k_pitch-=5;
				return true;
			case 's'://down, pitch up
				k_pitch+=5;
				return true;
			case 'q'://yaw down
				k_yaw-=5;
				return true;
			case 'e'://yaw up
				k_yaw+=5;
				return true;
		}
	}
	return false;
}


/*------------------------------------------------------------
 * send commands and receive ackonwledgements
 *------------------------------------------------------------
 */

int miss_count = 0;//count the successive miss of ack
bool check_ack = false; //need to check ack or not
int t_threshold = 500; 
void send_command(command c)
{
	rs232_putchar(0xFF);
	rs232_putchar(c.frame);
	rs232_putchar(c.mode);
	rs232_putchar(c.throttle);
	rs232_putchar(c.roll);
	rs232_putchar(c.pitch);
	rs232_putchar(c.yaw);
	last_sending_time = mon_time_ms();
	frame++;	
	check_ack = true;
}

int get_ack()
{
	if (check_ack == false) return 0;//need to check ack or not
	int c = rs232_getchar_nb(),last_c = -1;
	
	while(c != -1)
	{
		last_c = c;//the last meaningful element returned by rs232_getchar
		c = rs232_getchar_nb();
	}
	if(last_c == frame-1)//if it is the ack for the last frame
	{
		miss_count = 0;
		check_ack = false;
		fprintf(stderr,"get ack %d\n",last_c);
		return 0;
	}
	int ms = mon_time_ms();
	if(ms > last_sending_time + t_threshold)//not get ack in limited time
	{
		fprintf(stderr,"frame %u, ack time out\n", frame);
		miss_count++;
		check_ack = false;
		return 1;
	}
	fprintf(stderr,"nothing");//not get ack but also not reach the limited time
	return 2;
}



/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */


int main(int argc, char **argv)
{
	int	c;
	int	fd;
	//struct js_event js;
        FILE *fp; 
        fp = fopen("log.txt", "w");
	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}

	fcntl(fd, F_SETFL, O_NONBLOCK);

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	/* send & receive
	 */
	mon_delay_ms(1000);
	while((c = rs232_getchar_nb()) != -1)
		term_putchar(c);

	command C = {SAFE,0,0,0,0,0};
	send_command(C); //send the first command
	while (1)
	{
		bool send = false;
		//send = get_ack()==1;
		send = send || get_joystick(fd);
		send = send || get_keyboard();
		//send = send || sending_timer();

		//mon_delay_ms(300);
		if(miss_count>5) mode = PANIC;
		if(mode == PANIC || mode == EXIT) break;
		//fprintf(stderr,"mode = %d\n",mode);
		int ae[4];
		if (send)
		{
			throttle = j_throttle+k_throttle;
			roll = j_roll + k_roll;
			pitch = j_pitch + k_pitch;
			yaw = j_yaw + k_yaw;
			if(throttle > 255) throttle = 255;
			if(roll>127) roll = 127;
			if(roll<-128) roll = -128;
			if(pitch>127) pitch = 127;
			if(pitch<-128) pitch = -128;
			if(yaw>127) yaw = 127;
			if(yaw<-128) yaw = -128;

			command Command = {frame, mode,throttle,roll,pitch,yaw};
			fprintf(stderr,"mode = %d\n",mode);
			fprintf(stderr,"from keyboard: throttle = %u roll = %d pitch = %d yaw = %d\n",k_throttle, k_roll, k_pitch, k_yaw);
			fprintf(stderr,"from joystick: throttle = %u roll = %d pitch = %d yaw = %d\n",j_throttle, j_roll, j_pitch, j_yaw);
			send_command(Command);
		}
		while((c = rs232_getchar_nb()) != -1)
			term_putchar(c);
		//get_ack();
	}
	//send PANIC or EXIT until get ack
	C.mode = mode;
	C.frame = frame;
	send_command(C);
        
        if(C.mode == EXIT)
        {
              while(c = (rs232_getchar() != 0x7F));
              while(1)
              {    
                     c = rs232_getchar(); 
                     if(c == 0x7F) break;
                     fprintf(fp, "%d ", c);
                     count1++;
                     if(count1 == LOG_SIZE) 
                     {
                            fprintf(fp, "\n");
                            count1 = 0;
                     }
               }

        }	
	//while(get_ack()!=0)
	//{
	//	send_command(C);
	//	mon_delay_ms(t_threshold);
	//}
        fclose(fp);
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

