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

void get_command(command c)
{
	
	throttle = c.throttle;
	roll = c.roll;
	pitch = c.pitch;
	yaw = c.yaw;
	mode = c.mode;
	float A = 1, B = 1, C = 1; 
	ae[0] = (int16_t) sqrt(A*throttle-B*pitch-C*yaw)*20;
	ae[1] = (int16_t) sqrt(A*throttle-B*roll+C*yaw)*20;
	ae[2] = (int16_t) sqrt(A*throttle+B*roll-C*yaw)*20;
	ae[3] = (int16_t) sqrt(A*throttle+B*roll+C*yaw)*20;
        flash_data();
	printf("get %u, %d, %d, %d, %u, %u\n", c.throttle, c.roll, c.pitch, c.yaw, c.mode, c.frame);
	printf("mode = %u, ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",mode, ae[0],ae[1],ae[2],ae[3]);
}

void flash_data()
{
   cur_time = get_time_us();
   printf("time: %10ld, M: %d, T: %d, L: %d, N: %d, phi: %6d, theta: %6d, psi: %6d, sp: %6d, sq: %6d, sr: %6d\n\n", cur_time, throttle, roll, pitch, yaw, phi, theta, psi, sp,sq, sr);
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

   //flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count)

   if(!flash_write_bytes(write_address, data, DATASIZE))
   {   printf("wrtie failed\n");
       write_address = 0x00000000;
       flash_write_bytes(write_address, data, DATASIZE);  
   }

    if(flash_read_bytes(read_address, data_r, DATASIZE))
   {       
          printf("\ndata read:  ");
          for(int i = 0; i < DATASIZE; i++)
                {
                     printf("%d ", data_r[i]);
                    
                }
         printf(" : finieshed reading\n\n");

   }
   read_address += (DATASIZE * 8);
   write_address += (DATASIZE * 8);  
}

void log_data()
{   
    read_address = 0x00000000;
    uart_put(0x7F);
  // printf("loglogloglog");
    while(read_address <= 0xFFFF)
    {    //printf("read_address: %ld\n\n", read_address);
          if(flash_read_bytes(read_address, data_r2, DATASIZE))
          {    
    //            printf("\ndata read in log:  ");
                for(int i = 0; i < DATASIZE; i++)
                {
                     printf("%d ", data_r2[i]);
                     uart_put(data_r2[i]);
                }
                //printf("\n");
      //          printf(" : finieshed reading in log\n\n");
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

	while (!demo_done)
	{
		//printf("count = %d\n",c_rx_queue.count);
		if (c_rx_queue.count){
			printf("count = %d\n",c_rx_queue.count);
			get_command( c_dequeue(&c_rx_queue) );
		}			

                if(mode == 7)
                {
                        flash_data();
                        log_data();
                }

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();
			read_baro();

			//printf("%10ld | ", get_time_us());
			//printf("%3d %3d %3d %3d | ",throttle,roll,pitch,yaw);
			//printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
			//printf("%6d %6d %6d | ", phi, theta, psi);
			//printf("%6d %6d %6d | ", sp, sq, sr);
			//printf("%4d | %4ld | %6ld \n", bat_volt, temperature, pressure);

			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
