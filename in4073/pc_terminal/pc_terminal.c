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

#define LOG_SIZE 34
int count1 = 0;
int row = 0;
int data[0xFFFF];
int j;

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
<<<<<<< HEAD
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
=======
 * time
 *------------------------------------------------------------
 */
unsigned int previous_time;//last checking time
unsigned int interval=1000;//sending interval

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

bool send_period()//send command periodically to check the connection
{
	unsigned int current_time = mon_time_ms();
	if(current_time-previous_time > interval)
	{
		previous_time = current_time;
		return true;
	}
	return false;
>>>>>>> Liang
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
<<<<<<< HEAD

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
uint8_t get_crc(uint8_t crc, uint8_t *msg, uint8_t bufferSize)
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

=======
#define EXIT		7

uint16_t k_throttle,j_throttle,throttle;
int16_t k_roll = 0, k_pitch = 0, k_yaw = 0;
int16_t j_roll = 0, j_pitch = 0, j_yaw = 0;
int16_t roll = 0, pitch = 0, yaw = 0;
uint8_t P, P1, P2 = 0;
uint8_t mode = 0;
uint8_t frame = 0;
int last_sending_time;

typedef struct
{
	uint8_t frame;
	uint8_t mode;
	uint16_t throttle;
	int16_t roll, pitch, yaw;
        uint8_t P, P1, P2;
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
		//for (int i = 0; i < 12; i++) {
		//	fprintf(stderr,"%6d ",button[i]);
		//}
		//fprintf(stderr,"\n");
		get_values = true;
	}
	//if(change large enough)
	//{
	//	assign values
	//	return true;
	//}
	if(get_values)
	{
		j_throttle = (-axis[3]+32767); 
		j_roll = axis[0]; 
		j_pitch = axis[1]; 
		j_yaw = axis[2];
		if(button[0]==1) mode = EXIT;
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
				if(mode!=SAFE)
				{
					printf("Invalid command! Please go back to safe mode before switching mode\n");
					return false;
					
				}
				else if(j_throttle!=0||j_roll!=0||j_pitch!=0||j_yaw!=0)
				{
					printf("Invalid command! Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = MANUAL;
					return true;
				}
			case '3':
				if(mode!=SAFE)
				{
					printf("Invalid command! Please go back to safe mode before switching mode\n");
					return false;
				}
				else
				{
					mode = CALIBRATION;
					return true;
				}
			case '4':
				if(mode!=SAFE)
				{
					printf("Invalid command! Please go back to safe mode before switching mode\n");
					return false;
				}
				else if(j_throttle!=0||j_roll!=0||j_pitch!=0||j_yaw!=0)
				{
					printf("Invalid command! Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = YAW;
					return true;
				}
			case '5':
				if(mode!=SAFE)
				{
					printf("Invalid command! Please go back to safe mode before switching mode\n");
					return false;
				}
				else if(j_throttle!=0||j_roll!=0||j_pitch!=0||j_yaw!=0)
				{
					printf("Invalid command! Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = FULL;
					return true;
				}
			case '6':
				if(mode!=SAFE)
				{
					printf("Invalid command! Please go back to safe mode before switching mode\n");
					return false;
				}
				else if(j_throttle!=0||j_roll!=0||j_pitch!=0||j_yaw!=0)
				{
					printf("Invalid command! Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = RAW;
					return true;
				}
			case '7':
				if(mode!=SAFE)
				{
					printf("Invalid command! Please go back to safe mode before switching mode\n");
					return false;
				}
				else
				{
					mode = EXIT;
					return true;
				}
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
				k_throttle+=1000;
				return true;
			case 'f'://lift down
				k_throttle-=1000;
				return true;
			case 'a'://left, roll up
				k_roll+=1000;
				return true;
			case 'd'://right, roll down
				k_roll-=1000;
				return true;
			case 'w'://up, pitch down
				k_pitch-=1000;
				return true;
			case 's'://down, pitch up
				k_pitch+=1000;
				return true;
			case 'q'://yaw down
				k_yaw-=1000;
				return true;
			case 'e'://yaw up
				k_yaw+=1000;
				return true;
			case 'u'://yaw up
				P+=1;
				return true;
			case 'j'://yaw up
				P-=1;
                              if (P < 0) P = 0;
				return true;
			case 'i'://yaw up
				P1+=1;
				return true;
			case 'k'://yaw up
				P1-=1;
				if (P1 < 0) P1 = 0;
				return true;
			case 'o'://yaw up
				P2+=1;
				return true;
			case 'l'://yaw up
				P2-=1;
				if (P2 < 0) P2 = 0;
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
bool waiting_for_ack = false; //need to check whether reciveing ack or not
int t_threshold = 500; 
void send_command(command c)
{
	rs232_putchar(0xFF);
	rs232_putchar(c.frame);
	rs232_putchar(c.mode);
	rs232_putchar(c.throttle>>8);
	rs232_putchar(c.throttle);
	rs232_putchar(c.roll>>8);
	rs232_putchar(c.roll);
	rs232_putchar(c.pitch>>8);
	rs232_putchar(c.pitch);
	rs232_putchar(c.yaw>>8);
	rs232_putchar(c.yaw);
	rs232_putchar(c.P);
	rs232_putchar(c.P1);
	rs232_putchar(c.P2);
	last_sending_time = mon_time_ms();
	frame++;	
	waiting_for_ack = true;
}


void get_data()
{
	int c;
	while((c = rs232_getchar_nb()) != -1)
	{
		if(c == 255)
		{
			int next_c = rs232_getchar_nb();
			if(next_c == frame-1)//if it is the ack for the last frame
			{
				miss_count = 0;
				waiting_for_ack = false;
				if(mode == PANIC || mode == CALIBRATION) mode = SAFE;
				//4fprintf(stderr,"get ack %d\n",next_c);
			}	
		}
		else
		{
			term_putchar(c);
		}
	}
}

bool resend()
{
	if(waiting_for_ack)
	{
		int ms = mon_time_ms();
		if(ms > last_sending_time + t_threshold)//not get ack in limited time
		{
			fprintf(stderr,"frame %u, ack time out\n", frame);
			miss_count++;
			waiting_for_ack = false;
			return true;
		}
	}
	return false;
}
/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
<<<<<<< HEAD
#include <stdbool.h>
=======

>>>>>>> Liang

int main(int argc, char **argv)
{
	int	c;
<<<<<<< HEAD
	//int	fd;
	//struct js_event js;

=======
	int	fd;
	//struct js_event js;
        uint32_t time;
        int16_t log_data;


	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

<<<<<<< HEAD
//	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
//		perror("jstest");
//		exit(1);
//	}

//	fcntl(fd, F_SETFL, O_NONBLOCK);
=======
	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}

	fcntl(fd, F_SETFL, O_NONBLOCK);
>>>>>>> Liang

        FILE *fp; 
        fp = fopen("log.txt", "r+");
        FILE *fp_parse;
        fp_parse = fopen("log_parse.txt", "w");         

        fprintf(fp, "TIME \t THROTTLE \t ROLL \t PITCH \t YAW \t MODE \t PHI \t THETA \t PSI \t SP \t SQ \t SR \t MOTOR 0 \t MOTOR 1 \t MOTOR 2 \t MOTOR 3 \t LOOP TIME\n");

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);

	/* send & receive
	 */
<<<<<<< HEAD
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
		uint8_t val[7];
		val[0] = Command.mode;
		val[1] = Command.throttle;
		val[2] = Command.roll;
		val[3] = Command.yaw;
		val[4] = Command.frame;


		//int crc = get_crc(255, &val, 7);
		int check = get_crc(0, val, 5);
		printf("crc %u val %d\n",check, val[0]);
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

=======
	mon_delay_ms(1000);
	while((c = rs232_getchar_nb()) != -1)
		term_putchar(c);
	get_joystick(fd);
	while(j_throttle!=0||j_yaw!=0||j_pitch!=0||j_roll!=0)
	{
		fprintf(stderr,"please set joystick to neutral\n");
		mon_delay_ms(1000);
		get_joystick(fd);
	}
	command C = {SAFE,0,0,0,0,0};
	send_command(C); //send the first command
	while (1)
	{
		get_data();
		bool send = resend();
		send = send || get_joystick(fd);
		send = send || get_keyboard();
		send = send || send_period();

		//mon_delay_ms(300);
		if(miss_count>5) mode = PANIC;
		if( mode == EXIT) break;
		//fprintf(stderr,"mode = %d\n",mode);
		int ae[4];
		if (send)
		{
			throttle = j_throttle+k_throttle;
			roll = j_roll + k_roll;
			pitch = j_pitch + k_pitch;
			yaw = j_yaw + k_yaw;
			if(throttle > 65535) throttle = 65535;
			if(throttle < 0) throttle = 0;
			if(roll>32767) roll = 32767;
			if(roll<-32768) roll = -32768;
			if(pitch>32767) pitch = 32767;
			if(pitch<-32768) pitch = -32768;
			if(yaw>32767) yaw = 32767;
			if(yaw<-32768) yaw = -32768;

			command Command = {frame, mode,throttle,roll,pitch,yaw,P,P1,P2};
			//fprintf(stderr,"mode = %d\n",mode);
			//fprintf(stderr,"from keyboard: throttle = %u roll = %d pitch = %d yaw = %d\n",k_throttle, k_roll, k_pitch, k_yaw);
			//fprintf(stderr,"from joystick: throttle = %u roll = %d pitch = %d yaw = %d\n",j_throttle, j_roll, j_pitch, j_yaw);
			send_command(Command);
		}
	}
	//send PANIC or EXIT until get ack
	C.mode = mode;
	C.frame = frame;
	waiting_for_ack = true;
	while(waiting_for_ack)
	{
		send_command(C);
		mon_delay_ms(t_threshold);
		get_data();
	}

       if(C.mode == EXIT)
        {
              while(c = (rs232_getchar() != 0x7F));
             /*
               while(1)
              {   c = rs232_getchar();
                  if(c == 0x7F) break;
                  fprintf(stderr, "%d ", c);
                  fprintf(fp, "%d ", c);
                  //data[j] = c;
                  //j++;
                  //term_putchar(c);
                  //term_puts("\t");
                  count1++;
                  if(count1 == LOG_SIZE - 1) 
                  {
                       fprintf(stderr, "\n");
                       fprintf(fp, "\n");
                       count1 = 0;
                  }
              }*/
 
             while(1)
             {  
                     c = rs232_getchar();
                     if(c == 0x7F) break;
                     fprintf(stderr, "%d ", c);
                  
                     if(count1 >= 0 && count1 <= 3)
                     {  
                            if(count1 == 0)
                                 time = 0;
                            time += (c << ((8 * (3 - count1))));
                            //fprintf(stderr, "c: %d\ttime: %d\t ", c, time);
                            if (count1 == 3)
                                 fprintf(fp, "%d\t ", time);
                     }

                     else if(count1 >= 4 && count1 <= 8)
                            fprintf(fp, "%d\t ", c);
   
                     else if(count1 <= 28)
                     {
                            if(count1 % 2 != 0)
                                  log_data = (c << 8);
                            else
                            {
                                  log_data += c;
                                  fprintf(fp, "%d\t ", log_data);
                            }   
                      }   

                      else if(count1 >= 29 && count1 <= 32)
                      { 
                            if(count1  == 29)
                                  time = 0;
                            time += (c << (8 * (32 - count1)));
                            if (count1 == 32)
                                  fprintf(fp, "%d\t ", time);
                      }      

                      count1++;
                      if(count1 == LOG_SIZE - 1) 
                      {
                            fprintf(stderr, "\n");
                            fprintf(fp, "\n");
                            count1 = 0;
                      }
             }      
        }
   



	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");
        fclose(fp);
	return 0;
}

