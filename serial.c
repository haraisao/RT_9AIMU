/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#include "shmem.h"

/**/
#define IMU_HEAD	0xffff52543941
#define DEFAULT_PORT 	"/dev/ttyACM0"
#define PACKET_SIZE	28

static int cfd=0;


void sighandler(int x)
{
  close(cfd);
  exit(0);
}

char mkSum(char *data, int len)
{
  int i;
  char res;

  if (len<1) return (char)0;
  res = data[0];
  for(i=0;i<len;i++){
    res=res ^ data[i];
  }
  return res;
}

int
main(int argc, char **argv)
{
  struct termios tio;
  int c;
  char buf[PACKET_SIZE];
  char *cdev;
  int i;
  imu_data *data_shmem;

  signal(SIGINT, sighandler);

  if(argc > 1){
    cdev=argv[1];
  }else{
    cdev=DEFAULT_PORT;
  } 

  memset(&tio,0,sizeof(tio));
  tio.c_cflag=CS8|CREAD|CLOCAL;	
  tio.c_cc[VMIN]=1;

  data_shmem = (imu_data *)map_shared_mem(SHM_ID, sizeof(imu_data), 1);

  cfd=open(cdev, O_RDWR|O_NONBLOCK);
  if (cfd < 0){
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
  }

  cfsetospeed(&tio, B57600);
  cfsetispeed(&tio, B57600);
  tcsetattr(cfd, TCSANOW, &tio);

  while(1){
    memset(buf, 0, sizeof(buf));
    c = read(cfd, buf, PACKET_SIZE);
    if (c>0){
      memcpy(data_shmem,buf,c);
    }else{
      usleep(10);
    }
  }
  close(cfd);

}


