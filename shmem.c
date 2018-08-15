/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


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

  stat=shmat(shmid, NULL, 0666);
  if(stat < 0){
    fprintf(stderr, "Error: fail to map shared memory, id=%d\n", id);
  }else{
    fprintf(stderr, "Sucess to map shared memory, id=%d\n", id);
  }
  return stat;
}

