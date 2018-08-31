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
Mahony *mffilter;
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

  gx = OMEGA_RAW2DEGS(data->gyro[0]);
  gy = OMEGA_RAW2DEGS(data->gyro[1]);
  gz = OMEGA_RAW2DEGS(data->gyro[2]);

  ax = ACC_RAW2G(data->acc[0]);
  ay = ACC_RAW2G(data->acc[1]);
  az = ACC_RAW2G(data->acc[2]);

  mx = MAG_RAW2UT(data->mag[0]);
  my = MAG_RAW2UT(data->mag[1]);
  mz = MAG_RAW2UT(data->mag[2]);

 //// Apply Kalman filter to estimate the posture.
  if (prev_t > 0){
     d = data->timestamp - prev_t;
     if (d < 0) { d +=256; }
     Ts = 0.01*d;

#if 0
     apply_kalman_filter(data->acc, data->gyro, data->mag, x, &yaw, P, &p, Ts);
     double pitch=correct_pitch(x[0], data->acc);
     printf("Angle   : %lf, %lf, %lf\n", yaw*57.3,pitch*57.3,x[1]*57.3);
#endif
#if 0
     mdfilter->update(gx, gy, gz, ax, ay, -az, mx, my, mz);
     //mdfilter->updateIMU(gx, gy, gz, ax, ay, -az);
     printf( "Angle(M) : %f, %f, %f \n",
	 mdfilter->getYaw(),mdfilter->getPitch(),mdfilter->getRoll());

#endif
#if 1
     //mffilter->update(gx, gy, gz, ax, ay, -az, mx, my, mz);
     mffilter->updateIMU(gx, gy, gz, ax, ay, -az);
     printf( "Angle(M) : %f, %f, %f \n",
	 mffilter->getYaw(),mffilter->getPitch(),mffilter->getRoll());
#endif
  }
  /////
  prev_t=data->timestamp;

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
  int flag=0;
  int i;

  mdfilter = new Madgwick(100, 1.0);
  mffilter = new Mahony(100, 2.0, 0.0);

  for(i=0; (n < 0 || i<n) && flag < 1000;){
    current=_shmem->current;
    if (current == prev){
      flag++;
      usleep(1000);
    }else{
      flag=0;
      print_data(i, current,  _shmem);
      prev = current;
      i++;
    }
  }
}
