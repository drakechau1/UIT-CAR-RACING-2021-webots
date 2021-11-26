/*
 * File:          my_c_controller_V2.c
 * Date:	  11/07/2021
 * Description:	  LFR using PID
 * Author:	  Cao Tuan Kiet
 * Modifications: none
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/motor.h>, etc.
 */
#include <webots/robot.h>
#include <stdio.h>
#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/distance_sensor.h>

#define max_gs 520 
#define min_gs 110 
#define max_gs_default 1000

#define TIME_STEP 32

WbDeviceTag gs[8];

unsigned short sensors_value_org[8] = {0, 0, 0, 0, 0, 0, 0, 0};
short sensors_value_calc[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned long position = 0;
bool status_true = false;

void read_sensors(void)
{
  status_true = false;
  unsigned long sensors_average = 0;
  unsigned int sensors_sum = 0;
    
  for(int i=0; i<8; i++)
  {
    sensors_value_org[i] = wb_distance_sensor_get_value(gs[i]);

    sensors_value_calc[i] = ((float)sensors_value_org[i]-min_gs)/(max_gs-min_gs)*max_gs_default;

    if(sensors_value_calc[i]>max_gs_default) sensors_value_calc[i]=max_gs_default;
    if(sensors_value_calc[i]<0) sensors_value_calc[i]=0;
    
    if(sensors_value_calc[i]>200) status_true = true;
    if(sensors_value_calc[i]>50)
    {
      sensors_average += (unsigned long)sensors_value_calc[i]*(i*max_gs_default);
      sensors_sum += sensors_value_calc[i];
    }
  }
  if(status_true)position = sensors_average/sensors_sum; 
  else if(position < max_gs_default*3.5)position = 0; 
  else position = 7000; 

}

int lfm_speed_left;
int lfm_speed_right;

long P=0, I=0, D=0, error=0, PID=0;
float Kp=1.8;        
float Ki=0.01009999; 
float Kd=0.02;       

#define avg_speed 3500

void LineFollowingModule(void) 
{
  P = position - 3500;
  I = P + error;
  D = P - error;

  PID = Kp*P + Ki*I + Kd*D;
  
  error = P;
  
  lfm_speed_left = avg_speed + PID;
  lfm_speed_right = avg_speed - PID;
}

/*
 * This is the main program.
 * The arguments of the main function can be specified by the
 * "controllerArgs" field of the Robot node
 */
int main() 
{
  /* necessary to initialize webots stuff */
  wb_robot_init();

  /*
   * You should declare here WbDeviceTag variables for storing
   * robot devices like this:
   *  WbDeviceTag my_sensor = wb_robot_get_device("my_sensor");
   *  WbDeviceTag my_actuator = wb_robot_get_device("my_actuator");
   */
   
  /* initialization */
  char groundsensors[7];
  for (int i = 0; i < 8; i++) 
  {
    sprintf(groundsensors, "gs%d", i);
    gs[i] = wb_robot_get_device(groundsensors); 
    wb_distance_sensor_enable(gs[i], TIME_STEP);
  }
  
  WbDeviceTag leftmotor = wb_robot_get_device("left wheel motor");
  WbDeviceTag rightmotor = wb_robot_get_device("right wheel motor");
  
  wb_motor_set_position (leftmotor, INFINITY);
  wb_motor_set_position (rightmotor, INFINITY);
  
  wb_motor_set_velocity(leftmotor, 0.0);
  wb_motor_set_velocity(rightmotor, 0.0);
  
  
  /* main loop
   * Perform simulation steps of TIME_STEP milliseconds
   * and leave the loop when the simulation is over
   */
  while (wb_robot_step(TIME_STEP) != -1) {
    /*
     * Read the sensors :
     * Enter here functions to read sensor data, like:
     *  double val = wb_distance_sensor_get_value(my_sensor);
     */
     read_sensors();
     int motor_left_speed = 0;
     int motor_right_speed = 0;

    /* Process sensor data here */
     LineFollowingModule();
     
     motor_left_speed = lfm_speed_left;
     motor_right_speed= lfm_speed_right;
     if(!status_true)
    {
      if(P == -3500)
      {
        motor_left_speed = -avg_speed;
        motor_right_speed = avg_speed;
      }
      if(P == 3500)
      {
        motor_left_speed  = avg_speed;
        motor_right_speed = -avg_speed;
      }
    }
     
     //printf("%4d   %4d   %4d   %4d   %4d   %4d   %4d   %4d   LS: %3.3lf  LS: %3.3lf \n", sensors_value_calc[0], sensors_value_calc[1], sensors_value_calc[2], sensors_value_calc[3], sensors_value_calc[4], sensors_value_calc[5], sensors_value_calc[6], sensors_value_calc[7],0.005*motor_left_speed,0.005*motor_right_speed);
  
     wb_motor_set_velocity(leftmotor, 0.005*motor_left_speed);
     wb_motor_set_velocity(rightmotor,0.005*motor_right_speed);
  };

  /* Enter your cleanup code here */

  /* This is necessary to cleanup webots resources */
  wb_robot_cleanup();

  return 0;
}
