/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include "RT_9A_IMU.h"
#include "butter.h"
#include "utils.h"
#include <getopt.h>
#include <math.h>
#include <signal.h>

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

int record_time=0;
int loop_flag=1;

void sig_handler(int sig)
{
  loop_flag=0;
  fprintf(stderr, "....Terminated\n");
  return;
}

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

  if (prev_t > 0){
     d = data->timestamp - prev_t;
     if (d < 0) { d +=256; }
     Ts = 0.01*d;

     double roll = DEG2RAD(shm->roll);
     double pitch = DEG2RAD(shm->pitch);
     double yaw = DEG2RAD(shm->yaw);

     shm->acc_magnitude=calc_global_acc(ax, ay, az,
                                     roll, pitch, yaw, shm->global_acc);

     printf("rpy:%f, %f, %f ", shm->roll, shm->pitch, shm->yaw);
     printf("Acc: %+f, %+f, %+f  (%+f) ",
               shm->global_acc[0],shm->global_acc[1], shm->global_acc[2],
               shm->acc_magnitude);

     record_time++;
     if (log_pose_fd){
       fprintf(log_pose_fd, "%d %d ",record_time, data->timestamp);
       fprintf(log_pose_fd, "%f, %f, %f ", shm->yaw, shm->pitch,shm->roll);
       fprintf(log_pose_fd, "%f, %f, %f %f\n",
                  shm->global_acc[0],shm->global_acc[1],shm->global_acc[2],
                  shm->acc_magnitude);
     }

#if 0
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


//     printf("Velo: %+lf, %+lf, %+lf ", vx,vy,vz);
//     printf("Dist: %+lf, %+lf, %+lf\n", dx,dy,dz);
#endif
     
  }
  /////
  prev_t=data->timestamp;

  printf("A(%+4d, %+4d, %+4d) ",data->acc[0],data->acc[1],data->acc[2]);

  printf( "G(%+4d, %+4d, %+4d) ",data->gyro[0],data->gyro[1],data->gyro[2]);

  printf( "M(%+4d, %+4d, %+4d)\n",data->mag[0],data->mag[1],data->mag[2]);

  save_data(log_fd, data, &start_tv);

  return;
}

/*
   Main...
*/
int
main(int argc, char **argv)
{
  struct imu_data_shm* _shmem;
  int current;
  int prev;
  int n=-1;
  int shmid=SHM_ID;
  int i;
  char *logdir=NULL;

  /*
   *  Options....
   */
  struct option longopts[] = {
    { "num", required_argument, NULL, 'n'},
    { "shid", required_argument, NULL, 's'},
    { "logfile", required_argument, NULL, 'l'},
    {  0, 0, 0, 0},
  };

  int opt;
  int longindex;
  while((opt=getopt_long(argc, argv, "hl:n:s:", longopts, &longindex)) != -1){
    switch(opt){
      case 'n':
        n=atoi(optarg);
	break;
      case 's':
        shmid=atoi(optarg);
        break;
      case 'l':
        logdir=optarg;
        break;

      case 'h':
      default:
        printf("Usage: %s [-n num] [-s shmid] [-l log_dir]\n", argv[0]);
	exit(0);
    }
  }

  signal(SIGINT, sig_handler);

  /*
     Map shared memory
   */
  _shmem = MAP_SHM(struct imu_data_shm, shmid, 0);

  /*
   *  M A I N
   */
  gettimeofday(&start_tv, NULL);
  vx=vy=vz=0.0; 
  dx=dy=dz=0.0; 
  record_time=0;

  if(logdir){ /* Open logfiles */
    log_fd=open_logfile(logdir,"imu_data");
    log_pose_fd=open_logfile(logdir,"pose_data");
  }
  for(i=0; (n < 0 || i<n) && loop_flag == 1; i++){
    current=_shmem->current;
    if (current == prev){
      usleep(1000);
    }else {
      print_data(i, current,  _shmem);
      prev = current;
    }
  }
  /* Close logfiles */
  if(log_fd){ close_logfile(log_fd); }
  if(log_pose_fd){ close_logfile(log_pose_fd); }
}
