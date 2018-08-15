/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#ifndef __SHMEM_H__
#define __SHMEM_H__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifndef SHM_ID
#define SHM_ID	128
#endif

void *map_shared_mem(int id, int len, int create);

typedef struct imu_data{
  unsigned char header[6];
  unsigned char version;
  unsigned char timestamp;
  short acc[3];
  short templature;
  short gyro[3];
  short mag[3];
} imu_data;

#endif
