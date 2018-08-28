/*
 *
 *  Copyright (C) 2018, Isao Hara
 *  All rights reserved.
 *
 *  License: The MIT Licens
 */

#ifdef TEST
#define EXTERN 1
#endif
#include "RT_9A_IMU.h"

/**/

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

int open_port(char *dev){
  struct termios tio;
  int fd;

  memset(&tio,0,sizeof(tio));
  tio.c_cflag=CS8|CREAD|CLOCAL;	
  tio.c_cc[VMIN]=1;
  cfsetospeed(&tio, B57600);
  cfsetispeed(&tio, B57600);

  //fd=open(dev, O_RDONLY|O_NONBLOCK);
  fd=open(dev, O_RDONLY);

  if (fd < 0){
    fprintf(stderr, "Fail to open %s\n", dev);
    return(-1);
  }
  tcsetattr(fd, TCSANOW, &tio);

  return fd;
}

int read_one_packet(int fd, char *buf, int len){
  int c,c1;
  c=read(fd, buf, len);
  if (c<0){
      fprintf(stderr, "read error(%d)!\n", c);
      return -1;
  }
  while (c < len){
    c1 = read(fd, buf+c, len-c);
    if (c1<0){
      fprintf(stderr, "read error!\n");
      return -1;
    }
    c += c1;
    fprintf(stderr, "read %d\n", c);
  }
  return c;
}


int check_packet(char *buf){
  int idx=0;
  unsigned char *p;
  do{
    p=buf+idx;
    if (p[0]==0xff && p[1]==0xff && p[2] == 0x52 && p[3]==0x54 && p[4]==0x39 && p[5]==0x41){
      return idx;
    }
    fprintf(stderr, " %x %x %x %x \n", p[0],p[1],p[2],p[3]);
    idx++;
  }while(idx<PACKET_SIZE);

  return -1;
}

char *read_packet(int fd, char *buf, int buf_len){
  int c;
  int idx;
  memset(buf, 0, buf_len);
  c=read_one_packet(fd, buf, PACKET_SIZE);
  if (c<0){
    return NULL;
  }
  idx = check_packet(buf);

  if(idx == 0){ return buf; }

  if(idx < 0){
    return NULL;
  }else {
    c=read_one_packet(fd, buf+PACKET_SIZE, idx);
    if (c<0){
      return NULL;
    }else{
      return buf+idx;
    }
  }

  return NULL;
}


int read_loop(int fd, void*shmem){
  char buf[PACKET_SIZE*2];
  char *pack;

  while(1){
    pack = read_packet(fd, buf, PACKET_SIZE*2);
    if (pack==NULL){
      return -1;
    }else{
      memcpy(shmem, pack, PACKET_SIZE);
    }
  }

  return 0;
}

#ifdef TEST
int
main(int argc, char **argv)
{
  char *cdev;
  char buf[PACKET_SIZE*2];
  char *pack;
  int i;


  signal(SIGINT, sighandler);

  if(argc > 1){
    cdev=argv[1];
  }else{
    cdev=DEFAULT_PORT;
  } 

  cfd = open_port(cdev);
  if (cfd < 0){
    fprintf(stderr, "Fail to open %s\n", cdev);
    exit(-1);
  }

  while(1){
    pack = read_packet(cfd, buf, PACKET_SIZE*2);
    if( pack != NULL){
     for(i=0; i<PACKET_SIZE; i++){
       fprintf(stderr, "%02x ", pack[i]);
     }
     fprintf(stderr, "\n");
    }
  }
  close(cfd);

}

#endif
