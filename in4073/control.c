
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

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc
	if (mode == MANUAL)
	{
		
		//float A = 1/4/b, B = 1/2/b, C = 1/4/d;
                 
		int32_t A = 1, B = 1, C = 1; 
		ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch - C * yaw)*1.5;
		ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll + C * yaw)*1.5;
		ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch - C * yaw)*1.5;
		ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll + C * yaw)*1.5;
		//printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
	}
	else if (mode == PANIC)
	{
		int i;
		for(i=0;i<10;i++)
		{
			ae[0] *=0.9;
			ae[1] *=0.9;
			ae[2] *=0.9;
			ae[3] *=0.9;
			update_motors();
			nrf_delay_ms(1000);
			//printf("ae1 = %d ae2 = %d ae3 = %d ae4 = %d\n",ae[0],ae[1],ae[2],ae[3]);
		}
                
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
           {     printf("calibrated. return to safe mode\n");
                 c_phi   = cal_phi / 128;
                 c_theta = cal_theta / 128;
                 c_psi   = cal_psi / 128;
                 c_sp    = cal_sp / 128;
                 c_sq    = cal_sq / 128;
                 c_sr    = cal_sr / 128;
                 cal_count = 0;
		 mode = SAFE;
           }
	}
   
        else if(mode == YAW)
        {  int16_t yaw_new;
           //int8_t x, y;
           int32_t A = 1, B = 1, C = 1; 
           y_err = yaw - (sr - c_sr);
           if(y_err > 5 || y_err < -5)
           {   
               
               yaw_new = P * y_err;
               //pitch = 0;
               //roll = 0;
	       ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch - C * yaw_new)*1.5;
	       ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll  + C * yaw_new)*1.5;
	       ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch - C * yaw_new)*1.5;
	       ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll  + C * yaw_new)*1.5;
           }             
        }	
 
        else if(mode == FULL)
        {
           //int16_t p_err, r_err, prate_err, rrate_err;           
	   int32_t A = 1, B = 1, C = 1;   
           
           //p_err = 0 - theta;
           //r_err = 0 - psi;
          // prate_err = pitch - (sq - c_sq);
          // rrate_err = roll - (sp - c_sp);
          // yaw = 0;

           pitch_new = P1 * (0 - (theta - c_theta)) - P2 * (sq - c_sq);
           roll_new =  P1 * (0 - (phi - c_phi))   - P2 * (sp - c_sp);
           
          ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch_new - C * yaw) * 1.5;
	  ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll_new  + C * yaw) * 1.5;
	  ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch_new - C * yaw) * 1.5;
	  ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll_new  + C * yaw) * 1.5;

          /*while(p_err > 10 || r_err > 10)
           {
               while(prate_err > 3 || rrate_err > 3)
               {
                   pitch_new = fp_mul(P2, prate_err, 0);
                   roll_new  = fp_mul(P2, rrate_err, 0);
	           ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch_new - C * yaw) * 1.5;
	           ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll_new  + C * yaw) * 1.5;
	           ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch_new - C * yaw) * 1.5;
	           ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll_new  + C * yaw) * 1.5;
                   prate_err = pitch - (sq - c_sq);
                   rrate_err = roll - (sp - c_sp);
               } 
               pitch_new = pitch + P1 * p_err;
               roll_new  = roll  + P1 * r_err;
	       ae[0] = (int16_t) sqrt(A * throttle + 2 * B * pitch_new - C * yaw) * 1.5;
	       ae[1] = (int16_t) sqrt(A * throttle - 2 * B * roll_new  + C * yaw) * 1.5;
	       ae[2] = (int16_t) sqrt(A * throttle - 2 * B * pitch_new - C * yaw) * 1.5;
	       ae[3] = (int16_t) sqrt(A * throttle + 2 * B * roll_new  + C * yaw) * 1.5;     
               p_err = c_theta - theta;
               r_err = c_psi - psi;          
           } */
        }

        else 
	{
		ae[0] = ae[1] = ae[2] = ae[3] = 0;
	}
	update_motors();
}
