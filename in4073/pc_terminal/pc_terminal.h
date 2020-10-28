#include <gtk/gtk.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "math.h"
#include "crc.h"
#include <stdlib.h>


/*------------------------------------------------------------
 * GUI variables
 *------------------------------------------------------------
 */
GtkWidget *window;

GtkWidget *button_box;
GtkWidget *b_safe;
GtkWidget *b_panic;
GtkWidget *b_manual;
GtkWidget *b_calibration;
GtkWidget *b_yaw;
GtkWidget *b_full;
GtkWidget *b_raw;
GtkWidget *b_height;
GtkWidget *b_exit;

GtkWidget *throttle_scale;
GtkWidget *throttle_label;
GtkWidget *roll_scale;
GtkWidget *roll_label;
GtkWidget *pitch_scale;
GtkWidget *pitch_label;
GtkWidget *yaw_scale;
GtkWidget *yaw_label;
GtkWidget *P_scale;
GtkWidget *P_label;
GtkWidget *P1_scale;
GtkWidget *P1_label;
GtkWidget *P2_scale;
GtkWidget *P2_label;
GtkWidget *Q_scale;
GtkWidget *Q_label;
GtkWidget *info;
GtkWidget *cur_mode;
GtkWidget *grid;
GtkAdjustment *throttle_adjustment;
GtkAdjustment *roll_adjustment;
GtkAdjustment *pitch_adjustment;
GtkAdjustment *yaw_adjustment;
GtkAdjustment *P_adjustment;
GtkAdjustment *P1_adjustment;
GtkAdjustment *P2_adjustment;
GtkAdjustment *Q_adjustment;
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
#define HEIGHT		7
#define EXIT		8

uint16_t k_throttle = 0,j_throttle = 0,throttle = 0;
int16_t k_roll = 0, k_pitch = 0, k_yaw = 0;
int16_t j_roll = 0, j_pitch = 0, j_yaw = 0;
int16_t roll = 0, pitch = 0, yaw = 0;
uint8_t P, P1, P2 = 0;
uint16_t Q = 0;
uint8_t mode = 0;
uint8_t frame = 0;
uint8_t crc = 0;
uint8_t raw = 0;
uint8_t height = 0;
bool send = false;


#define LOG_SIZE 48
int count1 = 0;
int row = 0;
int data[0xFFFF];
int j;
bool ready = true;
        uint32_t time2;
        int16_t log_data;


        FILE *fp; 
                FILE *fp2;


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

void log_file(int c)
{
              fprintf(stderr, "%d ",c);
              if(count1 >= 0 && count1 <= 3)
              {  
                      if(count1 == 0)
                      time2 = 0;
                      time2 += (c << ((8 * (3 - count1))));
                      if (count1 == 3)
                      {
                               if(time2 == -1)
                               {        fprintf(stderr,"done logging!");
					rs232_putchar(0xFF);  ready = false;
					//break;
			       }
			       else
                               fprintf(fp, "%d,", time2);
                      }
              }

              else if(count1 >= 4 && count1 <= 8)
                      fprintf(fp, "%d,", c);
   
              else if(count1 <= 34)
              {
                      if(count1 % 2 != 0)
                             log_data = (c << 8);
                      else
                      {
                             log_data += c;
                             fprintf(fp, "%d,", log_data);
                      }   
              }   

              else if(count1 >= 35 && count1 <= 38)
              { 
                      if(count1  == 35)
                              time2 = 0;
                      time2 += (c << (8 * (38 - count1)));
                      if (count1 == 38)
                              fprintf(fp, "%d,", time2);
              } 

              else if(count1 >= 39 && count1 <= 42)
              { 
                      if(count1  == 39)
                              time2 = 0;
                      time2 += (c << (8 * (42 - count1)));
                      if (count1 == 42)
                              fprintf(fp, "%d,", time2);
              }   
              else if(count1 >= 43 && count1 <= 46)
              { 
                      if(count1  == 43)
                              time2 = 0;
                      time2 += (c << (8 * (46 - count1)));
                      if (count1 == 46)
                              fprintf(fp, "%d,", time2);
              }      
              count1++;
              if(count1 == LOG_SIZE - 1) 
              {
                      fprintf(stderr, "\n");
                      fprintf(fp, "\n");
                      count1 = 0;
              }
             //track++;
}
/*------------------------------------------------------------
 * time
 *------------------------------------------------------------
 */
