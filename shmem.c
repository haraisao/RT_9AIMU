/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */
#include "RT_9A_IMU.h"

extern int errno;

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

