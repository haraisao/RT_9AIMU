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
  unsigned short current;
  unsigned short pid;
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
import time
import numpy as np

import gl

# struct imu_data ==> 28+4+4=36
# struct imu_data_shm ==> 4+ 2*12 + 36*100 --> 3628
#  

class ImuShm(object):
  def __init__(self, gui=False, id=130):
    self.shm = sysv_ipc.SharedMemory(id, 0, mode=0666)
    self.shm_offset={'current':0, 'pid': 2,
            'acc_off': 4, 'gyro_off':10, 'mag_off':16, 'status': 22, 'cmd':23,
            'roll':24, 'pitch':28, 'yew':32, 'pos':36,
            'velocity':48, 'imu_data' : 60}
    self.imu_data_len=36
    self.imu_offset={'version':6, 'timestamp':7,'acc':8, 'templature':14,
            'gyro':16, 'mag':22, 'tv_sec':28, 'tv_usec':32}

    self.max_pool=100
    if gui : self.gui=gl.BoxViewer()
    else:  self.gui=None

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

  def read_float(self, off):
      return struct.unpack('f',self.shm.read(4, off))[0]

  def read_double(self, off):
      return struct.unpack('d',self.shm.read(8, off))[0]

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

  def get_mag_off(self):
      off = self.shm_offset['mag_off']
      return(self.read_short(off), self.read_short(off+2), self.read_short(off+4))

  def set_acc_off(self, vals=[0,0,-2048]):
      off = self.shm_offset['acc_off']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return

  def set_gyro_off(self, vals=[0,0,-2048]):
      off = self.shm_offset['gyro_off']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return

  def set_mag_off(self, vals=[0,0,0]):
      off = self.shm_offset['mag_off']
      for i in range(3):
          self.shm.write(struct.pack('h', vals[i]),off+i*2)
      return

  def get_angles(self):
      off = self.shm_offset['roll']
      return (self.read_float(off), self.read_float(off+4), self.read_float(off+8))

  def get_status(self):
      off = self.shm_offset['status']
      return self.read_uchar(off)

  def get_filter(self):
      v0=self.get_status()
      return ((v0 & 0x0e) >> 1)

  def set_filter(self, v):
      v0=self.get_status()
      off = self.shm_offset['status']
      v = ((v0 & 0x01) | (v << 1))
      self.shm.write(struct.pack('B', v), off)
      return 


#  def set_velocity(self, vals=[0,0,0]):
#      off = self.shm_offset['sp_x']
#      for i in range(3):
#          self.shm.write(struct.pack('h', vals[i]),off+i*2)
#      return
#
#  def set_angle(self, vals=[0,0,0]):
#      off = self.shm_offset['angle_x']
#      for i in range(3):
#          self.shm.write(struct.pack('h', vals[i]),off+i*2)
#      return


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
 
  def get_time(self, n):
      off = self.get_imu_data_offset(n) + self.imu_offset['tv_sec']
      return(self.read_int(off), self.read_int(off+4))
 
  def get_all_imu_data(self):
      offset = self.shm_offset['imu_data']
      data=self.shm.read(self.imu_data_len * self.max_pool, offset)
      return data

  def get_last_imu_data(self, n, off=0):
      data=bytes()
      offset = self.shm_offset['imu_data']
      cn = (self.get_current() + self.max_pool - off) % self.max_pool
      for i in range(n):
        cn = (cn + self.max_pool-1) % self.max_pool
        data=data+self.shm.read(self.imu_data_len, offset + cn* self.imu_data_len)
      return data

  def get_value_from_data(self, n, data, name, typ='h'):
      imu_data_off=self.imu_data_len + (self.imu_data_len % 4)
      off = n * imu_data_off+ self.imu_offset[name]
      if typ == 'B' :
        res=struct.unpack(typ, data[off:off+1])[0]
      elif typ == 'i' or typ == 'I':
        res=struct.unpack(typ, data[off:off+4])[0]
      else:
        res=struct.unpack(typ, data[off:off+2])[0]
      return res

  def get_acc_from_data(self, n, data):
      imu_data_off=self.imu_data_len + (self.imu_data_len % 4)
      off = n * imu_data_off+ self.imu_offset['acc']
      res=[ struct.unpack('h', data[off:off+2])[0],
            struct.unpack('h', data[off+2:off+4])[0],
            struct.unpack('h', data[off+4:off+6])[0]]
      return res

  def get_gyro_from_data(self, n, data):
      imu_data_off=self.imu_data_len + (self.imu_data_len % 4)
      off = n * imu_data_off+ self.imu_offset['gyro']
      res=[ struct.unpack('h', data[off:off+2])[0],
            struct.unpack('h', data[off+2:off+4])[0],
            struct.unpack('h', data[off+4:off+6])[0]]
      return res

  def get_mag_from_data(self, n, data):
      imu_data_off=self.imu_data_len + (self.imu_data_len % 4)
      off = n * imu_data_off+ self.imu_offset['mag']
      res=[ struct.unpack('h', data[off:off+2])[0],
            struct.unpack('h', data[off+2:off+4])[0],
            struct.unpack('h', data[off+4:off+6])[0]]
      return res

  def get_mag_calibration_data(self, m):
      arr=[]
      for x in range(m):
        time.sleep(1.5)
        data=self.get_all_imu_data()
        for n in range(self.max_pool):
          arr.append(self.get_mag_from_data(n, data))

      max_ar=np.array(arr).max(axis=0)
      min_ar=np.array(arr).min(axis=0)
      return max_ar - (max_ar - min_ar)/2

  def mag_calibration(self, m):
      return self.get_mag_calibration_data(m)

  def get_acc_average(self):
      data = self.get_all_imu_data()
      res=np.array([0,0,0], dtype='float')
      for x in range(self.max_pool):
          res += np.array(self.get_acc_from_data(x,data), dtype='float')
      res /= float(self.max_pool)
      return res

  def get_gyro_average(self):
      data = self.get_all_imu_data()
      res=np.array([0,0,0], dtype='float')
      for x in range(self.max_pool):
          res += np.array(self.get_gyro_from_data(x,data), dtype='float')
      res /= float(self.max_pool)
      return res

  def get_gyro_calibration_data(self, n):
      res=np.array([0,0,0], dtype='float')
      for x in range(n):
        res += self.get_gyro_average()
        time.sleep(1)

      res /= float(n)
      return res

  def get_acc_calibration_data(self, n):
      res=np.array([0,0,0], dtype='float')
      for x in range(n):
        res += self.get_acc_average()
        time.sleep(1)

      res /= float(n)
      return res

  def do_calibration(self, n):
      self.set_acc_off([0,0,0])
      self.set_gyro_off([0,0,0])
      time.sleep(1)

      gyro=np.array([0,0,0], dtype='float')
      acc=np.array([0,0,0], dtype='float')
      for x in range(n):
        acc += self.get_acc_average()
        gyro += self.get_gyro_average()
        time.sleep(1)

      gyro /= float(n)
      acc /= float(n)

      acc[2] += 2048
      self.set_acc_off(acc)
      self.set_gyro_off(gyro)

      return

  def create_gui(self):
    if not self.gui :
      self.gui=gl.BoxViewer()

  def start(self):
    if self.gui :
      self.gui.start(self)
