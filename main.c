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

void main_loop(char *cdev, struct imu_data_shm* _shmem)
{
  int pid;
  char *pack;
  char buf[PACKET_SIZE*2];
  struct timeval tv;
  int next=0;

  cfd = open_port(cdev);
  if (cfd < 0){
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
  }
  // Save PID
  pid=save_pid();
  _shmem->pid = pid;

  // Main loop
  while(1){
    pack = read_packet(cfd, buf, PACKET_SIZE*2);
    if (pack != NULL){
      gettimeofday(&tv,NULL);
      memcpy(&(_shmem->data[next]), pack,PACKET_SIZE);
      _shmem->data[next].acc[0] -= _shmem->acc_off[0];
      _shmem->data[next].acc[1] -= _shmem->acc_off[1];
      _shmem->data[next].acc[2] -= _shmem->acc_off[2];

      _shmem->data[next].gyro[0] -= _shmem->gyro_off[0];
      _shmem->data[next].gyro[1] -= _shmem->gyro_off[1];
      _shmem->data[next].gyro[2] -= _shmem->gyro_off[2];

      _shmem->sp_x += _shmem->data[next].acc[0];
      _shmem->sp_y += _shmem->data[next].acc[1];
      _shmem->sp_z += _shmem->data[next].acc[2];

      _shmem->angle_x += _shmem->data[next].gyro[0];
      _shmem->angle_y += _shmem->data[next].gyro[1];
      _shmem->angle_z += _shmem->data[next].gyro[2];


      _shmem->data[next].tv_sec=tv.tv_sec;
      _shmem->data[next].tv_usec=tv.tv_usec;
      _shmem->current=next;
      next = (_shmem->current+1) % MAX_POOL;
    }else{
      usleep(100);
    }
    usleep(10000);
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
   exit(-1);
  }
  _shmem->current=0;
  _shmem->pid=0;

  _shmem->sp_x=0;
  _shmem->sp_y=0;
  _shmem->sp_z=0;

  _shmem->angle_x=0;
  _shmem->angle_y=0;
  _shmem->angle_z=0;

  _shmem->acc_off[0]=0;
  _shmem->acc_off[1]=0;
  _shmem->acc_off[2]=0;
  _shmem->gyro_off[0]=0;
  _shmem->gyro_off[1]=0;
  _shmem->gyro_off[2]=0;
  next=0;

  /*
   * Start Daemon
   */
  if(daemon_flag == 1){
    if(daemon(0,0) == 0){
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


