#!/usr/bin/env python

import sys

from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
from numpy import *


app=None

class DataPlot(Qwt.QwtPlot):
  def __init__(self, *args):
    global app

    if app is None:
      app=Qt.QApplication([])

    Qwt.QwtPlot.__init__(self, *args)

    self.setCanvasBackground(Qt.Qt.white)
    self.alignScales()

    # Initialize data
    self.x = arange(0.0, 100.1, 0.5)
    self.curves=[]
    self.data_y=[]

    self.setTitle("Angle Graph")
    self.insertLegend(Qwt.QwtLegend(), Qwt.QwtPlot.BottomLegend);

    # Qt predefined colors:
    #   black, blue, color0, color1, cyan, darkBlue, darkCya, darkGray,
    #   darkGreen,darkMagenta, darkRed, darakYellow, gray, green, lightGray,
    #    magenta, red, transparent,white, yellow
    self.mkCurve("Roll", Qt.Qt.red)
    self.mkCurve("Pitch", Qt.Qt.green)
    self.mkCurve("Yaw", Qt.Qt.blue)
    self.mkCurve("Magnitude", Qt.Qt.magenta)
    #
    #

    mY = Qwt.QwtPlotMarker()
    mY.setLabelAlignment(Qt.Qt.AlignRight | Qt.Qt.AlignTop)
    mY.setLineStyle(Qwt.QwtPlotMarker.HLine)
    mY.setYValue(0.0)
    mY.attach(self)

    self.setAxisTitle(Qwt.QwtPlot.xBottom, "Time (seconds)")

    self.setAxisTitle(Qwt.QwtPlot.yLeft, "Values")
    self.setAxisScale(Qwt.QwtPlot.yLeft, -180, 180)

    #self.startTimer(50)
    self.idx=0
    self.resize(500, 300)

  #
  #  append a curve
  def mkCurve(self, name, color):
    data = zeros(len(self.x), float32)
    curve = Qwt.QwtPlotCurve(name)
    curve.attach(self)
    curve.setPen(Qt.QPen(color))
    self.curves.append(curve)
    self.data_y.append(data)
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
  #  set angle (roll,pitch,yaw)
  #
  def set_angles(self, val):
    for i in range(3):
      self.data_y[i] = concatenate((self.data_y[i][1:], self.data_y[i][:1]), 0)
      if i == 2:
        self.data_y[i][-1] = val[i] -180
      else:
        self.data_y[i][-1] = val[i]
      self.curves[i].setData(self.x, self.data_y[i])

    self.replot()

  def set_global_acc(self, val):
    for i in range(3):
      self.data_y[i] = concatenate((self.data_y[i][1:], self.data_y[i][:1]), 0)
      self.data_y[i][-1] = val[i] * 90
      self.curves[i].setData(self.x, self.data_y[i])

    self.replot()


  def setValue(self, idx, val):
    self.data_y[idx] = concatenate((self.data_y[idx][1:], self.data_y[idx][:1]), 0)
    self.data_y[idx][-1] = val
    self.curves[idx].setData(self.x, self.data_y[idx])

  #
  # callback 
  def timerEvent(self, e):
    self.update()
       
  def update(self):
    pass

# Admire
if __name__ == '__main__':
    demo = DataPlot()
    demo.resize(500, 300)
    demo.show()
    sys.exit(app.exec_())
