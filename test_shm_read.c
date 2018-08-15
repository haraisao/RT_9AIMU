/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include "RT_9A_IMU.h"

int main(int argc, char **argv)
{
  struct imu_data_shm* _shmem;
  int current;
  imu_data *data;

  _shmem = (struct imu_data_shm *)map_shared_mem(SHM_ID, sizeof(struct imu_data_shm), 0);
  current=_shmem->current;
  data = &(_shmem->data[current]);

  fprintf(stderr, "current: %d \n", current);
  fprintf(stderr, "Version: %d \n", data->version);
  fprintf(stderr, "TimeStamp: %d \n", data->timestamp);
  fprintf(stderr, "Temp: %d \n", data->templature);
  fprintf(stderr, "Acc: (%d, %d, %d) \n", data->acc[0], data->acc[1] , data->acc[2]);


}
