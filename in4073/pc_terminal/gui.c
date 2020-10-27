/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */
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

uint16_t k_throttle,j_throttle,throttle;
int16_t k_roll = 0, k_pitch = 0, k_yaw = 0;
int16_t j_roll = 0, j_pitch = 0, j_yaw = 0;
int16_t roll = 0, pitch = 0, yaw = 0;
uint8_t P, P1, P2 = 0;
uint8_t mode = 0;
uint8_t frame = 0;
uint8_t crc = 0;
bool send;

/*------------------------------------------------------------
 * GUI
 *------------------------------------------------------------
 */

static void
to_safe (GtkWidget *widget,
             gpointer   data)
{
  mode = SAFE;
  send = true;
}

static void
to_panic (GtkWidget *widget,
             gpointer   data)
{
  mode = PANIC;
  send = true;
}

static void
to_manual (GtkWidget *widget,
             gpointer   data)
{
  mode = MANUAL;
  send = true;
}

static void
to_calibration (GtkWidget *widget,
             gpointer   data)
{
  mode = CALIBRATION;
  send = true;
}

static void
to_yaw (GtkWidget *widget,
             gpointer   data)
{
  mode = YAW;
  send = true;
}

static void
to_full (GtkWidget *widget,
             gpointer   data)
{
  mode = FULL;
  send = true;
}

static void
to_raw (GtkWidget *widget,
             gpointer   data)
{
  mode = RAW;
  send = true;
}

static void
to_height (GtkWidget *widget,
             gpointer   data)
{
  mode = HEIGHT;
  send = true;
}

static void
to_exit (GtkWidget *widget,
             gpointer   data)
{
  mode = EXIT;
  send = true;
}


  /* Declare variables */
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
GtkWidget *grid;
GtkAdjustment *throttle_adjustment;
GtkAdjustment *roll_adjustment;
GtkAdjustment *pitch_adjustment;
GtkAdjustment *yaw_adjustment;
GtkAdjustment *P_adjustment;
GtkAdjustment *P1_adjustment;
GtkAdjustment *P2_adjustment;
/* This is the callback function. 
 * It is a handler function which reacts to the signal. 
 * In this case, it will notify the user the value of their scale as a label.
 */

static void
throttle_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */

   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_throttle = gtk_range_get_value (range);
	send = true;
  }

   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("throttle: %u", k_throttle);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
roll_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_roll = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("roll: %d", k_roll);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
pitch_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_pitch = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("pitch: %d", k_pitch);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
yaw_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	k_yaw = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("yaw: %d", k_yaw);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
P_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	P = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("P: %d", P);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
P1_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	P1 = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("P1: %d", P1);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}

static void
P2_moved (GtkRange *range,
              gpointer  user_data)
{
   GtkWidget *label = user_data;

   /* Get the value of the range, and convert it into a string which will be
    * used as a new label for the horizontal scale.
    * %.0f - stands for a double that will have 0 decimal places.
    */
   if(mode == SAFE) 
   {
	//printf("can't increase throttle in safe mode!\n");
	gtk_range_set_value(range,0);
   }
   else
  {
   	P2 = gtk_range_get_value (range);
	send = true;
  }
   /* Note: Using g_strdup_printf returns a string that must be freed. 
    * (In which is done below)
    */
   gchar *str = g_strdup_printf ("P2: %d", P2);
   gtk_label_set_text (GTK_LABEL (label), str);

   g_free(str);
}


