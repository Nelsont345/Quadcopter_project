/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include "pc_terminal/crc.h"
/*------------------------------------------------------------------
 * process_key -- process command keys
 *------------------------------------------------------------------
 */

uint8_t command_type;
uint8_t size;
bool receiving_data = false;
uint32_t cur_time;
uint32_t write_address = 0x00000000;
uint32_t read_address = 0x00000000;
uint8_t frame;
uint8_t data[DATASIZE];
uint8_t data_r[DATASIZE];
uint8_t data_r2[DATASIZE];
bool iflag = false;
uint32_t t_access;

void get_command()
{
	frame = dequeue(&rx_queue);
	uint8_t new_mode = dequeue(&rx_queue);
	uint8_t crc = dequeue(&rx_queue);

	if(crc != get_crc2(0, new_mode, 1)) 
	{
		//printf("wrong! frame: %u mode: %u throttle: %u roll: %d pitch: %d yaw: %d P: %u P1: %u P2: %u crc: %u \n",frame, new_mode, new_throttle, new_roll, new_pitch, new_yaw, new_P, new_P1, new_P2, crc);
		printf("wrong command!\n\n");
		for(int i=0;i<13;i++) dequeue(&rx_queue); 
		return;
	}

	mode = new_mode;
	uint8_t new_raw_mode = dequeue(&rx_queue);
	uint8_t new_height_mode = dequeue(&rx_queue);
	throttle = dequeue(&rx_queue);
	throttle = (throttle<<8)+dequeue(&rx_queue);
        f_throttle = throttle;
	roll = dequeue(&rx_queue);
	roll = (roll<<8)+dequeue(&rx_queue);
	pitch = dequeue(&rx_queue);
	pitch = (pitch<<8)+dequeue(&rx_queue);
	yaw = dequeue(&rx_queue);
	yaw = (yaw<<8)+dequeue(&rx_queue);
	P = dequeue(&rx_queue);
	P1 = dequeue(&rx_queue);
	P2 = dequeue(&rx_queue);
	Q = dequeue(&rx_queue);
	Q = (Q<<8)+dequeue(&rx_queue);
	//printf("frame: %u mode: %u raw_mode:%d throttle: %u roll: %d pitch: %d yaw: %d P: %u P1: %u P2: %u crc: %u fixed_pressure %ld throttle_new %d pressure %ld\n",frame, mode, raw_mode,throttle, roll, pitch, yaw, P, P1, P2, crc, fixed_pressure, throttle_new, pressure);
	//printf("frame: %u mode: %u\n",frame, mode);
	uart_put(0xFF);
	uart_put(frame);
    
        if(new_raw_mode == 1 && raw_mode == 0) imu_init(false, 100);
	else if(new_raw_mode == 0 && raw_mode == 1) imu_init(true, 100);
	if(height_mode == 0 && new_height_mode == 1) fixed_pressure = pressure;
	height_mode = new_height_mode;
	raw_mode = new_raw_mode;

	printf("frame: %u mode: %u raw_mode:%d height_mode:%d throttle: %u roll: %d pitch: %d yaw: %d P: %u P1: %u P2: %u Q: %u crc: %u fixed_pressure %ld throttle_new %ld pressure %ld\n\n",frame, mode, raw_mode, height_mode, throttle, roll, pitch, yaw, P, P1, P2, Q, crc, fixed_pressure, throttle_new, pressure);

	//if(mode!=8)
	//flash_data();
	t_access = get_time_us();
}

