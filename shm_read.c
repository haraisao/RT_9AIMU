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

void print_data(int current, struct imu_data_shm* _shmem)
{
  imu_data *data;

  data = &(_shmem->data[current]);

  mvprintw(3,10, "current : %4d (%3d)", current, data->timestamp);

  mvprintw(4,10, "ACC_Off : %4d %4d %4d",
	  _shmem->acc_off[0], _shmem->acc_off[1], _shmem->acc_off[2]);
  mvprintw(5,10, "Temp    : %2.3f \n", data->templature/340.0 + 35);
  mvprintw(6,10, "Mag     : %3.2f, %3.2f, %3.2f",
	  data->mag[0]*0.3, data->mag[1]*0.3, data->mag[2]*0.3);

  mvprintw(7,10, "Velocity: %3.2f, %3.2f, %3.2f",
	 _shmem->sp_x/2048.0, _shmem->sp_y/2048.0, _shmem->sp_z/2048.0);


  mvprintw(8,10, "Angle   : %3.2f, %3.2f, %3.2f",
	 _shmem->angle_x/16.4, _shmem->angle_y/16.4, _shmem->angle_z/16.4);

  mvprintw(9,10, "Acc     : %4d, %4d, %4d",
	 data->acc[0], data->acc[1] , data->acc[2]);
  mvprintw(10,10, "Gyro    : %4d, %4d, %4d",
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
  noecho();

  for(int i=0; i<n;){
    current=_shmem->current;
    if (current == prev){
      usleep(1000);
    }else{
      print_data(current,  _shmem);
      prev = current;
      i++;
    }
  }
  mvprintw(13,10, "...End. Please push any key");
  refresh();
  getch();
  endwin();
}

/*
{
	int ch;
	keypad(stdscr,TRUE);
	noecho();
	printw(">");
	ch=getch();
	printw("%c", ch);
	refresh();
	getch();
	endwin();
}
*/
