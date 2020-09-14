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

void get_command(command c)
{
	//printf("get %u %d, %d, %d, %u\n", c.throttle c.roll, c.pitch, c.yaw, c.mode);
	throttle = c.throttle;
	roll = c.roll;
	pitch = c.pitch;
	yaw = c.yaw;
	mode = c.mode;
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
		//printf("%d",myrx_queue.count);
		if (myrx_queue.count) get_command( mydequeue(&myrx_queue) );

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