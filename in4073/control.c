/*------------------------------------------------------------------
 *  control.c -- here you can implement your control algorithm
 *		 and any motor clipping or whatever else
 *		 remember! motor input =  0-1000 : 125-250 us (OneShot125)
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include "math.h"

void update_motors(void)
{					
	motor[0] = ae[0];
	motor[1] = ae[1];
	motor[2] = ae[2];
	motor[3] = ae[3];
}

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	if (mode == MANUAL)
	{
		
		//float A = 1/4/b, B = 1/2/b, C = 1/4/d; 
		float A = 1, B = 1, C = 1; 
<<<<<<< HEAD
		ae[0] = (int16_t) sqrt(A*throttle-B*pitch-C*yaw);
		ae[1] = (int16_t) sqrt(A*throttle-B*roll+C*yaw);
		ae[2] = (int16_t) sqrt(A*throttle+B*roll-C*yaw);
		ae[3] = (int16_t) sqrt(A*throttle+B*roll+C*yaw);
=======
		ae[0] = (int16_t) sqrt(A*throttle-B*pitch-C*yaw)*20;
		ae[1] = (int16_t) sqrt(A*throttle-B*roll+C*yaw)*20;
		ae[2] = (int16_t) sqrt(A*throttle+B*roll-C*yaw)*20;
		ae[3] = (int16_t) sqrt(A*throttle+B*roll+C*yaw)*20;
		//printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
>>>>>>> Liang
	}
	else 
	{
		ae[0] = ae[1] = ae[2] = ae[3] = 0;
	}
	update_motors();
}

