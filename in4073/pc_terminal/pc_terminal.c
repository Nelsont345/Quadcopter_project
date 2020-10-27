/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */
#include "gui.h"
//#include "pc_terminal.h"

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
        
 
        fprintf(fp, "TIME \t THROTTLE ROLL \t PITCH \t YAW \t MODE \t PHI \t THETA \t PSI \t SP \t SQ \t SR \t MOTOR 0\tMOTOR 1\tMOTOR 2\tMOTOR 3\t CYCLE TIME LOOP TIME\n");

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
	while(!check_joystick())
	{
		fprintf(stderr,"please set joystick to neutral\n");
		mon_delay_ms(1000);
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
		if(miss_count>5)
		{
			mode = PANIC;
			update_gui();
		}
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
                        //if(c != 0xFD)
                        log_file(c);
                }      
        }
   
	term_exitio();
	rs232_close();
        fclose(fp);
        fclose(fp2);
	term_puts("\n<exit>\n");

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
