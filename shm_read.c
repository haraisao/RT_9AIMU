/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include "RT_9A_IMU.h"
#include <getopt.h>
#include <ncurses.h>

void print_data(int i, int current, struct imu_data_shm* shm)
{
  imu_data *data;

  data = &(shm->data[current]);
  mvprintw(1,10, "==== RT-9A IMU (%d)=====", i);

  mvprintw(3,10, "current : %4d (%3d)", current, data->timestamp);

  mvprintw(4,10, "ACC_Off : %4d %4d %4d",
	  shm->acc_off[0], shm->acc_off[1], shm->acc_off[2]);
  mvprintw(5,10, "GYRO_Off : %4d %4d %4d",
	  shm->gyro_off[0], shm->gyro_off[1], shm->gyro_off[2]);

  mvprintw(6,10, "Temp    : %2.3f     \n", TEMP_RAW2DEG(data->templature));
  mvprintw(7,10, "Mag     : %+3.2f, %+3.2f, %+3.2f        ",
	  MAG_RAW2UT(data->mag[0]),
	  MAG_RAW2UT(data->mag[1]),
	  MAG_RAW2UT(data->mag[2]) );

  mvprintw(8,10, "Velocity: %+3.2f, %+3.2f, %+3.2f       ",
	 ACC_RAW2MS(shm->sp_x)/10,
         ACC_RAW2MS(shm->sp_y)/10,
         ACC_RAW2MS(shm->sp_z/10));


  mvprintw(9,10, "Angle   : %+3.2f, %+3.2f, %+3.2f       ",
	 OMEGA_RAW2DEGS(shm->angle_x),
         OMEGA_RAW2DEGS(shm->angle_y),
         OMEGA_RAW2DEGS(shm->angle_z));

  mvprintw(9,50, "Angle   : %d, %d, %d               ",
	 shm->angle_x,
         shm->angle_y,
         shm->angle_z);


  mvprintw(10,10, "Acc     : %+4d, %+4d, %+4d             ",
	 data->acc[0], data->acc[1] , data->acc[2]);

  mvprintw(11,10, "Gyro    : %+4d, %+4d, %+4d           ",
	 data->gyro[0], data->gyro[1] , data->gyro[2]);

  refresh();
  return;
}


int main(int argc, char **argv)
{
  struct imu_data_shm* _shmem;
  int current;
  int prev;
  int n=1;
  int shmid=SHM_ID;
  char c;

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

  /*
   *
   */
  initscr();
  raw();
  //cbreak();
  timeout(0);
  noecho();

  for(int i=0; n < 0 || i<n;){
    current=_shmem->current;
    if (current == prev){
      usleep(1000);
    }else{
      print_data(i, current,  _shmem);
      c=getch();
      if (c == 'q') break;
      prev = current;
      i++;
    }
  }
  timeout(-1);
  mvprintw(13,10, "...End. Please push any key");
  refresh();
  getch();
  endwin();
}
