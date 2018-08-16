/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include "RT_9A_IMU.h"
#include <getopt.h>

int main(int argc, char **argv)
{
  struct imu_data_shm* _shmem;
  int current;
  imu_data *data;
  int prev;
  int n=1;
  int shmid=SHM_ID;

  /*
   *  Options....
   */
  struct option longopts[] = {
    { "num", required_argument, NULL, 'n'},
    { "shid", required_argument, NULL, 's'},
    {  0, 0, 0, 0},
  };

  int opt;
  int longindex;
  while((opt=getopt_long(argc, argv, "hn:s:", longopts, &longindex)) != -1){
    switch(opt){
      case 'h':
        printf("Usage: %s [-n num] [-s shmid]\n", argv[0]);
	exit(0);
	break;
      case 'n':
        n=atoi(optarg);
	break;
      case 's':
        shmid=atoi(optarg);
        break;
      default:
        printf("Error: Invalid option( \"%c\" )\n", opt);
        break;
    }
  }


  _shmem = (struct imu_data_shm *)map_shared_mem(shmid, sizeof(struct imu_data_shm), 0);

  for(int i=0; i<n;){
    current=_shmem->current;
    if (current == prev){
	    usleep(1000);
    }else{
      data = &(_shmem->data[current]);

      fprintf(stderr, "current: %d (%d)\n", current, data->timestamp);
      fprintf(stderr, "ACC_Off: %d %d %d \n", _shmem->acc_off[0], _shmem->acc_off[1], _shmem->acc_off[2]);
      fprintf(stderr, "Temp: %f \n", data->templature/340.0 + 35);

      fprintf(stderr, "V: (%.2f, %.2f, %.2f) \n", _shmem->sp_x/2048.0, _shmem->sp_y/2048.0, _shmem->sp_z/2048.0);

      fprintf(stderr, "Angle: (%.2f, %.2f, %.2f) \n", _shmem->angle_x/16.4, _shmem->angle_y/16.4, _shmem->angle_z/16.4);

      fprintf(stderr, "Acc: (%d, %d, %d) \n", data->acc[0], data->acc[1] , data->acc[2]);
      fprintf(stderr, "Gyro: (%d, %d, %d) \n", data->gyro[0], data->gyro[1] , data->gyro[2]);
      prev = current;
      i++;
    }
  }

}
