/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#define EXTERN	1
#include "RT_9A_IMU.h"
#include "config.h"
#include <getopt.h>
#include <sys/stat.h>
#include <math.h>

#include "Kalman.h"
#include "lib/MadgwickAHRS.h"
#include "lib/MahonyAHRS.h"
#include "lib/complementary_filter.h"

/*
  Filter
*/
Madgwick *mdfilter;
Mahony *mhfilter;
imu_tools::ComplementaryFilter *cfilter;


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

/*
  Filter
*/

static double x[2]={0.0,0.0};  // pitch, roll
static double P[4]={0.0,0.0,0.0,0.0};  // covaiance matrix
static double yaw=0.0;
static double p=0.0;

void apply_filter(struct imu_data_shm *shm, struct imu_data *data, char *typ)
{
  double Ts;

  Ts = 0.01;

  if(!strcmp(typ, "Kalman")){
    apply_kalman_filter(data->acc,data->gyro,data->mag,x,&yaw,P,&p,Ts,0);

    shm->pitch=RAD2DEG(correct_pitch(x[0], data->acc));
    shm->roll=RAD2DEG(x[1]);
    shm->yaw=RAD2DEG(yaw);

  }else if(!strcmp(typ, "Madgwick")){
    mdfilter->updateIMU(data->gyro[0], data->gyro[1], data->gyro[2],
		data->acc[0], data->acc[1], -data->acc[2]);

    shm->yaw=mdfilter->getYaw();
    shm->pitch=mdfilter->getPitch();
    shm->roll=mdfilter->getRoll();

  }else if(!strcmp(typ, "Mahony")){
    mhfilter->updateIMU(data->gyro[0], data->gyro[1], data->gyro[2],
		data->acc[0], data->acc[1], -data->acc[2]);

    shm->yaw=mhfilter->getYaw();
    shm->pitch=mhfilter->getPitch();
    shm->roll=mhfilter->getRoll();

  }else if(!strcmp(typ, "Complementary")){
    //cfilter->updateIMU(gx, gy, gz, ax, ay, -az);

  }else{

  }

  return;
}

/*
   Main Loop
  
*/
void main_loop(char *cdev, struct imu_data_shm* shm, char *typ)
{
  int pid;
  char *pack;
  struct imu_data *data;
  char buf[PACKET_SIZE*2];
  struct timeval tv;
  int next=0;
  int prev_t=-1;
  int i;

  cfd = open_port(cdev);
  if (cfd < 0){
#if 0
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
#endif
    shm->status &= 0xfe;
  }else{
    shm->status |= 0x01;
  }
  // Save PID
  pid=save_pid();
  shm->pid = pid;

  // Main loop
  while(1){
    if(cfd < 0){
        cfd = open_port(cdev);
        if (cfd < 0){
          shm->status &= 0xfe;
          sleep(1);
        }else{
          shm->status |= 0x01;
        }
        continue;
    }
    
    pack = read_packet(cfd, buf, PACKET_SIZE*2);

    if (pack != NULL){
      data = &(shm->data[next]);
      gettimeofday(&tv,NULL);

      memcpy(data, pack,PACKET_SIZE);

      /** subtract offsets **/
      for(i=0;i<3;i++){
        data->acc[i]  -= shm->acc_off[i];
        data->gyro[i] -= shm->gyro_off[i];
        data->mag[i] -= shm->mag_off[i];
      }

      /******/

      apply_filter(shm, data, typ);

      data->tv_sec=tv.tv_sec;
      data->tv_usec=tv.tv_usec;
      shm->current=next;

      next = NEXT_N(shm->current, MAX_POOL);
    }else{
      fprintf(stderr, "Error in read packet\n");
      usleep(100);
      if(cfd > 0){
        close(cfd);
        cfd = -1;
        shm->status &= 0xfe;
      }
    }
    usleep(9000);
  }
  unlink(PID_FILE);
  if(cfd > 0){ close(cfd); }
}


/*
 *  M A I N
 */
int
main(int argc, char *argv[])
{
  char *cdev=NULL;
  //char buf[PACKET_SIZE*2];
  char *pack;
  int i;
  int next=0;
  int create_flag=0;
  int shmid = -1;
  int daemon_flag=0;
  unsigned short pid;
  struct timeval tv;

  struct imu_data_shm* _shmem;
  const char *config_file="rt_imu.conf";

  short acc_x_off, acc_y_off, acc_z_off;
  short gyro_x_off, gyro_y_off, gyro_z_off;
  short mag_x_off, mag_y_off, mag_z_off;

  char *filter_type=NULL;

  /*
   *  Options....
   */
  struct option longopts[] = {
    { "create", no_argument, NULL, 'c'},
    { "daemon", no_argument, NULL, 'd'},
    { "help", no_argument, NULL, 'h'},
    { "port", required_argument, NULL, 'p'},
    { "shid", required_argument, NULL, 's'},
    { "config", required_argument, NULL, 'f'},
    {  0, 0, 0, 0},
  };

  int opt;
  int longindex;
  while((opt=getopt_long(argc, argv, "cdf:hp:s:", longopts, &longindex)) != -1){
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
      case 'f':
        config_file=optarg;
        break;
      default:
        printf("Error: Invalid option( \"%c\" )\n", opt);
        break;
    }
  }

  /****/
  struct configuration *config=load_config_file(config_file);

  if (config){
    char *val;

    if(cdev == NULL){ cdev=get_value(config, "device"); }
    if(shmid < 0){
      val = get_value(config, "shmem_id");
      if(val != NULL){ shmid=atoi(val); }
    }

    val = get_value(config, "acc_off");
    if(val){ sscanf(val, "%hd %hd %hd", &acc_x_off, &acc_y_off, &acc_z_off); }
    else{ acc_x_off=acc_y_off=acc_z_off=0;}

    val = get_value(config, "gyro_off");
    if(val){ sscanf(val, "%hd %hd %hd",&gyro_x_off,&gyro_y_off,&gyro_z_off); }
    else{ gyro_x_off=gyro_y_off=gyro_z_off=0;}

    val = get_value(config, "mag_off");
    if(val){ sscanf(val, "%hd %hd %hd", &mag_x_off, &mag_y_off, &mag_z_off); }
    else{ mag_x_off=mag_y_off=mag_z_off=0;}

    filter_type = get_value(config, "filter");
  }

  /*** set default values ****/
  if(cdev == NULL){ cdev=(char *)DEFAULT_PORT; } 
  if(shmid < 0){ shmid=SHM_ID; }

  /*****************/

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

  _shmem->acc_off[0]=acc_x_off;
  _shmem->acc_off[1]=acc_y_off;
  _shmem->acc_off[2]=acc_z_off;

  _shmem->gyro_off[0]=gyro_x_off;
  _shmem->gyro_off[1]=gyro_y_off;
  _shmem->gyro_off[2]=gyro_z_off;

  _shmem->mag_off[0]=mag_x_off;
  _shmem->mag_off[1]=mag_y_off;
  _shmem->mag_off[2]=mag_z_off;

  _shmem->status=0;
  _shmem->cmd=0;

  next=0;

  mdfilter = new Madgwick(100, 0.6);
  mhfilter = new Mahony(100, 1.0, 0.0);
  cfilter = new imu_tools::ComplementaryFilter();


  /*
   * Start Daemon
   */
  if(daemon_flag == 1){
    if(daemon(0,0) == 0){ // run as deamon
      main_loop(cdev,  _shmem, filter_type);
    }else{
      printf("Error : faild to start imud\n");
      return -1;
    }
  }else{
    main_loop(cdev,  _shmem, filter_type);
  }
  return 0;
}