static void
activate (GtkApplication *app,
          gpointer        user_data)
{


  /* The Adjustment object represents a value 
   * which has an associated lower and upper bound.
   */


  /* Create a window with a title and a default size */
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Scale Example");
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);
  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  //button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  //gtk_container_add (GTK_CONTAINER (window), button_box);

  b_safe = gtk_button_new_with_label ("SAFE");
  g_signal_connect (b_safe, "clicked", G_CALLBACK (to_safe), NULL);
  b_panic = gtk_button_new_with_label ("PANIC");
  g_signal_connect (b_panic, "clicked", G_CALLBACK (to_panic), NULL);
  b_manual = gtk_button_new_with_label ("MANUAL");
  g_signal_connect (b_manual, "clicked", G_CALLBACK (to_manual), NULL);
  b_calibration = gtk_button_new_with_label ("CALIBTRATION");
  g_signal_connect (b_calibration, "clicked", G_CALLBACK (to_calibration), NULL);
  b_yaw = gtk_button_new_with_label ("YAW");
  g_signal_connect (b_yaw, "clicked", G_CALLBACK (to_yaw), NULL);
  b_full = gtk_button_new_with_label ("FULL");
  g_signal_connect (b_full, "clicked", G_CALLBACK (to_full), NULL);
  b_raw = gtk_button_new_with_label ("RAW");
  g_signal_connect (b_raw, "clicked", G_CALLBACK (to_raw), NULL);
  b_height = gtk_button_new_with_label ("HEIGHT");
  g_signal_connect (b_height, "clicked", G_CALLBACK (to_height), NULL);
  b_exit = gtk_button_new_with_label ("EXIT");
  g_signal_connect (b_exit, "clicked", G_CALLBACK (to_exit), NULL);

  //g_signal_connect_swapped (b_safe, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  //gtk_container_add (GTK_CONTAINER (button_box), b_safe);


  /* Two labels to be shown in the window */
  throttle_label = gtk_label_new (g_strdup_printf ("throttle: %u", k_throttle));
  roll_label = gtk_label_new (g_strdup_printf ("roll: %u", k_roll));
  pitch_label = gtk_label_new (g_strdup_printf ("pitch: %u", k_pitch));
  yaw_label = gtk_label_new (g_strdup_printf ("yaw: %u", k_yaw));
  P_label = gtk_label_new (g_strdup_printf ("P: %u", P));
  P1_label = gtk_label_new (g_strdup_printf ("P1: %u", P1));
  P2_label = gtk_label_new (g_strdup_printf ("P2: %u", P2));
  //vlabel = gtk_label_new ("Move the scale handle...");

   
  /* gtk_adjustment_new takes six parameters, three of which 
   * may be difficult to understand:
   * step increment- move the handle with the arrow keys on your keyboard to see.
   * page increment - move the handle by clicking away from it 
   * on the scale to see.
   * page size - not used here.
   */
  throttle_adjustment = gtk_adjustment_new (0, 0, 65535, 5, 10, 0);
  roll_adjustment = gtk_adjustment_new (0, -32768, 32767, 5, 10, 0);
  pitch_adjustment = gtk_adjustment_new (0, -32768, 32767, 5, 10, 0);
  yaw_adjustment = gtk_adjustment_new (0, -32768, 32767, 5, 10, 0);
  P_adjustment = gtk_adjustment_new (0, 0, 100, 1, 5, 0);
  P1_adjustment = gtk_adjustment_new (0, 0, 100, 1, 5, 0);
  P2_adjustment = gtk_adjustment_new (0, 0, 100, 1, 5, 0);
  /* Create the Horizontal scale, making sure the 
   * digits used have no decimals.
   */
  throttle_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, throttle_adjustment);
  roll_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, roll_adjustment);
  pitch_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, pitch_adjustment);
  yaw_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, yaw_adjustment);
  P_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, P_adjustment);
  P1_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, P1_adjustment);
  P2_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, P2_adjustment);

  gtk_scale_set_digits (GTK_SCALE (throttle_scale), 0); 
  gtk_scale_set_digits (GTK_SCALE (roll_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (pitch_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (yaw_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (P_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (P1_scale), 0);
  gtk_scale_set_digits (GTK_SCALE (P2_scale), 0);

  /* Allow it to expand horizontally (if there's space), and 
   * set the vertical alignment
   */
  gtk_widget_set_hexpand (throttle_scale, TRUE);
  gtk_widget_set_valign (throttle_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (roll_scale, TRUE);
  gtk_widget_set_valign (roll_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (pitch_scale, TRUE);
  gtk_widget_set_valign (pitch_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (yaw_scale, TRUE);
  gtk_widget_set_valign (yaw_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (P_scale, TRUE);
  gtk_widget_set_valign (P_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (P1_scale, TRUE);
  gtk_widget_set_valign (P1_scale, GTK_ALIGN_START);
  gtk_widget_set_hexpand (P2_scale, TRUE);
  gtk_widget_set_valign (P2_scale, GTK_ALIGN_START);
  
  /* Connecting the "value-changed" signal for the horizontal scale 
   * to the appropriate callback function. 
   * take note that GtkRange is part of GtkScale's Object Hierarchy.
   */
  g_signal_connect (throttle_scale, 
                    "value-changed", 
                    G_CALLBACK (throttle_moved), 
                    throttle_label);
  g_signal_connect (roll_scale, 
                    "value-changed", 
                    G_CALLBACK (roll_moved), 
                    roll_label);
  g_signal_connect (pitch_scale, 
                    "value-changed", 
                    G_CALLBACK (pitch_moved), 
                    pitch_label);
  g_signal_connect (yaw_scale, 
                    "value-changed", 
                    G_CALLBACK (yaw_moved), 
                    yaw_label);
  g_signal_connect (P_scale, 
                    "value-changed", 
                    G_CALLBACK (P_moved), 
                    P_label);
  g_signal_connect (P1_scale, 
                    "value-changed", 
                    G_CALLBACK (P1_moved), 
                    P1_label);
  g_signal_connect (P2_scale, 
                    "value-changed", 
                    G_CALLBACK (P2_moved), 
                    P2_label);

  /* Create a grid and arrange everything accordingly */
  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
  //gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_attach (GTK_GRID (grid), throttle_scale, 1, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), throttle_label, 2, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), roll_scale, 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), roll_label, 2, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), pitch_scale, 1, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), pitch_label, 2, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), yaw_scale, 1, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), yaw_label, 2, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P_scale, 1, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P_label, 2, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P1_scale, 1, 5, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P1_label, 2, 5, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P2_scale, 1, 6, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), P2_label, 2, 6, 1, 1);

  gtk_grid_attach (GTK_GRID (grid), b_safe, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_panic, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_manual, 0, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_calibration, 0, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_yaw, 0, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_full, 0, 5, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_raw, 0, 6, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_height, 0, 7, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), b_exit, 0, 8, 1, 1);

  gtk_container_add (GTK_CONTAINER (window), grid);

  gtk_widget_show_all (window);
}

#define LOG_SIZE 38
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
                               {   
					rs232_putchar(0xFF);  ready = false;
					//break;
			       }
			       else
                               fprintf(fp, "%d\t ", time2);
                      }
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
                              time2 = 0;
                      time2 += (c << (8 * (32 - count1)));
                      if (count1 == 32)
                              fprintf(fp, "%d\t ", time2);
              } 

              else if(count1 >= 33 && count1 <= 36)
              { 
                      if(count1  == 33)
                              time2 = 0;
                      time2 += (c << (8 * (36 - count1)));
                      if (count1 == 36)
                              fprintf(fp, "%d\t ", time2);
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
		if(button[0]==1) mode = EXIT;
		return true;
	}

	return false;
}

bool get_keyboard()
{
	int c;
	if ((c = term_getchar_nb()) != -1)
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
				if(mode!=SAFE && mode!= 2)
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
				if(mode!=SAFE && mode!=3)
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
				if(mode!=SAFE && mode!= 4 && mode!=RAW)
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
				if(mode!=SAFE && mode!=5 && mode!=RAW)
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
				/*if(mode!=SAFE && mode!= 6)
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
				{*/
					mode = RAW;
					return true;
				//}
			case '7':
				/*if(mode!=SAFE && mode!= 6)
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
				{*/
					mode = HEIGHT;
					return true;
				//}
			case '8':
				if(0)
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
				if(k_throttle>64535) k_throttle = 65535;
				else k_throttle+=1000;
				return true;
			case 'f'://lift down
				if(k_throttle<1000) k_throttle = 0;
				else k_throttle-=1000;
				return true;
			case 'a'://left, roll up
				if(k_roll>31767) k_roll = 32767;
				else k_roll+=1000;
				return true;
			case 'd'://right, roll down
				if(k_roll<-31768) k_roll = -32768;
				else k_roll-=1000;
				return true;
			case 'w'://up, pitch down
				if(k_pitch<-31768) k_pitch = -32768;
				else k_pitch-=1000;
				return true;
			case 's'://down, pitch up
				if(k_pitch>31767) k_pitch = 32767;
				else k_pitch+=1000;
			case 'q'://yaw down
				if(k_yaw<-31768) k_yaw = -32768;
				else k_yaw-=1000;
				return true;
			case 'e'://yaw up
				if(k_yaw>31767) k_yaw = 32767;
				else k_yaw+=1000;
				return true;
			case 'u'://P+
				P+=1;
				return true;
			case 'j'://P-
				if(P<1) P = 0;
				else P-=1;
				return true;
			case 'i'://P1+
				P1+=1;
				return true;
			case 'k'://P1-
				if (P1<1) P1 = 0;
				else P1-=1;
				return true;
			case 'o'://P2+
				P2+=1;
				return true;
			case 'l'://P2-
				if(P2<1) P2 = 0;
				else P2-=1;
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
				if(mode == PANIC || mode == CALIBRATION) mode = SAFE;
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
				if(mode == PANIC || mode == CALIBRATION) mode = SAFE;
				//fprintf(stderr,"get ack %d\n",next_c);
			}	
		}
		else if(c == 0xFD)
		{
			fprintf(stderr,"recover from PANIC mode");
			mode = SAFE;
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

#define MILLION  1000000L
void *loop()
{
	int	c;
	int	fd;
	//struct js_event js;

        double accum;
        struct timespec start, stop;

	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();
/*
	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}
*/
	fcntl(fd, F_SETFL, O_NONBLOCK);
	
        fp2 = fopen("log2.txt", "w"); 
        fp = fopen("log.txt", "w");
        
 
        fprintf(fp, "TIME \t THROTTLE \t ROLL \t PITCH \t YAW \t MODE \t PHI \t THETA \t PSI \t SP \t SQ \t SR \t MOTOR 0 \t MOTOR 1 \t MOTOR 2 \t MOTOR 3 \t LOOP TIME\n");

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
	//get_joystick(fd);
	while(j_throttle!=0||j_yaw!=0||j_pitch!=0||j_roll!=0)
	{
		fprintf(stderr,"please set joystick to neutral\n");
		mon_delay_ms(500);
		//get_joystick(fd);
	}
	send_command();
	while (1)
	{
		get_data();
		send = send || resend();
               
                if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) 
		{
                	perror( "clock gettime" );
      			exit( EXIT_FAILURE );
    		}

		//send = send || get_joystick(fd);
		send = send || get_keyboard();
		if(miss_count>5) mode = PANIC;
		if(mode == EXIT) break;
		if (send)
		{
			//fprintf(stderr,"send frame: %u mode: %u throttle: %u roll: %d pitch: %d yaw: %d P: %u P1: %u P2: %u\n",frame, mode, throttle, roll, pitch, yaw, P, P1, P2);
			send_command();
			if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
      				perror( "clock gettime" );
      				exit( EXIT_FAILURE );
    			}
    			accum = (float)((float)(stop.tv_sec-start.tv_sec)*MILLION + (float)((stop.tv_nsec- start.tv_nsec)/1000));
                        fprintf(fp2, "%f \n", accum);
                        //fprintf(stderr, "time :%f\n", accum);
		}
                else if(send_period())
                {
                        send_connection_check();
                }
	}
	//send PANIC or EXIT until get ack
	waiting_for_ack = true;
	while(waiting_for_ack)
	{
		send_command();
		mon_delay_ms(t_threshold);
		get_data();
	}

        if(mode == EXIT)
        {  
                while(rs232_getchar() != 0x00);
                c = 0;
                log_file(c);

                rs232_putchar(0x00);
                while(ready )
                {      
                        c = rs232_getchar(); 
                        log_file(c);
                }      
        }
   
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");
        fclose(fp);
	return 0;
}

int
main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_thread_new("loop",loop, NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
