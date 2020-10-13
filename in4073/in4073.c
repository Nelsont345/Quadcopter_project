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

/*------------------------------------------------------------------
 * process_key -- process command keys
 *------------------------------------------------------------------
 */

uint32_t cur_time;
uint32_t write_address = 0x00000000;
uint32_t read_address = 0x00000000;
uint8_t data[DATASIZE];
uint8_t data_r[DATASIZE];
uint8_t data_r2[DATASIZE];
bool iflag = false;

void get_command(command c)
{
	
	throttle = c.throttle;
	roll = c.roll;
	pitch = c.pitch;
	yaw = c.yaw;
	mode = c.mode;
        P = c.P;
        P1 = c.P1;
        P2 = c.P2;

        flash_data();
        start_time = get_time_us();
        loop_time = start_time - c.start_time;

        queue_time = loop_time;
	//printf("get %d, %d, %d, %d",throttle,roll,pitch,yaw);
        //printf("\nqueue time: %ld\n", queue_time);
}

void flash_data()
{
   cur_time = get_time_us();
  //printf("mode = %u, P = %u, P1 = %u P2 = %u yaw_err = %d c_sr = %d, c_sp = %d, c_sq = %d, roll_new = %d, pitch_new = %d\n", mode, P, P1, P2, y_err, c_sr, c_sp, c_sq, roll_new, pitch_new);
  if(mode != 7)
//  printf("time: %10ld, throttle: %d, roll: %d, pitch: %d, yaw: %d, phi: %6d, theta: %6d, psi: %6d, sp: %6d, sq: %6d, sr: %6d, loop_time : %ld\nmode = %u, P = %u, P1 = %u P2 = %u yaw_err = %d c_sr = %d, c_sp = %d, c_sq = %d, roll_new = %d, pitch_new = %d\n\n", cur_time, throttle, roll, pitch, yaw, phi, theta, psi, sp,sq, sr, prev_loop_time, mode, P, P1, P2, y_err, c_sr, c_sp, c_sq, roll_new, pitch_new);
   //printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n\n",ae[0],ae[1],ae[2],ae[3]);
   data[0] = ((cur_time & 0xFFFFFFFF) >> 24);
   data[1] = ((cur_time & 0xFFFFFF) >> 16);
   data[2] = ((cur_time & 0xFFFF) >> 8);
   data[3] = (cur_time & 0xFF); 
   //printf("0: %d, 1: %d, 2: %d, 3: %d\n", data[0], data[1], data[2], data[3]); 

      
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

   data[29] = (prev_loop_time & 0xFFFFFFFF) >> 24;
   data[30] = (prev_loop_time & 0xFFFFFF) >> 16; 
   data[31] = (prev_loop_time & 0xFFFF) >> 8;
   data[32] = (prev_loop_time & 0xFF);
  // printf("0: %d, 1: %d, 2: %d, 3: %d, 4: %d , 5: %d , 6: %d , 7: %d , 8: %d , 9: %d , 10: %d , 11: %d , 12: %d, 13: %d, 14: %d, 15: %d, 16: %d ,  17: %d, 18: %d, 19: %d, 20: %d, 21: %d, 22: %d, 23: %d,   24: %d, 25: %d , 26: %d, 27: %d, 28: %d \n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15],  data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], data[27], data[28]); 
  if (mode!= 7) 
  { printf(" %d, %d, %d, %d, %d, %d, %d, %d ,%d ,%d , %d , %d , %d, %d, %d, %d, %d , %d, %d, %d, %d, %d, %d, %d, %d, %d , %d, %d, %d, %d, %d, %d, %d \n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15],  data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31], data[32]); }
   //flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count)

   if(!flash_write_bytes(write_address, data, DATASIZE))
   {   //printf("wrtie failed\n");
       write_address = 0x00000000;
       flash_write_bytes(write_address, data, DATASIZE);  
   }
  /* if(mode!=7){
    if(flash_read_bytes(read_address, data_r, DATASIZE))
   {       
          printf("\ndata read:  ");
          for(int i = 0; i < DATASIZE; i++)
                {
                     printf("%d ", data_r[i]);
                    
                }
         printf(" : finished reading\n\n");

   }}*/
   //if(flag)
   //   read_address += (DATASIZE * 8);
   write_address += (DATASIZE * 8);
  // flag = true;  
}

void log_data()
{   
    read_address = 0x00000000;
    printf("in log func\n");
    while(ready2 == 0)
          uart_put(0x7F);
    printf("after start byte\n");
    //uart_put(0x7F);
    while(read_address <= 0xFFFF)
    {       
            if(flash_read_bytes(read_address, data_r, DATASIZE))
            {    
                  for(int i = 0; i < DATASIZE; i++)
                  {
                          uart_put(data_r[i]);
                  }
                  nrf_delay_ms(1000);
            }
            read_address += (DATASIZE * 8);
    }
    uart_put(0x7F);
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
	mode = SAFE;
	throttle = roll = pitch = yaw = 0;
	uint32_t counter = 0;
	demo_done = false;
        last_receiving_time = get_time_us();
	while (!demo_done)
	{      // printf("%10ld | \n", get_time_us());
                tot_intr_time = 0; 
		//printf("count = %d\n",c_rx_queue.count);
		if (c_rx_queue.count){
			//printf("count = %d\n",c_rx_queue.count);
			get_command( c_dequeue(&c_rx_queue) );
                        iflag = true;
			last_receiving_time = get_time_us();
		}	
                else
		{
                        iflag = false;
			if(get_time_us()-last_receiving_time > 2000000) mode = PANIC;
		}		
                if(mode == EXIT)
                {   
                       // flash_data();
                         printf("flashed data\n");
                        log_data();
                         printf("finished logging\n");
			break;
                }

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();
			read_baro();
                        if(counter%32 == 0)
                      {
		        flash_data();
			
                 /*	printf("%10ld | ", get_time_us());
			printf("%3d %3d %3d %3d | ",throttle,roll,pitch,yaw);
			printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
			printf("%6d %6d %6d | ", phi, theta, psi);
			printf("%6d %6d %6d | ", sp, sq, sr);
			//printf("%4d | %4ld | %6ld |", bat_volt, temperature, pressure);
			printf("%6d %6d %6d | %d || %d |||    %d  - %d\n",P, P1, P2, mode, y_err, yaw, sr);
                    
                 */
                        }
                    
			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}
                loop_time += ( get_time_us() - start_time );
                //if(iflag) {printf("\nloop time: %ld, tot_intr_time: %ld, intr_start_time: %ld, intr_stop_time : %ld\n\n", loop_time, tot_intr_time, intr_start_time, intr_stop_time);
                prev_loop_time = loop_time - tot_intr_time;
                //} 
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
