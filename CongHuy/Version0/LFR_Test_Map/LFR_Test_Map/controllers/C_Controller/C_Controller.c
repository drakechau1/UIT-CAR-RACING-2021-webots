#include <stdio.h>
#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/distance_sensor.h>
#include <webots/led.h>
#include <webots/camera.h>
#include <webots/supervisor.h>
#define TIME_STEP 32

#define max_gs 520 
#define min_gs 110 
#define max_gs_default 1000

WbDeviceTag gs[8];

unsigned short sensors_value_org[8] = {0, 0, 0, 0, 0, 0, 0, 0};
short svc[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long pre_position = 3500;
unsigned long position = 3500;
bool  status = false;
int   check_cross = 0;
float time_meet_precross = 0;
float time_meet_cross = 0;
int   sharp_turn = 0;
bool  signal_turn_cross = false;
int   pre_circle = 0;
float time_circle = 0;
int   stop;
float time_stop = 0;

int svc_max = 0, i_max = 0, pre_imax = 0;

double find_rad(double x1, double x2, double x3, double y1, double y2, double y3)
{
  int a;
  a = (x1*y1 + x2*y2 + x3*y3)/(sqrt(pow(x1,2)+pow(x2,2)+pow(x3,2))*sqrt(pow(y1,2)+pow(y2,2)+pow(y3,2)));
  return acos(a);
}

void check_false(void)  //Hàm xử lý tín hiệu nhiễu
{    
    svc_max = 0; i_max = 0;
    for (int i = 0; i < 8; i++)
    {
        if (svc[i] > svc_max)
        {
            svc_max = svc[i];
            i_max = i;
        }
    }
    for (int i = i_max-1; i >= 0; i--)
    {
      if (svc[i] < svc[i-1]) svc[i-1] = svc[i];
    }
    for (int i = i_max+1; i < 8; i++)
    {
      if (svc[i] < svc[i+1]) svc[i+1] = svc[i];
    }
}

void read_sensors(void)    // Hàm đọc giá trị cảm biến và xử lý lấy vị trí
{
  status = false;
  unsigned long sensors_average = 0;
  unsigned int sensors_sum = 0;
    
  for(int i=0; i<8; i++)
  {
    sensors_value_org[i] = wb_distance_sensor_get_value(gs[i]);

    svc[i] = ((float)sensors_value_org[i]-min_gs)/(max_gs-min_gs)*max_gs_default;

    if(svc[i]>max_gs_default) svc[i]=max_gs_default;
    if(svc[i]<0) svc[i]=0;
  }
  
  check_false(); // Xử lý nhiễu

  for(int i = 0; i < 8; i++)
  {
    if(svc[i]>500) status = true;
    if(svc[i]>500)
    {
      sensors_average += (unsigned long)svc[i]*(i*max_gs_default);
      sensors_sum += svc[i];
    }
  }
  
  if (wb_robot_get_time() - time_meet_precross > 0.64) check_cross = 0;
  if (wb_robot_get_time() - time_meet_precross > 0.192) sharp_turn = 0;
  if (wb_robot_get_time() - time_meet_cross > 0.128) signal_turn_cross = false;
  if (wb_robot_get_time() - time_circle > 0.576 && pre_circle == 1) pre_circle = 0;
  if (wb_robot_get_time() - time_circle > 1.344) pre_circle = 0;
  if (wb_robot_get_time() - time_stop > 0.033) stop = 0;

  if(status)
  {
    if (svc[1] > 450 && svc[3] > 450 && svc[5] < 200 && svc[6] < 200 && svc[7] < 200 && pre_circle != 2) 
    { 
      sharp_turn = -1; 
      check_cross = -1;
      time_meet_precross = wb_robot_get_time();
      position = 3500;
      if (pre_circle == 2) {position = 0; pre_position = 0;}
    }
    else if (svc[4] > 450 && svc[6] > 450 && svc[0] < 200 && svc[1] < 200 && svc[2] < 200 && pre_circle != 2) 
    { 
      sharp_turn = 1; 
      check_cross = 1;
      time_meet_precross = wb_robot_get_time();
      position = 3500;
      if (pre_circle == 2) {position = 0; pre_position = 0;}
    }
    else if (svc[1] > 450 && svc[2] > 450 && svc[5] > 450 && svc[6] > 450)
    { 
      signal_turn_cross = true;
      if (check_cross == -1) position = 0;
      else if (check_cross == 1) position = 7000;
      else position = 3500;
      time_meet_cross = wb_robot_get_time();
      
      if (pre_circle == 0 || wb_robot_get_time() - time_circle > 0.224)  pre_circle ++;
      time_circle = wb_robot_get_time();
      if (pre_circle == 2) position = 0;
      
      stop ++;
      time_stop = wb_robot_get_time();
    }

    else 
    { 
      sharp_turn = 0;
      position = sensors_average/sensors_sum;
    }
  }
  else 
  {
    check_cross = 0;
    if (sharp_turn == -1 || position < 2000) {position = 0; pre_position = position;}         // Mất line rẽ trái
    else if(sharp_turn == 1 || position > 5000) {position = 7000; pre_position = position;}   // Mất line rẽ phải
    else if (position > 2500 && position < 4500) position = 3500;                             // Mất line đi thẳng
  }

}

float lfm_speed_left;
float lfm_speed_right;

long  P=0, I=0, D=0, error=0;
float PID=0;
float Kp=0.7609;         
float Ki=0.0109;      
float Kd=0.02;       

#define avg_speed 3500 //3500

void LineFollowingModule(void)   // Hàm tính toán đưa ra tốc độ hợp lí
{
  if (stop >= 3 && stop <= 9)
    {
      lfm_speed_left  = avg_speed;
      lfm_speed_right = avg_speed;
    }
    else if (signal_turn_cross) 
  {
    if (check_cross == -1)
    {
      lfm_speed_left  = -avg_speed/2;
      lfm_speed_right = avg_speed*2;
    }
    else if (check_cross == 1)
    {
      lfm_speed_left  = avg_speed*2;
      lfm_speed_right = -avg_speed/2;
    }
    else if (pre_circle == 2 && wb_robot_get_time() - time_circle < 0.033)
    {
      //printf("%f\n",wb_robot_get_time() - time_circle);
      lfm_speed_left  = -avg_speed/2;
      lfm_speed_right = avg_speed*2;
      if (wb_robot_get_time() - time_circle > 0) signal_turn_cross = false;
      
    }
  }
  else if ( ( (int)position - (int)pre_position <= 2500) && ( (int)position - (int)pre_position >= -2500) )
  {
    P = position - 3500;
    I = P + error;
    D = P - error;

    PID = Kp*P + Ki*I + Kd*D;
  
    error = P;
  
    lfm_speed_left = avg_speed + PID;
    lfm_speed_right = avg_speed - PID;
  
    if (wb_robot_get_time() > 0.3) pre_position = position;
    if (wb_robot_get_time() - time_circle >= 1.152 && wb_robot_get_time() - time_circle <= 1.216 && pre_circle == 2)
    {
      lfm_speed_left = -avg_speed/2;
      lfm_speed_right = avg_speed*2;
    }
    
  }
 
}

int main() 
{
  wb_robot_init();
  WbDeviceTag camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, 64); 
  
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
  
  float motor_left_speed = 0;
  float motor_right_speed = 0;
  
  for (;;) 
  {
     wb_robot_step(TIME_STEP);
     
     read_sensors();
     
     if (stop >= 15) 
     {
       wb_motor_set_velocity(leftmotor, 0.0);
       wb_motor_set_velocity(rightmotor, 0.0);
       wb_supervisor_simulation_set_mode(WB_SUPERVISOR_SIMULATION_MODE_PAUSE);
     }
     
     motor_left_speed = 0;
     motor_right_speed = 0;

     LineFollowingModule();
     
     motor_left_speed = lfm_speed_left;
     motor_right_speed= lfm_speed_right;
     if(!status)
    {
      if(P == -3500 || position == 0)
      {
        motor_left_speed  = -avg_speed/2;
        motor_right_speed = avg_speed*2;
      }
      if(P == 3500 || position == 7000)
      {
        motor_left_speed  = avg_speed*2;
        motor_right_speed = -avg_speed/2;
      }
    }

     float scale_factor = 0.01;
     if (check_cross != 0) scale_factor = 0.005;
     if (sharp_turn != 0) scale_factor = 0.005;
     if (pre_circle == 1) scale_factor = 0.005;
     if (wb_robot_get_time() < 0.3) 
     {
       scale_factor = 0;
     }
     //printf("%4d   %4d   %4d   %4d   %4d   %4d   %4d   %4d | %4d    %5d   %4d |LS: %3.3lf   LS: %3.3lf | %d  %2d  %2d  %2d  %2d  %2d |\n", svc[0], svc[1], svc[2], svc[3], svc[4], svc[5], svc[6], svc[7],(int)position,(int)P,(int)pre_position,scale_factor*motor_left_speed,scale_factor*motor_right_speed,status,check_cross,sharp_turn,signal_turn_cross,pre_circle,stop);
     wb_motor_set_velocity(leftmotor, scale_factor*motor_left_speed);
     wb_motor_set_velocity(rightmotor,scale_factor*motor_right_speed);
     
  };
  wb_robot_cleanup();
  return 0;
}
