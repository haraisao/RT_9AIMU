/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#define EXTERN	1
#include "RT_9A_IMU.h"
#include <getopt.h>
#include <sys/stat.h>
#include <math.h>

/*
 * Serial port
 */
static int cfd=0;

/*
 * Signal Handler
 */
void sighandler(int x)
{
  close(cfd);
  unlink(PID_FILE);
  exit(0);
}
/*
 * Save Process ID
 */
unsigned short save_pid()
{
  FILE *fd;
  int pid;
  fd=fopen(PID_FILE, "w");
  if (fd){
    pid = getpid();
    fprintf(fd,"%d\n", pid);
    fclose(fd);
  }else{
    syslog(LOG_ERR, "imud: failed to write pid.\n");
  }
  return (unsigned short)pid;
}

void main_loop(char *cdev, struct imu_data_shm* shm)
{
  int pid;
  char *pack;
  struct imu_data *data;
  char buf[PACKET_SIZE*2];
  struct timeval tv;
  int next=0;
  int prev_t=-1;

  cfd = open_port(cdev);
  if (cfd < 0){
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
  }
  // Save PID
  pid=save_pid();
  shm->pid = pid;

  // Main loop
  while(1){
    pack = read_packet(cfd, buf, PACKET_SIZE*2);

    if (pack != NULL){
      data = &(shm->data[next]);
      gettimeofday(&tv,NULL);

      memcpy(data, pack,PACKET_SIZE);

      /** subtract offsets **/
      for(int i=0;i<3;i++){
        data->acc[i]  -= shm->acc_off[i];
        data->gyro[i] -= shm->gyro_off[i];
        data->mag[i] -= shm->mag_off[i];
      }

      /******/
#if 0
      if (prev_t > 0){
      int d = data->timestamp - prev_t;
      if (d < 0) { d +=256; }

      shm->sp_x += (data->acc[0]*d+50)/100;
      shm->sp_y += (data->acc[1]*d+50)/100;
      shm->sp_z += (data->acc[2]*d+50)/100;

      shm->angle_x = (shm->angle_x + (data->gyro[0]*d+50)/100);
      if (shm->angle_x >2952){ shm->angle_x -= 5904; }
      else if (shm->angle_x < -2952){ shm->angle_x += 5904; }

      shm->angle_y = (shm->angle_y + (data->gyro[1]*d+50)/100);
      if (shm->angle_y >2952){ shm->angle_y -= 5904; }
      else if (shm->angle_y < -2952){ shm->angle_y += 5904; }

      shm->angle_z = (shm->angle_z + (data->gyro[2]*d+50)/100);
      if (shm->angle_z >2952){ shm->angle_z -= 5904; }
      else if (shm->angle_z < -2952){ shm->angle_z += 5904; }
      }
      prev_t=data->timestamp;
#endif
      /******/

      data->tv_sec=tv.tv_sec;
      data->tv_usec=tv.tv_usec;
      shm->current=next;

      next = NEXT_N(shm->current, MAX_POOL);
    }else{
      usleep(100);
    }
    usleep(9000);
  }
  unlink(PID_FILE);
  close(cfd);
}

/*
 *  M A I N
 */
int
main(int argc, char *argv[])
{
  char *cdev=NULL;
  char buf[PACKET_SIZE*2];
  char *pack;
  int i;
  int next=0;
  int create_flag=0;
  int shmid=SHM_ID;
  int daemon_flag=0;
  unsigned short pid;
  struct timeval tv;

  struct imu_data_shm* _shmem;

  /*
   *  Options....
   */
  struct option longopts[] = {
    { "create", no_argument, NULL, 'c'},
    { "daemon", no_argument, NULL, 'd'},
    { "help", no_argument, NULL, 'h'},
    { "port", required_argument, NULL, 'p'},
    { "shid", required_argument, NULL, 's'},
    {  0, 0, 0, 0},
  };

  int opt;
  int longindex;
  while((opt=getopt_long(argc, argv, "cdhp:s:", longopts, &longindex)) != -1){
    switch(opt){
      case 'c':
        create_flag=1;
        break;
      case 'd':
        daemon_flag=1;
        break;
      case 'h':
        printf("Usage: %s [-cd] [-p devname] [-s shmid]\n", argv[0]);
	exit(0);
	break;
      case 'p':
        cdev=optarg;
	break;
      case 's':
        shmid=atoi(optarg);
        break;
      default:
        printf("Error: Invalid option( \"%c\" )\n", opt);
        break;
    }
  }

  if(cdev == NULL){ cdev=DEFAULT_PORT; } 

  struct stat st;
  if(stat(PID_FILE, &st) == 0){
    fprintf(stderr, "imud already running.\n");
    exit(1);
  }
  /*
   * Set Signal handler
   */
  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  /*
   * Init shared memory
   */
  _shmem = (struct imu_data_shm *)map_shared_mem(shmid,
		 	 sizeof(struct imu_data_shm), create_flag);
  if (_shmem == NULL){
   fprintf(stderr, "Error in map_shared_mem\n");
   exit(-1);
  }
 // Initialize
  _shmem->current=0;
  _shmem->pid=0;

  _shmem->acc_off[0]=41;
  _shmem->acc_off[1]=-3;
  _shmem->acc_off[2]=12;

  _shmem->gyro_off[0]=-7;
  _shmem->gyro_off[1]=-1;
  _shmem->gyro_off[2]=-10;

  _shmem->mag_off[0]=52;
  _shmem->mag_off[1]=16;
  _shmem->mag_off[2]=76;

  next=0;

  /*
   * Start Daemon
   */
  if(daemon_flag == 1){
    if(daemon(0,0) == 0){ // run as deamon
      main_loop(cdev,  _shmem);
    }else{
      printf("Error : faild to start imud\n");
      return -1;
    }
  }else{
    main_loop(cdev,  _shmem);
  }
  return 0;
}


