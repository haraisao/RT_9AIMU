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
#include <sys/time.h>

#include <errno.h>
#include <syslog.h>

/**/
#ifndef SHM_ID
#define SHM_ID	130
#endif

#ifndef IMU_HEAD
#define IMU_HEAD	0xffff52543941
#define DEFAULT_PORT 	"/dev/ttyACM0"
#define PACKET_SIZE	28
#define MAX_POOL	100

#define PID_FILE	"/var/run/imud.pid"
#endif

#define NEXT_N(n, mx)	(n+1) % mx
#define PREV_N(n, mx)	(n+mx-1) % mx

#define ACC_RAW2G(v)	v/2048.0
#define ACC_RAW2MS(v)	v/2048.0 * 9.8
#define OMEGA_RAW2DEGS(v)	v/16.4
#define MAG_RAW2UT(v)	v*0.3
#define TEMP_RAW2DEG(v)	v/340.0 + 35

typedef struct imu_data{
  unsigned char header[6];
  unsigned char version;
  unsigned char timestamp;
  short acc[3];
  short templature;
  short gyro[3];
  short mag[3];
  int tv_sec;
  int tv_usec;
} imu_data;


struct imu_data_shm{
  unsigned short current;
  unsigned short pid;
  short acc_off[3];
  short gyro_off[3];
/*
  short sp_x;
  short sp_y;
  short sp_z;

  short angle_x;
  short angle_y;
  short angle_z;
*/

  struct imu_data data[MAX_POOL];
};


#ifndef EXTERN
extern int cfd;
#endif

void *map_shared_mem(int id, int len, int create);

int open_port(char *dev);
char *read_packet(int fd, char *buf, int buf_len);

#endif
