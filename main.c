/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#define EXTERN	1
#include "RT_9A_IMU.h"

int
main(int argc, char **argv)
{
  char *cdev;
  char buf[PACKET_SIZE*2];
  char *pack;
  int i;
  int next=0;

  struct imu_data_shm* _shmem;

  signal(SIGINT, sighandler);

  if(argc > 1){
    cdev=argv[1];
  }else{
    cdev=DEFAULT_PORT;
  } 

  _shmem = (struct imu_data_shm *)map_shared_mem(SHM_ID, sizeof(struct imu_data_shm), 1);
  if (_shmem == NULL){
   exit(-1);
  }
  _shmem->current=0;
  next=0;

  cfd = open_port(cdev);
  if (cfd < 0){
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
  }

  while(1){
    pack = read_packet(cfd, buf, PACKET_SIZE*2);
    if (pack != NULL){
      memcpy(&(_shmem->data[next]), pack,PACKET_SIZE);
      _shmem->current=next;
      next = (_shmem->current+1) % 10;
    }else{
      usleep(10);
    }
  }
  close(cfd);

}


