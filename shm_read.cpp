/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include "RT_9A_IMU.h"
#include "Kalman.h"
#include "lib/MadgwickAHRS.h"
#include "lib/MahonyAHRS.h"
#include <getopt.h>
#include <ncurses.h>
#include <math.h>

/*
  global variables
*/
static int prev_t=-1;

static double x[2]={0.0,0.0};  // pitch, roll
static double yaw=0.0; 
static double P[4]={0.0,0.0,0.0,0.0};  // covaiance matrix
static double p=0.0; 

Madgwick *mdfilter;
Mahony *mhfilter;
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
#if 0
     d = data->timestamp - prev_t;
     if (d < 0) { d +=256; }
     Ts = 0.01*d;
     apply_kalman_filter(data->acc, data->gyro, data->mag, x, &yaw, P, &p, Ts);
     double pitch=correct_pitch(x[0], data->acc);
     mvprintw(12,10, "Angle   : %lf, %lf, %lf               ",
		 yaw*57.3,pitch*57.3,x[1]*57.3);
     mvprintw(12,60, "Ts  : %lf          ", Ts);
#endif

#if 0
     mdfilter->update(gx, gy, gz, ax, ay, -az, mx, my, mz);
     //mdfilter->updateIMU(gx, gy, gz, ax, ay, -az);
     mvprintw(13,10, "Angle(M) : %f, %f, %f               ",
	 mdfilter->getYaw(),mdfilter->getPitch(),mdfilter->getRoll());
#endif
#if 1
     //mffilter->update(gx, gy, gz, ax, ay, -az, mx, my, mz);
     mhfilter->updateIMU(gx, gy, gz, ax, ay, -az);
     mvprintw(14,10, "Angle(M) : %f, %f, %f               ",
	 mhfilter->getYaw(),mhfilter->getPitch(),mhfilter->getRoll());
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


  _shmem = (struct imu_data_shm *)map_shared_mem(shmid, sizeof(struct imu_data_shm), 0);

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

  mdfilter = new Madgwick(100, 1.0);
  mhfilter = new Mahony(100, 1.0, 0.0);

  for(i=0; (n < 0 || i<n) && flag < 1000;){
    current=_shmem->current;
    if (current == prev){
      flag++;
      usleep(1000);
    }else{
      flag=0;
      print_data(i, current,  _shmem);
      c=getch();
      if (c == 'q') break;
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
