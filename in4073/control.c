
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
#define P2PHI_SHIFT 8
#define C1_SHIFT 7
#define C2_SHIFT 20


uint16_t isqrt(long x)
{
	/*
	 *	Logically, these are unsigned. We need the sign bit to test
	 *	whether (op - res - one) underflowed.
	 */
	if(x<0) return 0;
	int op, res, one;

	op = x;
	res = 0;

	/* "one" starts at the highest power of four <= than the argument. */

	one = 1 << 30;	/* second-to-top bit set */
	while (one > op) one >>= 2;

	while (one != 0) {
		if (op >= res + one) {
			op = op - (res + one);
			res = res +  2 * one;
		}
		res /= 2;
		one /= 4;
	}
	return(res);
}



uint8_t cal_count = 0;
int16_t fp_mul(int8_t a, int8_t b, int8_t n)
{
   return (a * b) >> n;
}
void update_motors(void)
{					
	motor[0] = ae[0]>0?ae[0]:0;
	motor[1] = ae[1]>0?ae[1]:0;
	motor[2] = ae[2]>0?ae[2]:0;
	motor[3] = ae[3]>0?ae[3]:0;
}


int butterworth(int32_t x0, int32_t x1, int32_t x2, int32_t y1, int32_t y2)
{
    int32_t a[6]; 
    int32_t b[6];
    //a[0] = 16384; a[1] = -28672; a[2] = 14336; b[0] = 66; b[1] = 136; b[2] = 66; //actual values
	//a[0] = 14; a[1] = 0; a[2] = 15; a[3] = 10; a[4] = 14; a[5] = 10; //shifts
    //b[0] = 3; b[1] = 1; b[2] = 4; b[3] = 2; b[4] = 3; b[5] = 1;  

    a[0] = 14; a[1] = 0; a[2] = 15; a[3] = 12; a[4] = 14; a[5] = 11; //shifts
    b[0] = 6; b[1] = 1; b[2] = 7; b[3] = 3; b[4] = 6; b[5] = 1;      
    int32_t part1 = (x0<<b[0]) + (x0<<b[1]) + (x1<<b[2]) + (x1<<b[3]) + (x2<<b[4]) +(x2<<b[5]);
    int32_t part2 = (-y1<<a[2]) + (y1<<a[3]) + (y2<<a[4]) - (y2<<a[5]);
    int32_t filtered = (part1 - part2) >>14;  //a[0] = 1 or 16384, should be divided or shifted right 
    return filtered;
}

