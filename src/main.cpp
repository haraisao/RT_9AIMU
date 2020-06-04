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

#include "KalmanFilter.h"
#include "MadgwickAHRS.h"
#include "MahonyAHRS.h"
//#include "complementary_filter.h"
#include "butter.h"

/*
  Filter
*/
KalmanFilter *kfilter;
Madgwick *mdfilter;
Mahony *mhfilter;
//imu_tools::ComplementaryFilter *cfilter;
ButterFilter *bhfilter, *blfilter;


/*
 * Serial port
 */
static int cfd=0;

static int calib_idx=0;
static double acc_x[CALIB_DATA_LEN];
static double acc_y[CALIB_DATA_LEN];
static double acc_z[CALIB_DATA_LEN];
static double gyro_x[CALIB_DATA_LEN];
static double gyro_y[CALIB_DATA_LEN];
static double gyro_z[CALIB_DATA_LEN];
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
void apply_filter(struct imu_data_shm *shm, struct imu_data *data)
{
  double Ts = 0.01;
  int status=GET_FILTER_TYPE(shm->status);

  if( status == F_KALMAN){
    /// Convert physical values
    double gyro[3];
    double acc[3];
    double mag[3];

    /* for IMU 9A */
    for (int i=0; i<3; i++){
      gyro[i]=OMEGA_RAW2RAD(data->gyro[i]);
      acc[i]=ACC_RAW2G(data->acc[i]);
      mag[i]=MAG_RAW2UT(data->mag[i]);
    }
    kfilter->update(acc, gyro, mag, Ts);

    shm->yaw  = kfilter->getYaw();
    shm->pitch= kfilter->getPitch();
    shm->roll = kfilter->getRoll();

  }else if( status == F_MADGWICK) {
    mdfilter->updateIMU( OMEGA_RAW2DEGS(data->gyro[0]),
                         OMEGA_RAW2DEGS(data->gyro[1]),
                         OMEGA_RAW2DEGS(data->gyro[2]),
                         ACC_RAW2G(data->acc[0]),
                         ACC_RAW2G(data->acc[1]),
                         ACC_RAW2G(data->acc[2]));

    shm->yaw=mdfilter->getYaw();
    shm->pitch=mdfilter->getPitch();
    shm->roll=mdfilter->getRoll();

  }else if( status == F_MAHONY) {
    mhfilter->updateIMU( OMEGA_RAW2DEGS(data->gyro[0]),
                         OMEGA_RAW2DEGS(data->gyro[1]),
                         OMEGA_RAW2DEGS(data->gyro[2]),
                         ACC_RAW2G(data->acc[0]),
                         ACC_RAW2G(data->acc[1]),
                         ACC_RAW2G(data->acc[2]));

    shm->yaw=mhfilter->getYaw();
    shm->pitch=mhfilter->getPitch();
    shm->roll=mhfilter->getRoll();

  }else if( status ==  F_COMPLEMENTARY){
/*
    double mx = MAG_RAW2UT(data->mag[0]);
    double my = MAG_RAW2UT(data->mag[1]);
    double mz = MAG_RAW2UT(data->mag[2]);
    double roll, pitch, yaw;
    cfilter->update(OMEGA_RAW2DEGS(data->gyro[0]),
                    OMEGA_RAW2DEGS(data->gyro[1]),
                    OMEGA_RAW2DEGS(data->gyro[2]),
                    ACC_RAW2G(data->acc[0]),
                    ACC_RAW2G(data->acc[1]),
                    ACC_RAW2G(data->acc[2]), Ts );

    cfilter->computeAngles(&roll, &pitch, &yaw);
    shm->roll = RAD2DEG(roll);
    shm->pitch= RAD2DEG(pitch);
    shm->yaw  = RAD2DEG(yaw);

*/
  }else {

  }

  return;
}

/*
*/
void apply_command(struct imu_data_shm *shm, struct imu_data *data)
{
  /*** calibration **/

  if(shm->cmd == 1){
    double acc_x_off=0;
    double acc_y_off=0;
    double acc_z_off=0;
    double gyro_x_off=0;
    double gyro_y_off=0;
    double gyro_z_off=0;

    for(int i=0; i<CALIB_DATA_LEN;i++){
      acc_x_off += acc_x[i];
      acc_y_off += acc_y[i];
      acc_z_off += acc_z[i];
      gyro_x_off += gyro_x[i];
      gyro_y_off += gyro_y[i];
      gyro_z_off += gyro_z[i];
    }
      shm->acc_off[0] = acc_x_off /CALIB_DATA_LEN;
      shm->acc_off[1] = acc_y_off /CALIB_DATA_LEN;
      shm->acc_off[2] = (acc_z_off /CALIB_DATA_LEN) + 2048;
      shm->gyro_off[0] = gyro_x_off /CALIB_DATA_LEN;
      shm->gyro_off[1] = gyro_y_off /CALIB_DATA_LEN;
      shm->gyro_off[2] = gyro_z_off /CALIB_DATA_LEN;
    shm->cmd=0;
  } 
  return;
}