unsigned int last_sending_time;
unsigned int previous_time;//last checking time
unsigned int interval=500;//sending interval
unsigned int sending_time[256];

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
	if(current_time-last_sending_time > interval)
	{
		return true;
	}
	return false;
}


/*------------------------------------------------------------
 * utilities
 *------------------------------------------------------------
 */

int16_t sadd(int16_t a, int16_t b)
{
	int16_t c = a + b;
	if(a>0&&b>0) return c>0?c:32767;
	else if(a<0&&b<0) return c>0?-32768:c;
	else return c;
}
uint16_t unsigned_sadd(uint16_t a, uint16_t b)
{
	uint16_t c = a+b;
	if(c<a) return 65535;
	else return c;
}

void initialize()
{
	k_throttle = j_throttle = throttle = 0;
	k_roll = k_pitch = k_yaw = 0;
	//j_roll = j_pitch = j_yaw = 0;
	roll = pitch = yaw = 0;
	return;
}
void update_gui()
{
	gtk_adjustment_set_value(throttle_adjustment,k_throttle);
	gtk_adjustment_set_value(pitch_adjustment,k_pitch);
	gtk_adjustment_set_value(roll_adjustment,k_roll);
	gtk_adjustment_set_value(yaw_adjustment,k_yaw);
	gtk_adjustment_set_value(P_adjustment,P);
	gtk_adjustment_set_value(P1_adjustment,P1);
	gtk_adjustment_set_value(P2_adjustment,P2);
	gtk_adjustment_set_value(Q_adjustment,Q);
	gchar *mode_str;
	switch(mode)
	{
		case 0:
			mode_str = g_strdup_printf ("current mode: SAFE");
			break;
		case 1:
			mode_str = g_strdup_printf ("current mode: PANIC");
			break;
		case 2:
			mode_str = g_strdup_printf ("current mode: MANUAL");
			break;
		case 3:
			mode_str = g_strdup_printf ("current mode: CALIBRATION");
			break;
		case 4:
			if(raw&&height) mode_str = g_strdup_printf ("current mode: YAW(RAW-HEIGHT)");
			else if(raw) mode_str = g_strdup_printf ("current mode: YAW(RAW)");
			else if(height) mode_str = g_strdup_printf ("current mode: YAW(HEIGHT)");
			else mode_str = g_strdup_printf ("current mode: YAW");
			break;
		case 5:
			if(raw&&height) mode_str = g_strdup_printf ("current mode: FULL(RAW-HEIGHT)");
			else if(raw) mode_str = g_strdup_printf ("current mode: FULL(RAW)");
			else if(height) mode_str = g_strdup_printf ("current mode: FULL(HEIGHT)");
			else mode_str = g_strdup_printf ("current mode: FULL");
			break;
		case 8:
			mode_str = g_strdup_printf ("current mode: EXIT");
			break;
	}
	gtk_label_set_text (GTK_LABEL (cur_mode), mode_str);
}

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

		switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
		}
		get_values = true;
	}
	if(get_values)
	{
		j_throttle = (-axis[3]+32767); 
		j_roll = axis[0]; 
		j_pitch = axis[1]; 
		j_yaw = axis[2];
		if(button[0]==1) 
		{
			mode = PANIC;   
			update_gui();
		}
		return true;
	}

	return false;
}

bool check_joystick()
{
	return (j_throttle==0&&j_roll==0&&j_pitch==0&&j_yaw==0);
}

