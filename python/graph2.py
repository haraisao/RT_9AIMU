#
#
import sys
import time


import numpy as np
from scipy import signal

#from PySide import QtCore,QtGui
from PyQt4 import QtCore,QtGui
import pyqtgraph as pg

from math import *
import threading


#
#  QApplication
qtapp=None
win=None

def pushData(data, val):
  data=np.concatenate((data[1:], data[:1]), 0)
  data[-1]=val
  return data

def parseName(name):
  names=name.split(':')
  if len(names)==1 or names[1] == '':
    cname=wname=names[0]
  else:
    wname=names[0]
    cname=names[1]
  return (wname, cname)


def main():
  global win,gtapp
#  qtapp=QtGui.QApplication([])
  win=GraphWin()
  win.show()
  #qtapp.exec_()

#
#
class GraphWin(QtGui.QWidget):
  def __init__(self, title="DataPlot", parent=None):
    global qtapp

    if qtapp is None:
      qtapp=QtGui.QApplication(sys.argv)
     
    QtGui.QWidget.__init__(self, parent)
    self.setWindowTitle(title)
    self.layout = QtGui.QVBoxLayout() 
    self.setLayout(self.layout)

    self.plotter={}

    self.resize(600,300)


  def mkPlot(self, nm):
    pw=DataPlotWidget(nm)
    self.layout.addWidget(pw)
    self.plotter[nm]=pw
    pw.addCurveItem(nm)

  def mkAccelPlot(self, nm, imu):
    pw=AccelPlotWidget(nm, imu)
    self.layout.addWidget(pw)
    self.plotter[nm]=pw

  def mkAnglePlot(self, nm, imu):
    pw=AnglePlotWidget(nm, imu)
    self.layout.addWidget(pw)
    self.plotter[nm]=pw

  def mkImuGraph(self, imu):
     self.mkAnglePlot('Angle', imu)
     self.mkAccelPlot('Acc', imu)
     self.show()

  def setValue(self, val, name):
    wname, cname = parseName(name)
    self.plotter[wname].setValue(cname, val)

  def refresh(self, wname):
    self.plotter[wname].refresh()

  def start(self, wname, n):
    self.plotter[wname].startTimer(n)

  def start_all(self, n):
    for i,k in enumerate(self.plotter):
      self.plotter[k].startTimer(n)

  def stop(self, wname):
    self.plotter[wname].stopTimer()

  def stop_all(self):
    for i,k in enumerate(self.plotter):
      self.plotter[k].stopTimer()

  def reset(self, wname):
    self.plotter[wname].reset_data()

#
#
#     
class DataPlotWidget(pg.PlotWidget):
  def __init__(self, name, *args, **kargs):
    pg.PlotWidget.__init__(self, *args, **kargs) 
    self.name=name
    self.curves={}

    self.x=np.arange(0.0, 100.1, 0.5)

    self.timer=QtCore.QTimer()
    self.timer.timeout.connect(self.refresh)

  def addCurveItem(self, nm, **kargs):
    itm=DataCurve(**kargs)
    self.plotItem.addItem(itm)
    self.curves[nm]=itm

  def startTimer(self, val):
    self.timer.start(val)

  def stopTimer(self):
    self.timer.stop()

  def refresh_data(self):
    for i,k in enumerate(self.curves):
      self.setValue(k,sin(time.time()))
    pass

  def refresh(self):
    self.clear()
    self.refresh_data()
    for i,k in enumerate(self.curves):
      self.curves[k].refresh()
      self.plotItem.addItem(self.curves[k])

  def setValue(self,name, val):
    if self.curves.has_key(name):
      self.curves[name].setValue(val)

  def setValues(self, val):
    for i,k in enumerate(self.curves):
      try:
        self.curves[k].setValue(val[i])
      except:
        pass

  def reset_data(self):
    for i,k in enumerate(self.curves):
      self.curves[k].reset_data()
#
# 
class DataCurve(pg.PlotCurveItem):
  def __init__(self, *args, **kargs):
    pg.PlotCurveItem.__init__(self,*args, **kargs) 

    if kargs.has_key('x') :
      self.data_x=kargs['x']
      self.data_len=len(self.data_x)
      self.data_y=np.zeros(len(self.data_x))
    else:
      self.data_x=None
      self.data_len=100
      self.data_y=[]

  def setValue(self,val):
    if len(self.data_y) < self.data_len :
      self.data_y.append(val)
    else:
      self.data_y=pushData(self.data_y, val)

  def refresh(self):
    self.setData(x=self.data_x, y=self.data_y)

  def reset_data(self):
    self.data_y=np.zeros(len(self.data_len))

#
#
#
class AccelPlotWidget(DataPlotWidget):
  def __init__(self, name, imu, *args):
    DataPlotWidget.__init__(self, name, *args, title="Accel") 
    self.addCurveItem('acc_x', pen=pg.mkPen('r'), name="acc_x")
    self.addCurveItem('acc_y', pen=pg.mkPen('g'), name="acc_y")
    self.addCurveItem('acc_z', pen=pg.mkPen('b'), name="acc_z")
    self.imu=imu
    self.plotItem.setRange(yRange=(-3,3))

  def set_imu(self, x):
    self.imu=x

  def refresh_data(self):
    if self.imu :
      acc = self.imu.get_global_acc()
      self.setValue('acc_x', acc[0])
      self.setValue('acc_y', acc[1])
      self.setValue('acc_z', acc[2])


#
#
#
class AnglePlotWidget(DataPlotWidget):
  def __init__(self, name, imu, *args):
    DataPlotWidget.__init__(self, name, *args, title="Angle") 
    self.addCurveItem('angle_x', pen=pg.mkPen('r'), name="roll")
    self.addCurveItem('angle_y', pen=pg.mkPen('g'), name="pitch")
    self.addCurveItem('angle_z', pen=pg.mkPen('b'), name="yaw")
    self.imu=imu
    self.plotItem.setRange(yRange=(-200,200))

  def set_imu(self, x):
    self.imu=x

  def refresh_data(self):
    if self.imu :
      ang = self.imu.get_angles()
      self.setValue('angle_x', ang[0])
      self.setValue('angle_y', ang[1])
      self.setValue('angle_z', ang[2])