void kalman_filter()
{
	sphi = say << 16;  
    p_kalman = (sp << 16) - p_bias;   
    phi_kalman = phi_kalman + (p_kalman >> P2PHI_SHIFT);
    phi_error = phi_kalman - sphi;
    phi_kalman = phi_kalman - (phi_error >> C1_SHIFT);// C1;
    p_bias = p_bias + ((phi_error << P2PHI_SHIFT)>>C2_SHIFT);// C2;
	phi = phi_kalman>>16;

	stheta = sax << 16;  
    q_kalman = (sq << 16) - q_bias;   
    theta_kalman = theta_kalman + (q_kalman >> P2PHI_SHIFT);
    theta_error = theta_kalman - stheta;
    theta_kalman = theta_kalman - (theta_error >> C1_SHIFT);// C1;
    q_bias = q_bias + ((theta_error << P2PHI_SHIFT)>>C2_SHIFT);// C2;
	theta = theta_kalman>>16;
}
void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
        if(raw_mode)
        {

			if(mode == YAW || mode == FULL)
			{
				processed_yaw = butterworth(sr, prev_yaw_x[0], prev_yaw_x[1], prev_yaw_y[0], prev_yaw_y[1]);	
				prev_yaw_x[1] = prev_yaw_x[0];
				prev_yaw_x[0] = sr;	
				prev_yaw_y[1] = prev_yaw_y[0];
				prev_yaw_y[0] = processed_yaw;
				sr = processed_yaw;
					
			}
			if(mode == FULL)   
			{
				kalman_filter();  
			}	  		  	
        }

	if(height_mode)
	{
		int32_t h_err;
		uint32_t Q = 2;
		h_err = pressure - fixed_pressure;                  
               	throttle_new = Q * h_err;
	       	if(throttle_new > 65535) throttle_new = 65535;
	       	if(throttle_new < 0) throttle_new = 0;           	
		throttle = throttle_new;
	}



	if (mode == MANUAL)
	{
		
		//float A = 1/4/b, B = 1/2/b, C = 1/4/d;
                 
		int32_t A = 1, B = 1, C = 1; 
		ae[0] = (int16_t) isqrt(A * throttle + 2 * B * pitch - C * yaw)*0.7+120;
		ae[1] = (int16_t) isqrt(A * throttle - 2 * B * roll + C * yaw)*0.7+120;
		ae[2] = (int16_t) isqrt(A * throttle - 2 * B * pitch - C * yaw)*0.7+120;
		ae[3] = (int16_t) isqrt(A * throttle + 2 * B * roll + C * yaw)*0.7+120;
		//printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
	}
	else if (mode == PANIC)
	{       
		printf("panic\n");

		for(int i=0;i<10;i++)
		{
			ae[0] *=0.9;
			ae[1] *=0.9;
			ae[2] *=0.9;
			ae[3] *=0.9;
			update_motors();
			nrf_delay_ms(500);
			//printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
		}
		uart_put(0xFD);              
		mode = SAFE;
	}
	else if (mode == CALIBRATION)
	{  
           
		cal_count++;
		// take average readings over 100 values 
		if(cal_count < 128)
	   { 
			cal_phi   += phi;
			cal_theta += theta;
			cal_psi   += psi;
			cal_sp    += sp;
			cal_sq    += sq;
			cal_sr    += sr;
		}
		else if(cal_count == 128)
		{   
			printf("calibrated. return to safe mode\n");
			c_phi   = cal_phi / 128;
			c_theta = cal_theta / 128;
			c_psi   = cal_psi / 128;
			c_sp    = cal_sp / 128;
			c_sq    = cal_sq / 128;
			c_sr    = cal_sr / 128;
			cal_count = 0;
			uart_put(0xFC);  
			mode = SAFE;
		}
	}
   
	else if(mode == YAW)
	{  
	   int32_t yaw_new = 0;
           //int8_t x, y;
           int32_t A = 1, B = 1, C = 1; 
           y_err = yaw - (sr - c_sr);
           if(y_err > 100 || y_err < -100)
           {
               yaw_new = P * y_err;
	       if(yaw_new>32767) yaw_new = 32767;
	       if(yaw_new<-32768) yaw_new = -32768;
           }
           ae[0] = (int16_t) isqrt(A * throttle + 2 * B * pitch - C * yaw_new)*0.7+160;
	   ae[1] = (int16_t) isqrt(A * throttle - 2 * B * roll  + C * yaw_new)*0.7+160;
	   ae[2] = (int16_t) isqrt(A * throttle - 2 * B * pitch - C * yaw_new)*0.7+160;
	   ae[3] = (int16_t) isqrt(A * throttle + 2 * B * roll  + C * yaw_new)*0.7+160;       
	}	

 
	else if(mode == FULL)
	{
             int16_t p_err, r_err;
                  
	     int32_t A = 1, B = 1, C = 1;   
           
             p_err = pitch - (theta - c_theta);
             r_err = roll - (phi   - c_phi);
           
            if(p_err > 100 || p_err < -100 || r_err < -100 || r_err > 100)
            { 
                pitch_new = P2 * P1 * p_err - P2 * (sp - c_sp);
                roll_new = P2 * P1 * r_err - P2 * (sq - c_sq);
                if(pitch_new < -32768) pitch_new = -32768;
                if(pitch_new > 32767) pitch_new = 32767;
                if(roll_new < -32768) roll_new = -32768;
                if(roll_new > 32767) roll_new = 32767;
	}
          
          ae[0] = (int16_t) isqrt(A * throttle + 2 * B * pitch_new - C * yaw)*0.7 + 160;
	  ae[1] = (int16_t) isqrt(A * throttle - 2 * B * roll_new  + C * yaw)*0.7 + 160;
	  ae[2] = (int16_t) isqrt(A * throttle - 2 * B * pitch_new - C * yaw)*0.7 + 160;
	  ae[3] = (int16_t) isqrt(A * throttle + 2 * B * roll_new  + C * yaw)*0.7 + 160;
           
        }

	else if(mode == SAFE) 
	{
		ae[0] = ae[1] = ae[2] = ae[3] = 0;
	}
	update_motors();
}
