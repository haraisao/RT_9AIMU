/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */
#include "RT_9A_IMU.h"

extern int errno;

/*
   Map shared memory
*/
void *map_shared_mem(int id, int len, int create)
{
  int shmid;
  void *stat;

  if(create){
    shmid=shmget(id, len, IPC_CREAT|0666);
  }else{
    shmid=shmget(id, len, 0666);
  }
  if(shmid < 0){
    fprintf(stderr, "Error shared memory, id=%d, errno=%d\n", id, errno);
    return NULL;
  }

  stat=shmat(shmid, NULL, 0666);
  if(stat < 0){
    fprintf(stderr, "Error: fail to map shared memory, id=%d, shmid=%d\n", id,shmid);
  }else{
    fprintf(stderr, "Sucess to map shared memory, id=%d, shmid=%d\n", id, shmid);
  }
  return stat;
}

/***
   Open logfile
*/
FILE *open_logfile(const char *dirname, const char *name){
  FILE *fd;
  struct timeval tv;
  char fname[256];

  gettimeofday(&tv, NULL);
  if (dirname == NULL){
    sprintf(fname, "%s-%ld.txt", name, tv.tv_sec);
  }else{
    sprintf(fname, "%s/%s-%ld.txt", dirname, name, tv.tv_sec);
  }
  
  fd = fopen(fname, "w"); 

  if(fd == NULL){
    fprintf(stderr, "Fail to open file.\n");
  }

  return  fd;
}

/*
  Close logfile
*/
void close_logfile(FILE *fd){
  if(fd){
    fclose(fd);
    fd=NULL;
  }
  return;
}

/*
  Save data 
*/
void save_data(FILE *fd, imu_data *data, struct timeval *tv){
 if (fd){
   double tm = data->tv_sec - tv->tv_sec + data->tv_usec/1000000.0;
   fprintf(fd, "%lf %d %d %d %d %d %d %d %d %d %d %d %d\n",
	tm,
	data->timestamp,
 	data->acc[0], data->acc[1], data->acc[2], 
 	data->gyro[0], data->gyro[1], data->gyro[2], 
 	data->mag[0], data->mag[1], data->mag[2], data->tv_sec, data->tv_usec );
 }
 return;
}