bool get_keyboard()
{
	int c;
	if ((c = term_getchar_nb()) != -1)
	{
		switch (c)
		{
			case '0':
				if(mode==SAFE)
				{
					printf("already in safe mode\n");
					return false;
				}
				mode = SAFE;
				initialize();
				update_gui();
				return true;
			case '1':
				mode = PANIC;
				return true;
			case '2':
				if(mode==MANUAL)
				{
					printf("already n manual mode\n");
					return false;
				}
				else if(mode!=SAFE && mode!= 2)
				{
					printf("Please go back to safe mode before switch mode\n");
					return false;
					
				}
				else if(!check_joystick())
				{
					printf("Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = MANUAL;
					update_gui();
					return true;
				}
			case '3':
				if(mode==CALIBRATION)
				{
					printf("already calibrating\n");
					return false;
				}
				else if(mode!=SAFE)
				{
					printf("Please go back to safe mode before switch mode\n");
					return false;
				}
				else
				{
					mode = CALIBRATION;
					update_gui();
					return true;
				}
			case '4':
				if(mode==YAW)
				{
					printf("already in yaw-control mode\n");
					return false;
				}
				else if(mode!=SAFE)
				{
					printf("Please go back to safe mode before switch mode\n");
					return false;
				}
				else if(!check_joystick())
				{
					printf("Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = YAW;
					update_gui();
					return true;
				}
			case '5':
				if(mode==FULL)
				{
					printf("already in full-control mode\n");
					return false;
				}
				else if(mode!=SAFE)
				{
					printf("Please go back to safe mode before switch mode\n");
					return false;
				}
				else if(!check_joystick())
				{
					printf("Please set joystick to origin\n");
					return false;

				}
				else
				{
					mode = FULL;
					update_gui();
					return true;
				}
			case '6':
					if(raw == 0) raw = 1;
					else raw = 0;
					update_gui();
					return true;
			case '7':

					if(height == 0) height = 1;
					else height = 0;
					update_gui();
					return true;
			case '8':
				if(mode!=SAFE)
				{
					printf("Please go back to safe mode before exit\n");
					return false;
				}
				else
				{
					mode = EXIT;
					update_gui();
					return true;	
				}
		}
		switch(c)
		{
			case 'u'://P+
				if(P!=255) P+=1;
				gtk_adjustment_set_value(P_adjustment,P);
				return true;
			case 'j'://P-
				if(P<1) P = 0;
				else P-=1;
				gtk_adjustment_set_value(P_adjustment,P);
				return true;
			case 'i'://P1+
				if(P1!=255) P1+=1;
				gtk_adjustment_set_value(P1_adjustment,P1);
				return true;
			case 'k'://P1-
				if (P1<1) P1 = 0;
				else P1-=1;
				gtk_adjustment_set_value(P1_adjustment,P1);
				return true;
			case 'o'://P2+
				if(P2!=255) P2+=1;
				gtk_adjustment_set_value(P2_adjustment,P2);
				return true;
			case 'l'://P2-
				if(P2<1) P2 = 0;
				else P2-=1;
				gtk_adjustment_set_value(P2_adjustment,P2);
				return true;
			case 'y'://Q+
				if(Q!=65535) Q+=100;
				gtk_adjustment_set_value(Q_adjustment,Q);
				return true;
			case 'h'://Q-
				if(Q<100) Q = 0;
				else Q-=100;
				gtk_adjustment_set_value(Q_adjustment,Q);
				return true;
		}
		if (mode == SAFE || mode == CALIBRATION) 
		{
			printf("can't change vlaues in safe/calibration mode.\n");
			return false;
		}
		switch (c)
		{
			//still need to limit the values
			case 'r'://lift up
				if(k_throttle>64535) k_throttle = 65535;
				else k_throttle+=1000;
				//gtk_range_set_value(throttle_scale->Range,k_throttle);
				height = 0;
				gtk_adjustment_set_value(throttle_adjustment,k_throttle);
				return true;
			case 'f'://lift down
				if(k_throttle<1000) k_throttle = 0;
				else k_throttle-=1000;
				gtk_adjustment_set_value(throttle_adjustment,k_throttle);
				return true;
			case 'a'://left, roll up
				if(k_roll>31767) k_roll = 32767;
				else k_roll+=1000;
				gtk_adjustment_set_value(roll_adjustment,roll);
				return true;
			case 'd'://right, roll down
				if(k_roll<-31768) k_roll = -32768;
				else k_roll-=1000;
				gtk_adjustment_set_value(roll_adjustment,k_roll);
				return true;
			case 'w'://up, pitch down
				if(k_pitch<-31768) k_pitch = -32768;
				else k_pitch-=1000;
				gtk_adjustment_set_value(pitch_adjustment,k_pitch);
				return true;
			case 's'://down, pitch up
				if(k_pitch>31767) k_pitch = 32767;
				else k_pitch+=1000;
				gtk_adjustment_set_value(pitch_adjustment,k_pitch);
				return true;
			case 'q'://yaw down
				if(k_yaw<-31768) k_yaw = -32768;
				else k_yaw-=1000;
				gtk_adjustment_set_value(yaw_adjustment,k_yaw);
				return true;
			case 'e'://yaw up
				if(k_yaw>31767) k_yaw = 32767;
				else k_yaw+=1000;
				gtk_adjustment_set_value(yaw_adjustment,k_yaw);
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
int t_threshold = 1000; 
void send_command()
{
	throttle = unsigned_sadd(j_throttle, k_throttle);
	roll = sadd(j_roll, k_roll);
	pitch = sadd(j_pitch, k_pitch);
	yaw = sadd(j_yaw, k_yaw);
	rs232_putchar(0xFF);
	rs232_putchar(frame);
	rs232_putchar(mode);
        rs232_putchar(get_crc(0, mode, 1));
	rs232_putchar(raw);
	rs232_putchar(height);
	rs232_putchar(throttle>>8);
	rs232_putchar(throttle);
	rs232_putchar(roll>>8);
	rs232_putchar(roll);
	rs232_putchar(pitch>>8);
	rs232_putchar(pitch);
	rs232_putchar(yaw>>8);
	rs232_putchar(yaw);
	rs232_putchar(P);
	rs232_putchar(P1);
	rs232_putchar(P2);
	rs232_putchar(Q>>8);
	rs232_putchar(Q);

        //fprintf(stderr, "send frame: %u mode: %u throttle: %u roll: %d pitch: %d yaw: %d P: %u P1: %u P2: %u crc: %u \n",frame, mode, throttle, roll, pitch, yaw, P, P1, P2, get_crc(0, mode, 1));
	last_sending_time = mon_time_ms();
	sending_time[frame] = last_sending_time;
	frame++;
	waiting_for_ack = true;
	send = false;
}


void send_connection_check()
{
	rs232_putchar(0xFE);
	rs232_putchar(frame);
	last_sending_time = mon_time_ms();
	sending_time[frame] = last_sending_time;
	frame++;	
	waiting_for_ack = true;
}

void get_data()
{
	int c;
	while((c = rs232_getchar_nb()) != -1)
	{
		if(c == 0xFF)
		{
			int ack_frame = rs232_getchar_nb();
			if(ack_frame == frame-1)//if it is the ack for the last frame
			{
				unsigned int receiving_time = mon_time_ms();
				//fprintf(stderr,"get ack %d after %ums\n",ack_frame,receiving_time-sending_time[ack_frame]);
				fprintf(fp2,"message_packet\t%u ms\n", receiving_time-sending_time[ack_frame]);
				miss_count = 0;
				waiting_for_ack = false;
				//fprintf(stderr,"get ack %d\n",next_c);
			}	
		}
		else if(c == 0xFE)
		{
			int ack_frame = rs232_getchar_nb();
			if(ack_frame == frame-1)//if it is the ack for the last frame
			{
				unsigned int receiving_time = mon_time_ms();
				//fprintf(stderr,"get ack(check) %d after %ums\n",ack_frame,receiving_time-sending_time[ack_frame]);
				miss_count = 0;
				waiting_for_ack = false;
				//fprintf(stderr,"get ack %d\n",next_c);
			}	
		}
		else if(c == 0xFC)
		{

				//fprintf(stderr,"get ack %d\n",next_c);
			gchar *str = g_strdup_printf ("calibrated!");
			gtk_label_set_text (GTK_LABEL (info), str);
			mode = SAFE;
			initialize();
			update_gui();
		}
		else if(c == 0xFD)
		{
			gchar *str = g_strdup_printf ("recover from PANIC mode");
			gtk_label_set_text (GTK_LABEL (info), str);
			fprintf(stderr,"recover from PANIC mode");
			mode = SAFE;
			initialize();
			update_gui();
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