void get_connection_check()
{
	frame = dequeue(&rx_queue);
	//printf("frame: %u mode: %u raw_mode:%d throttle: %u roll: %d pitch: %d yaw: %d P: %u P1: %u P2: %u crc: %u sp %d sq %d sr %d sax %d say %d saz %d\n",frame, mode, raw_mode, throttle, roll, pitch, yaw, P, P1, P2, crc, sp, sq, sr, sax, say, saz);

	//printf("frame(check connection): %u\n",frame);
        //flash_data();
	uart_put(0xFE);
	uart_put(frame);
//flash_data();
}
void flash_data()
{
   cur_time = get_time_us();
   data[0] = ((cur_time & 0xFFFFFFFF) >> 24);
   data[1] = ((cur_time & 0xFFFFFF) >> 16);
   data[2] = ((cur_time & 0xFFFF) >> 8);
   data[3] = (cur_time & 0xFF); 
     
   data[4] = throttle;
   data[5] = roll;
   data[6] = pitch;
   data[7] = yaw;
   data[8] = mode;

   data[9]  = (phi & 0xFFFF) >> 8;
   data[10] = phi & 0xFF;
   data[11] = (theta & 0xFFFF) >> 8;
   data[12] = theta & 0xFF;
   data[13] = (psi & 0xFFFF) >> 8;
   data[14] = psi & 0xFF;
   data[15] = (sp & 0xFFFF) >> 8;
   data[16] = sp & 0xFF;
   data[17] = (sq & 0xFFFF) >> 8;
   data[18] = sq & 0xFF;
   data[19] = (sr & 0xFFFF) >> 8;
   data[20] = (sr & 0xFF); 


   data[21] = (motor[0] & 0xFFFF) >> 8;
   data[22] = motor[0] & 0xFF;
   data[23] = (motor[1] & 0xFFFF) >> 8;
   data[24] = motor[1] & 0xFF;
   data[25] = (motor[2] & 0xFFFF) >> 8;
   data[26] = motor[2] & 0xFF;
   data[27] = (motor[3] & 0xFFFF) >> 8;
   data[28] = motor[3] & 0xFF;

   data[29] = (cycle_time & 0xFFFFFFFF) >> 24;
   data[30] = (cycle_time & 0xFFFFFF) >> 16; 
   data[31] = (cycle_time & 0xFFFF) >> 8;
   data[32] = (cycle_time & 0xFF);

   data[33] = (response_time & 0xFFFFFFFF) >> 24;
   data[34] = (response_time & 0xFFFFFF) >> 16; 
   data[35] = (response_time & 0xFFFF) >> 8;
   data[36] = (response_time & 0xFF);

 
   if(!flash_write_bytes(write_address, data, DATASIZE))
   {   
       write_address = 0x00000000;
       flash_write_bytes(write_address, data, DATASIZE);  
   }


   write_address += (DATASIZE * 8);

}

void log_data()
{   uart_put(0x00);
    read_address = 0x00000000;
    bool read_rdy = false;
    while(!read_rdy)
    {   

        //printf("5");
        if(rx_queue.count)
        {     // printf("count\n");
               if(dequeue(&rx_queue) == 0x00)
               {     read_rdy = true;
                     break;
               }  
        }
        uart_put(0x00); 
        nrf_delay_ms(100);
    }
   // while(rx_queue.count && dequeue(&rx_queue) != 0x00)
   // uart_put(0x00); 
    nrf_delay_ms(1000);

    while(1)
    { 	    //printf("1\n");
            if(rx_queue.count && (dequeue(&rx_queue) == 0xFF))
                  break;
            if(flash_read_bytes(read_address, data_r, DATASIZE))
            { //   printf("2\n");
                  nrf_delay_ms(500);
                  for(int i = 0; i < DATASIZE; i++)
                  {
                          uart_put(data_r[i]);
		          //printf("%d ", data_r[i]);
                  }
                 
                  //printf("\n");
		 //uart_put("\n");
            }
            read_address += (DATASIZE * 8);
    }

    //uart_put(0xFE);
}

void raw_init()
{
	//ready = true;
	raw_mode = false;
	height_mode = false;
	processed_yaw = 0;       
	prev_yaw_x[0] = 0;
	prev_yaw_x[1] = 0;
	prev_yaw_y[0] = 0;
	prev_yaw_y[1] = 0; 
	processed_pitch = 0;       
	prev_pitch_x[0] = 0;
	prev_pitch_x[1] = 0;
	prev_pitch_y[0] = 0;
	prev_pitch_y[1] = 0; 
	processed_roll = 0;       
	prev_roll_x[0] = 0;
	prev_roll_x[1] = 0;
	prev_roll_y[0] = 0;
	prev_roll_y[1] = 0; 
	p_bias = 0;
    phi_error = 0;
    p_kalman = 0;
    phi_kalman = 0;
    sphi = 0;
    q_bias = 0;
    stheta = 0;
    theta_error = 0;
    q_kalman = 0;
    theta_kalman = 0;
}

