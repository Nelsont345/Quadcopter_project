
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

int16_t fp_mul(int8_t a, int8_t b, int8_t n)
{
   return (a * b) >> n;
}
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
                 
		int32_t A = 1, B = 1, C = 1; 
		ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch - C * yaw) * 20;
		ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll + C * yaw) * 20;
		ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch - C * yaw) * 20;
		ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll + C * yaw) * 20;
		//printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
	}
	else if (mode == PANIC)
	{
		int AE = 200;
		for(int i = 0; i<10;i++);
		{
			nrf_delay_ms(200);
			ae[0]=ae[1]=ae[2]=ae[3] = AE;
			AE = AE- 20;
		}
		mode = SAFE;
	}

	else if (mode == CALIBRATION)
	{
	    cal_phi = phi;
            cal_theta = theta;
            cal_psi = psi;
            cal_sp = sp;
            cal_sq = sq;
            cal_sr = sr;
	    //printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
	}
   
        else if(mode == YAW)
        {  int8_t yaw_new;
           //int8_t x, y;
           int32_t A = 1, B = 1, C = 1;
           y_err = yaw - (sr - cal_sr);
           if((y_err > 10) || (y_err < -10))
           {   
               y_err = yaw - (sr - cal_sr);
               //yaw_new = yaw + fp_mul(P, y_err, 0);
               yaw_new = yaw - P * y_err;
	       ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch - C * yaw_new) * 20;
	       ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll  + C * yaw_new) * 20;
	       ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch - C * yaw_new) * 20;
	       ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll  + C * yaw_new) * 20;
           }             
        }	
 
        else if(mode == FULL)
        {nrf_delay_ms(100);
           int8_t pitch_new, roll_new;           
	   int32_t A = 1, B = 1, C = 1;   
           p_err = 0 - theta;
           r_err = 0 - psi;
           prate_err = pitch - sq;
           rrate_err = roll - sp;
           if(p_err > 10 || r_err > 10 || p_err < -10 || r_err < -10)
           {
               if(prate_err > 3 || rrate_err > 3 || prate_err < -3 || rrate_err < -3 )
               {
                   pitch_new = pitch + fp_mul(P2, prate_err, 0);
                   roll_new  = roll  + fp_mul(P2, rrate_err, 0);
	           ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch_new - C * yaw) * 20;
	           ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll_new  + C * yaw) * 20;
	           ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch_new - C * yaw) * 20;
	           ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll_new  + C * yaw) * 20;
                   prate_err = pitch - (sq - cal_sq);
                   rrate_err = roll - (sp - cal_sp);
               } 
               pitch_new = pitch + P1 * p_err;
               roll_new  = roll  + P1 * r_err;
	       ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch_new - C * yaw) * 20;
	       ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll_new  + C * yaw) * 20;
	       ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch_new - C * yaw) * 20;
	       ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll_new  + C * yaw) * 20;     
               p_err = cal_theta - (theta - cal_theta);
               r_err = cal_psi - (psi - cal_psi);          
           } 
        }

        else 
	{
		ae[0] = ae[1] = ae[2] = ae[3] = 0;
                cal_sp = cal_sq = cal_sr = 0;
	}
	update_motors();
}
