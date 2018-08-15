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

  imu_data *data_shmem;

  signal(SIGINT, sighandler);

  if(argc > 1){
    cdev=argv[1];
  }else{
    cdev=DEFAULT_PORT;
  } 

  data_shmem = (imu_data *)map_shared_mem(SHM_ID, sizeof(imu_data), 1);

  cfd = open_port(cdev);
  if (cfd < 0){
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
  }

  while(1){
    pack = read_packet(cfd, buf, PACKET_SIZE*2);
    if (pack != NULL){
      memcpy(data_shmem, pack,PACKET_SIZE);
    }else{
      usleep(10);
    }
  }
  close(cfd);

}


