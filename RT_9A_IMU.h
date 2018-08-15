/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#ifndef __RT_9A_IMU_H__
#define __RT_9A_IMU_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>


/**/
#ifndef SHM_ID
#define SHM_ID	128
#endif

#ifndef IMU_HEAD
#define IMU_HEAD	0xffff52543941
#define DEFAULT_PORT 	"/dev/ttyACM0"
#define PACKET_SIZE	28
#endif

typedef struct imu_data{
  unsigned char header[6];
  unsigned char version;
  unsigned char timestamp;
  short acc[3];
  short templature;
  short gyro[3];
  short mag[3];
} imu_data;




#ifndef EXTERN
extern int cfd;
void sighandler(int x);

#else
static int cfd=0;

void sighandler(int x)
{
  close(cfd);
  exit(0);
}
#endif

void *map_shared_mem(int id, int len, int create);

int open_port(char *dev);
char *read_packet(int fd, char *buf, int buf_len);

#endif
