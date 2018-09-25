/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include "RT_9A_IMU.h"
#include "KalmanFilter.h"
#include "MadgwickAHRS.h"
#include "MahonyAHRS.h"
#include "complementary_filter.h"
//#include "my_filter.h"
#include <getopt.h>
#include <ncurses.h>
#include <math.h>

/*
  global variables
*/
static int prev_t=-1;

static double vx=0.0; 
static double vy=0.0; 
static double vz=0.0; 

static double dx=0.0; 
static double dy=0.0; 
static double dz=0.0; 

FILE *log_fd=NULL;
FILE *log_pose_fd=NULL;

struct timeval start_tv;

Madgwick *mdfilter;
Mahony *mhfilter;
imu_tools::ComplementaryFilter *cfilter;

int record_time=0;

/*

*/
void print_data(int i, int current, struct imu_data_shm* shm)
{
  imu_data *data;
  int d;
  double Ts;
  float gx, gy, gz;
  float ax, ay, az;
  float mx, my, mz;

  data = &(shm->data[current]);

  mvprintw(1,10, "==== RT-9A IMU (%d)=====", i);
  mvprintw(3,10, "current : %4d (%3d)", current, data->timestamp);
  mvprintw(4,10, "ACC_Off : %4d %4d %4d",
	  shm->acc_off[0], shm->acc_off[1], shm->acc_off[2]);
  mvprintw(5,10, "GYRO_Off : %4d %4d %4d",
	  shm->gyro_off[0], shm->gyro_off[1], shm->gyro_off[2]);
  mvprintw(6,10, "MAG_Off : %4d %4d %4d",
	  shm->mag_off[0], shm->mag_off[1], shm->mag_off[2]);

  /*** Convert ***/
  gx = OMEGA_RAW2DEGS(data->gyro[0]);
  gy = OMEGA_RAW2DEGS(data->gyro[1]);
  gz = OMEGA_RAW2DEGS(data->gyro[2]);

  ax = ACC_RAW2G(data->acc[0]);
  ay = ACC_RAW2G(data->acc[1]);
  az = ACC_RAW2G(data->acc[2]);

  mx = MAG_RAW2UT(data->mag[0]);
  my = MAG_RAW2UT(data->mag[1]);
  mz = MAG_RAW2UT(data->mag[2]);

  mvprintw(7,10, "Temp    : %2.3f     \n", TEMP_RAW2DEG(data->templature));
  mvprintw(8,10, "Mag     : %+3.2f, %+3.2f, %+3.2f        ", mx, my, mz);

  mvprintw(9,50, "Direction (h) : %lf          ", round(atan2(mx, my) * 57.3));

  mvprintw(10,50,"Direction (v) : %lf          ",
		    round(atan2(mz, sqrt(mx*mx+my*my)) *57.3));


 //// Apply Kalman filter to estimate the posture.
  if (prev_t > 0){
     d = data->timestamp - prev_t;
     if (d < 0) { d +=256; }
     Ts = 0.01*d;

     mvprintw(11,60, "Ts  : %lf          ", Ts);

#if 1
     double roll = -DEG2RAD(shm->roll);
     double pitch = DEG2RAD(shm->pitch);
     double yaw = -DEG2RAD(shm->yaw);
     double cph, cth, cps, sph, sth, sps;
     double a_x, a_y, a_z;

     cph=cos(yaw);
     sph=sin(yaw);
     cth=cos(pitch);
     sth=sin(pitch);
     cps=cos(roll);
     sps=sin(roll);
     a_x = cph*cth*ax + (cph*sth*sps-sph*cps)*ay + cph*sth*cps*az+sph*sps*az;
     a_y = sph*cth*ax + (sph*sth*sps+cph*cps)*ay + (sph*sth*cps-cph*sps)*az;
     a_z = -sth*ax + cth*sps*ay + cth*cps*az + 1;

     shm->global_acc[0]=a_x;
     shm->global_acc[1]=a_y;
     shm->global_acc[2]=a_z;

     mvprintw(10,10, "rpy   : %f, %f, %f",
                       roll, pitch, yaw);

     record_time++;
     if (log_pose_fd){
       fprintf(log_pose_fd, "%d %d ",record_time, data->timestamp);
       fprintf(log_pose_fd, "%lf, %lf, %lf ", yaw*57.3,pitch*57.3,roll*57.3);
       fprintf(log_pose_fd, "%lf, %lf, %lf\n", a_x,a_y,a_z);
     }

     double acc_mag;
     acc_mag = sqrt(a_x*a_x+a_y*a_y+a_z*a_z);
     //acc_mag = sqrt(ax*ax+ay*ay+az*az);
     shm->acc_magnitude=acc_mag;
     mvprintw(12,10, "Acc  : %+lf, %+lf, %+lf  (%+lf)  ",
                  a_x, a_y, a_z, acc_mag);

     {
       vx += a_x*Ts*9.8; 
       vy += a_y*Ts*9.8; 
       vz += a_z*Ts*9.8; 
       if(fabs(vx) < 0.005) { vx=0.0;}
       if(fabs(vy) < 0.005) { vy=0.0;}
       if(fabs(vz) < 0.005) { vz=0.0;}

     }

     dx=dx+ vx*Ts;
     dy=dy+ vy*Ts;
     dz=dz+ vz*Ts;


//     mvprintw(13,10, "Velo  : %+lf, %+lf, %+lf               ", vx,vy,vz);
//     mvprintw(14,10, "Dist  : %+lf, %+lf, %+lf               ", dx,dy,dz);
     
#endif
  }
  /////
  prev_t=data->timestamp;

  mvprintw(16,10, "Acc     : %+4d, %+4d, %+4d             ",
	 data->acc[0], data->acc[1] , data->acc[2]);

  mvprintw(17,10, "Gyro    : %+4d, %+4d, %+4d           ",
	 data->gyro[0], data->gyro[1] , data->gyro[2]);

  mvprintw(18,10, "Mag    : %+4d, %+4d, %+4d           ",
	 data->mag[0], data->mag[1] , data->mag[2]);

  save_data(log_fd, data, &start_tv);

  refresh();
  return;
}


