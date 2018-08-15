/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include <stdio.h>
#include "shmem.h"

int
main(int argc, char **argv)
{
  imu_data *data_mem; 

  data_mem = (imu_data *)map_shared_mem(SHM_ID, sizeof(imu_data), 0);

  fprintf(stderr, "Version: %d \n", data_mem->version);
  fprintf(stderr, "Temp: %d \n", data_mem->templature);
  fprintf(stderr, "Acc: (%d, %d, %d) \n", data_mem->acc[0], data_mem->acc[1] , data_mem->acc[2]);


}
