#
# Copyright(C) 2018 Isao Hara
# All rights reserved.
#
#

'''
#define MAX_POOL 100
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
  unsigned int current;
  short sp_x;
  short sp_y;
  short sp_z;

  short angle_x;
  short angle_y;
  short angle_z;
  short acc_off[3];
  short gyro_off[3];

  struct imu_data data[MAX_POOL];
};
'''
#
import os
import signal
import sysv_ipc
import struct

# struct imu_data ==> 28+4+4=34 ---> 36
# struct imu_data_shm ==> 4+ 2*12 + 36*100 --> 3628
#  
def getshmem(id=130):
  shm = sysv_ipc.SharedMemory(id, 0, mode=0666)
  return shm

def get_count(mem):
    return struct.unpack('I',mem.read(4))

class ImuShm(object):
  def __init__(self,id):
    self.shm = sysv_ipc.SharedMemory(id, 0, mode=0666)
    self.shm_offset={'current':0, 'pid': 2, 'sp_x': 4, 'sp_y' :6, 'sp_z':8,
            'angle_x': 10, 'angle_y':12, 'angle_z':14, 'acc_off': 16,
            'gyro_off':22, 'imu_data' : 28}
    self.imu_data_len=36
    self.imu_offset={'version':6, 'timestamp':7,'acc':8, 'templature':14,
            'gyro':16, 'mag':22, 'tv_sec':28, 'tv_usec':32}

  def get_shm_bytes(self, name, size=2):
      return self.shm.read(size, self.shm_offset[name])

  def get_short(self, name):
      return struct.unpack('h',self.get_shm_bytes(name))[0]

  def get_ushort(self, name):
      return struct.unpack('H',self.get_shm_bytes(name))[0]

  def read_int(self, off):
      return struct.unpack('i',self.shm.read(4, off))[0]

  def read_short(self, off):
      return struct.unpack('h',self.shm.read(2, off))[0]

  def read_ushort(self, off):
      return struct.unpack('H',self.shm.read(2, off))[0]

  def read_uchar(self, off):
      return struct.unpack('B',self.shm.read(1, off))[0]

  def kill_imud(self):
      pid=struct.unpack('H', self.get_byts('pid'))[0]
      os.kill(pid,signal.SIGTERM)

  def get_imu_data(self, n):
      imu_data_off=self.imu_data_len + (self.imu_data_len % 4)
      offset = self.shm_offset['imu_data'] + n * imu_data_off
      return self.shm.read(self.imu_data_len, offset)

  def get_imu_data_offset(self, n):
      imu_data_off=self.imu_data_len + (self.imu_data_len % 4)
      return self.shm_offset['imu_data'] + n * imu_data_off

  def get_current(self):
      return self.get_ushort('current')

  def get_acc_off(self):
      off = self.shm_offset['acc_off']
      return(self.read_short(off), self.read_short(off+2), self.read_short(off+4))

  def get_gyro_off(self):
      off = self.shm_offset['gyro_off']
      return(self.read_short(off), self.read_short(off+2), self.read_short(off+4))

  def set_acc_off(self, vals=[0,0,-2048]):
      off = self.shm_offset['acc_off']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return

  def set_gyro_off(self, vals=[0,0,0]):
      off = self.shm_offset['gyro_off']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return

  def set_velocity(self, vals=[0,0,0]):
      off = self.shm_offset['sp_x']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return

  def set_angle(self, vals=[0,0,0]):
      off = self.shm_offset['angle_x']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return


  def get_current_imu_data(self):
      n= self.get_ushort('current')
      return self.get_imu_data(n)

  def get_acc(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['acc']
      return(self.read_short(off), self.read_short(off+2), self.read_short(off+4))

  def get_gyro(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['gyro']
      return(self.read_short(off), self.read_short(off+2), self.read_short(off+4))

  def get_mag(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['mag']
      return(self.read_short(off), self.read_short(off+2), self.read_short(off+4))
 
  def get_templature(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['templature']
      return self.read_short(off)

  def get_timestamp(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['imestamp']
      return self.read_uchar(off)

  def get_version(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['version']
      return self.read_uchar(off)

  def get_time(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['tv_sec']
      return(self.read_int(off), self.read_int(off+4))
 
      

      

