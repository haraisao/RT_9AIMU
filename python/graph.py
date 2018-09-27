#!/usr/bin/env python

import sys

from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
from numpy import *
import time
from scipy import signal

#
#
qtapp=None

#
color={ 'black':Qt.Qt.black, 'blue':Qt.Qt.blue, 'color0':Qt.Qt.color0,
   'color1':Qt.Qt.color1, 'cyan':Qt.Qt.cyan, 'darkBlue':Qt.Qt.darkBlue,
  'darkCya':Qt.Qt.darkCyan, 'darkGray':Qt.Qt.darkGray,
   'darkGreen':Qt.Qt.darkGreen, 'darkMagenta':Qt.Qt.darkMagenta,
   'darkRed':Qt.Qt.darkRed, 'darakYellow':Qt.Qt.darkYellow,
   'gray':Qt.Qt.gray, 'green':Qt.Qt.green, 'lightGray':Qt.Qt.lightGray,
    'magenta':Qt.Qt.magenta, 'red':Qt.Qt.red, 'transparent':Qt.Qt.transparent,
  'white':Qt.Qt.white, 'yellow':Qt.Qt.yellow }

class DataPlot(Qwt.QwtPlot):
  #
  def __init__(self, title="", size=(500,300), *args):
    global qtapp

    if qtapp is None:
      qtapp=Qt.QApplication([])

    Qwt.QwtPlot.__init__(self, *args)
    self.imu=None

    # Initialize data
    self.x = arange(0.0, 100.1, 0.5)
    self.curves=[]
    self.data_y=[]

    self.curve_list={}

    self.init_window()
    self.setTitle(title)

    self.idx=0
    self.resize(size[0], size[1])
    self.timer_id=0

    self.filters=[]

  #
  #
  def init_window(self):
    self.setCanvasBackground(Qt.Qt.white)
    #self.alignScales()

    self.insertLegend(Qwt.QwtLegend(), Qwt.QwtPlot.BottomLegend);

    #
    # Insert a horizontal maker
    mY = Qwt.QwtPlotMarker()
    mY.setLabelAlignment(Qt.Qt.AlignRight | Qt.Qt.AlignTop)
    mY.setLineStyle(Qwt.QwtPlotMarker.HLine)
    mY.setYValue(0.0)
    mY.attach(self)

    self.setAxisTitle(Qwt.QwtPlot.xBottom, "Time (seconds)")

    self.setAxisTitle(Qwt.QwtPlot.yLeft, "Values")
    self.setAxisScale(Qwt.QwtPlot.yLeft, -200, 200)

  #
  #  append a curve
  # Qt predefined colors:
  #   black, blue, color0, color1, cyan, darkBlue, darkCya, darkGray,
  #   darkGreen,darkMagenta, darkRed, darakYellow, gray, green, lightGray,
  #    magenta, red, transparent,white, yellow
  def mkCurve(self, name, color):
    data = zeros(len(self.x), float32)
    curve = Qwt.QwtPlotCurve(name)
    curve.attach(self)
    curve.setPen(Qt.QPen(color))
    self.curves.append(curve)
    self.data_y.append(data)
    self.curve_list[name]=(curve, data)
    return (curve, data)

  #
  #
  def alignScales(self):
    self.canvas().setFrameStyle(Qt.QFrame.Box | Qt.QFrame.Plain)
    self.canvas().setLineWidth(1)
    for i in range(Qwt.QwtPlot.axisCnt):
      scaleWidget = self.axisWidget(i)
      if scaleWidget:
        scaleWidget.setMargin(0)
      scaleDraw = self.axisScaleDraw(i)
      if scaleDraw:
        scaleDraw.enableComponent(
          Qwt.QwtAbstractScaleDraw.Backbone, False)


  #
  #
  def append_filter(self, n, v=0.01, typ='low'):
    if n > 0:
      self.filters.append(signal.butter(n, v, typ))
  #
  #
  def apply_filters(self, i):
    data_y=self.data_y[i]
    for ff in self.filters :
      data_y=signal.filtfilt(ff[0], ff[1], data_y)
    return data_y

  #
  def setValue(self, idx, val, filters=[]):
    self.data_y[idx]=concatenate((self.data_y[idx][1:],self.data_y[idx][:1]),0)
    self.data_y[idx][-1]=val

    data_y=self.data_y[idx]
    for ff in filters:
      data_y=signal.filtfilt(ff[0], ff[1], data_y)

    self.curves[idx].setData(self.x, data_y)

  #
  # callback 
  def timerEvent(self, e):
    self.update()
       
  def update(self):
    pass

  #
  # start/stop timer
  def start(self, intval=10):
    self.timer_id=self.startTimer(intval)

  def stop(self):
    if self.timer_id :
      self.killTimer(self.timer_id)
      self.timer_id=0

#
#
#
class PlotAccel(DataPlot):
  #
  def __init__(self, imu, size=(500,300), *args):
    DataPlot.__init__(self, title="Global Accel")
    self.imu = imu

    self.mkCurve("Roll", Qt.Qt.red)
    self.mkCurve("Pitch", Qt.Qt.green)
    self.mkCurve("Yaw", Qt.Qt.blue)
    self.mkCurve("Magnitude", Qt.Qt.magenta)

  #
  #
  def set_global_acc(self, val):
    for i in range(3):
      self.setValue(i, val[i]*100, self.filters)

    self.replot()

  #
  #
  def update(self):
    if self.imu :
      acc = self.imu.get_global_acc()
      self.set_global_acc( acc )
      val = self.imu.get_acc_magnitude()
      self.setValue(3, (val-1)*100)

#
# Plot Angles
#
class PlotAngles(DataPlot):
  def __init__(self, imu, size=(500,300), *args):
    DataPlot.__init__(self, title="Angles")
    self.imu = imu

    self.mkCurve("Roll", Qt.Qt.red)
    self.mkCurve("Pitch", Qt.Qt.green)
    self.mkCurve("Yaw", Qt.Qt.blue)

    #self.append_filter(1,0.002,'high')
    #self.append_filter(1,0.01,'low')

  #
  #  set angle (roll,pitch,yaw)
  #
  def set_angles(self, val):
    for i in range(3):
      self.data_y[i]=concatenate((self.data_y[i][1:], self.data_y[i][:1]), 0)
      if i == 2:
        self.data_y[i][-1] = val[i] -180
      else:
        self.data_y[i][-1] = val[i]

      data_y=self.apply_filters(i)

      self.curves[i].setData(self.x, data_y)

    self.replot()

  #
  #
  def update(self):
    if self.imu :
      angles = self.imu.get_angles()
      self.set_angles( angles )