/*
   Main Loop
  
*/
void main_loop(char *cdev, struct imu_data_shm* shm)
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
    SET_STATUS( shm->status, 0);
  }else{
    SET_STATUS( shm->status, 1);
  }
  // Save PID
  pid=save_pid();
  shm->pid = pid;

  // Main loop
  while(1){
    if(cfd < 0){
        cfd = open_port(cdev);
        if (cfd < 0){
          SET_STATUS(shm->status, 0);
          sleep(1);
        }else{
          SET_STATUS(shm->status, 1);
        }
        continue;
    }
    
    pack = read_packet(cfd, buf, PACKET_SIZE*2);

    if (pack != NULL){
      data = &(shm->data[next]);
      gettimeofday(&tv,NULL);

      memcpy(data, pack,PACKET_SIZE);

      acc_x[calib_idx]=data->acc[0];
      acc_y[calib_idx]=data->acc[1];
      acc_z[calib_idx]=data->acc[2];
      gyro_x[calib_idx]=data->gyro[0];
      gyro_y[calib_idx]=data->gyro[1];
      gyro_z[calib_idx]=data->gyro[2];
      calib_idx = (calib_idx+1) % CALIB_DATA_LEN;

      /** subtract offsets **/
      for(i=0;i<3;i++){
        data->acc[i]  -= shm->acc_off[i];
        data->gyro[i] -= shm->gyro_off[i];
        data->mag[i] -= shm->mag_off[i];
      }

      /******/

      apply_filter(shm, data);
      apply_command(shm, data);

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
        SET_STATUS(shm->status, 0);
      }
    }
    //usleep(9000);
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

  float sample_freq=0;
  float madgwick_beta=0;
  float mahony_Kp=0;
  float mahony_Ki=0;

  int butter_h_dim=0;
  double butter_h_Wn=0;
  int butter_l_dim=0;
  double butter_l_Wn=0;

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

    val = get_value(config, "sample_freq");
    if(val){ sscanf(val, "%f", &sample_freq); }

    val = get_value(config, "madgwick_beta");
    if(val){ sscanf(val, "%f", &madgwick_beta); }

    val = get_value(config, "mahony_Kp");
    if(val){ sscanf(val, "%f", &mahony_Kp); }

    val = get_value(config, "mahony_Ki");
    if(val){ sscanf(val, "%f", &mahony_Ki); }

    val = get_value(config, "butter_h_dim");
    if(val){ sscanf(val, "%d", &butter_h_dim); }

    val = get_value(config, "butter_h_Wn");
    if(val){ sscanf(val, "%lf", &butter_h_Wn); }

    val = get_value(config, "butter_l_dim");
    if(val){ sscanf(val, "%d", &butter_l_dim); }

    val = get_value(config, "butter_l_Wn");
    if(val){ sscanf(val, "%lf", &butter_l_Wn); }

  }else{
  }

  /*** set default values ****/
  if(cdev == NULL){ cdev=(char *)DEFAULT_PORT; } 
  if(shmid < 0){ shmid=SHM_ID; }

  if(sample_freq == 0){ sample_freq=100; }
  if(madgwick_beta == 0){ madgwick_beta=0.6; }
  if(mahony_Kp == 0){ mahony_Kp=1.0; }
  if(mahony_Ki == 0){ mahony_Ki=0.0; }
  if(butter_h_dim == 0){ butter_h_dim=2; }
  if(butter_h_Wn == 0){ butter_h_Wn=0.01; }
  if(butter_l_dim == 0){ butter_l_dim=2; }
  if(butter_l_Wn == 0){ butter_l_Wn=0.01; }

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
//  _shmem = (struct imu_data_shm *)map_shared_mem(shmid,
//		 	 sizeof(struct imu_data_shm), create_flag);

  if (( _shmem = map_imu_shm(shmid)) == NULL){
   fprintf(stderr, "Error in map_shared_mem\n");
   exit(-1);
  }

  clear_all_shm(_shmem);
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

  
  if(!filter_type){
    SET_FILTER_TYPE(_shmem->status, F_NONE);
  }else if(!strcmp(filter_type, "Kalman")){
    SET_FILTER_TYPE(_shmem->status, F_KALMAN);
  }else if(!strcmp(filter_type, "Madgwick")){
    SET_FILTER_TYPE(_shmem->status, F_MADGWICK);
  }else if(!strcmp(filter_type, "Mahony")){
    SET_FILTER_TYPE(_shmem->status, F_MAHONY);
  }else if(!strcmp(filter_type, "Complementary")){
    SET_FILTER_TYPE(_shmem->status, F_COMPLEMENTARY);
  }else{
    SET_FILTER_TYPE(_shmem->status, F_NONE);
  }

  /*
    set global variables...
  **/

  next=0;

  kfilter = new KalmanFilter(sample_freq);
  mdfilter = new Madgwick(sample_freq, madgwick_beta);
  mhfilter = new Mahony(sample_freq, mahony_Kp, mahony_Ki);

//  cfilter = new imu_tools::ComplementaryFilter();

  bhfilter = new ButterFilter(butter_h_dim, butter_h_Wn, BUTTER_HIGH);
  blfilter = new ButterFilter(butter_l_dim, butter_l_Wn, BUTTER_LOW);

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