/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */
int main(void)
{
        
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(true, 100);	
	baro_init();
	spi_flash_init();
	ble_init();
	raw_init();
	mode = SAFE;
	throttle = roll = pitch = yaw = 0;
	uint32_t counter = 0;
	demo_done = false;
        last_receiving_time = get_time_us();
	//prev_loop_time = last_receiving_time;
        //bool key_press = false;
	while (!demo_done)
	{      
		//printf("last%lu\n", last_receiving_time);
                tot_intr_time = 0;
		if (rx_queue.count>0)
		{
			if(!receiving_data)
			{
				command_type = dequeue(&rx_queue);
				if(command_type == 0xFF || command_type == 0xFE) receiving_data = true;
			}
			else
			{
				if(command_type == 0xFF && rx_queue.count>=18)
				{
					get_command();
					last_receiving_time = get_time_us();
					//key_press = true;
					receiving_data =false;
				}
				if(command_type == 0xFE && rx_queue.count>=1)
				{
					get_connection_check();
					last_receiving_time = get_time_us();
					receiving_data=false;
				}
			}
		}
		cur_time = get_time_us();
                if(mode!=SAFE && (cur_time>2500000+last_receiving_time)) 
		{
			printf("%lu  %lu\n",cur_time, last_receiving_time);
			mode = PANIC;
		}
                if(mode == EXIT)
                {       //mode = PANIC;
                        //run_filters_and_control();
                        uart_put(0x00);
                        demo_done = true;
//
        		log_data();

                }


		if (check_timer_flag()) 
		{			
			adc_request_sample();
			read_baro();
			 //printf("cycle time: %lu \n", cycle_time);
			//	printf("%3d %3d %3d %3d |\n ",ae[0],ae[1],ae[2],ae[3]);
			//	printf("%6d %6d %6d | ", phi, theta, psi);
			//	printf("%6d	%6d	%6d\n	", sp, sq, sr);
                        if(counter++%16 == 0)
                        {       //plotting help
                                //printf("sr %6d yaw %3d P %6d\n", sr, yaw, P);
                                //printf("phi %6d roll %3d sp %6d P1 %6d P2 %6d\n", phi, roll, sp, P1, P2);
                                //printf("theta %6d pitch %3d sq %6d P1 %6d P2 %6d\n", theta, pitch, sq, P1, P2);


                                //debugging help
                                //printf("pitch_new %ld pitch_rate_err %ld ae %3d %3d %3d %3d\n", pitch_new, pitch_rate_err, ae[0], ae[1], ae[2], ae[3]);
                                //printf("pitch_new %ld pitch_angle_err %ld ae %3d %3d %3d %3d\n", pitch_new, pitch_angle_err, ae[0], ae[1], ae[2], ae[3]);
                                //printf("roll_new %ld pitch_new  %ld yaw_new %ld ae %3d %3d %3d %3d\n", roll_new, pitch_new, yaw_new, ae[0], ae[1], ae[2], ae[3]);
                                //printf("roll_new %ld roll_angle_err %ld ae %3d %3d %3d %3d\n", roll_new, roll_angle_err, ae[0], ae[1], ae[2], ae[3]);
			        //printf("%ld %3d %3d %3d %3d |\n ",pitch_angle_err, ae[0],ae[1],ae[2],ae[3]);		//counter++;
				nrf_gpio_pin_toggle(BLUE);


				//flash_data();
				//printf("last%lu\n", last_receiving_time);
	                 	//printf("%10ld	", get_time_us());
				//printf("%3d %3d %3d %3d | ",throttle,roll,pitch,yaw);
				//printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
				//printf("%6d %6d %6d | ", phi, theta, psi);
				//printf("%6d	%6d	%6d %ld\n	", sp, sq, sr, processed_yaw);
				//printf("%d | %4ld | %6ld |\n", bat_volt, temperature, pressure);
				//printf("%6d %6d %6d | %d || %d |||    %d  - %d | %d\n",P, P1, P2, mode, y_err, yaw, sr, raw_mode);
                        

                        }
			if(bat_volt<1050) 
			{
				mode = PANIC;
				printf("panic because of battery\n");
			}
			else if (bat_volt<1100)
			{
				printf("battery is low\n");
			}
			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{       
                //        filter_start_time = get_time_us();
			if(!raw_mode)
			     get_dmp_data();
			else if(raw_mode) 
				get_raw_sensor_data();

			run_filters_and_control();
                //      filter_stop_time = get_time_us();
		}
                //sensor_start_time = get_time_us();		  

                //loop_time = get_time_us();            
                //cycle_time = (loop_time - prev_loop_time);
				
                //prev_loop_time = loop_time;
		/*if(key_press)
		{   if(filter_stop_time  > t_access)				
			{	
				response_time = filter_stop_time - t_access;
				key_press = false;
                                printf("%ld %ld\n", response_time, cycle_time); 
			}
		}*/
		
		//printf("counter, %ld \n", counter);
        if(counter%16 == 0)
        { //     printf("%ld %ld %ld \n", response_time, cycle_time, filter_stop_time - filter_start_time);  
            
		//	 printf("cycle time: %lu \n", cycle_time);

        }



	}	

	//printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