int main(int argc, char **argv)
{
  struct imu_data_shm* _shmem;
  int current;
  int prev;
  int n=-1;
  int shmid=SHM_ID;
  char c;

  /*
   *  Options....
   */
  struct option longopts[] = {
    { "num", required_argument, NULL, 'n'},
    { "shid", required_argument, NULL, 's'},
    {  0, 0, 0, 0},
  };

  int opt;
  int longindex;
  while((opt=getopt_long(argc, argv, "hn:s:", longopts, &longindex)) != -1){
    switch(opt){
      case 'h':
        printf("Usage: %s [-n num] [-s shmid]\n", argv[0]);
	exit(0);
	break;
      case 'n':
        n=atoi(optarg);
	break;
      case 's':
        shmid=atoi(optarg);
        break;
      default:
        printf("Error: Invalid option( \"%c\" )\n", opt);
        break;
    }
  }


  //_shmem = (struct imu_data_shm *)map_shared_mem(shmid, sizeof(struct imu_data_shm), 0);
  //_shmem = map_imu_shm(shmid, 0);
  _shmem = MAP_SHM(struct imu_data_shm, shmid, 0);


  /*
   *
   */
  initscr();
  raw();
  //cbreak();
  timeout(0);
  noecho();

  int flag=0;
  int i;

  mdfilter = new Madgwick(100, 0.6);
  mhfilter = new Mahony(100, 1.0, 0.0);
  cfilter = new imu_tools::ComplementaryFilter();
  //myfilter_ax = new MyFilter(0.01, 0);
  //myfilter_ay = new MyFilter(0.01, 0);
  //myfilter_az = new MyFilter(0.01, 0);
  gettimeofday(&start_tv, NULL);

  for(i=0; (n < 0 || i<n) && flag < 1000;){
    current=_shmem->current;
#if 1
    if (current == prev){
 //     flag++;
      usleep(1000);
      c=getch();
      if (c == 'q') break;
    }else
#endif
    {
      flag=0;
      print_data(i, current,  _shmem);
      c=getch();
      if (c == 'q') break;
      else if (c == 'v'){
        vx=vy=vz=0.0; 
        dx=dy=dz=0.0; 
      }else if (c == 's'){
        record_time=0;
        log_fd=open_logfile("logs","imu_data");
        log_pose_fd=open_logfile("logs","pose_data");

      }else if (c == 'e'){
        close_logfile(log_fd);
        close_logfile(log_pose_fd);
      }
      prev = current;
      i++;
    }
  }
  timeout(-1);
  mvprintw(13,10, "...End. Please push any key");
  refresh();
  getch();
  endwin();
}
