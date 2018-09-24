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
    self.roll = zeros(len(self.x), float32)
    self.pitch = zeros(len(self.x), float32)
    self.yaw = zeros(len(self.x), float32)

    self.setTitle("Angle Graph")
    self.insertLegend(Qwt.QwtLegend(), Qwt.QwtPlot.BottomLegend);

    #
    #
    #self.curveR = Qwt.QwtPlotCurve("Roll")
    #self.curveR.attach(self)

    #self.curveR.setPen(Qt.QPen(Qt.Qt.blue))
    self.cRoll = self.mkCurve("Roll", Qt.Qt.red)
    self.cPitch = self.mkCurve("Pitch", Qt.Qt.green)
    self.cYaw = self.mkCurve("Yaw", Qt.Qt.blue)
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

  def mkCurve(self, name, color):
    curve = Qwt.QwtPlotCurve(name)
    curve.attach(self)
    curve.setPen(Qt.QPen(color))
    return curve

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

  def set_angles(self, val):
    self.roll = concatenate((self.roll[1:], self.roll[:1]), 0)
    self.roll[-1] = val[0]
    self.cRoll.setData(self.x, self.roll)

    self.pitch = concatenate((self.pitch[1:], self.pitch[:1]), 0)
    self.pitch[-1] = val[1]
    self.cPitch.setData(self.x, self.pitch)

    self.yaw = concatenate((self.yaw[1:], self.yaw[:1]), 0)
    self.yaw[-1] = val[2] -180
    self.cYaw.setData(self.x, self.yaw)

    self.replot()

  def timerEvent(self, e):
    self.update()
       
  def update(self):
    pass

# Admire
if __name__ == '__main__':
#    __app = Qt.QApplication(sys.argv)
    demo = DataPlot()
    demo.resize(500, 300)
    demo.show()
    sys.exit(app.exec_())
