#!/usr/bin/python

#
#

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import graph

class BoxViewer(object):
  def __init__(self, imu=None):
    self.initBox(0.5, 1, 0.25)
    self.angleX = 0.0
    self.angleY = 0.0
    self.angleZ = 0.0
    self.camX = -3.0
    self.camY = 3.0
    self.camZ = 5.0
    self.light0p = [ 0.0, 3.0, 5.0, 1.0 ]
    self.light1p = [ 5.0, 3.0, 0.0, 1.0 ]
    self.redColor = [ 1.0, 0.0, 0.0, 1.0 ]
    self.greenColor = [ 0.0, 1.0, 0.0, 1.0 ]
    self.imu = imu
    self.event_loop=False

    self.initView(320, 320)

#    self.graph=graph.DataPlot()
#    self.graph.show()


  def initBox(self, x=1, y=2, z=0.5):
    self.vertex = [
        [  -x, -y, -z ],
        [   x, -y, -z ],
        [   x,  y, -z ],
        [  -x,  y, -z ],
        [  -x, -y,  z ],
        [   x, -y,  z ],
        [   x,  y,  z ],
        [  -x,  y,  z ]]

    self.face = [
        [ 0, 1, 2, 3 ],
        [ 1, 5, 6, 2 ],
        [ 5, 4, 7, 6 ],
        [ 4, 0, 3, 7 ],
        [ 4, 5, 1, 0 ],
        [ 3, 2, 6, 7 ]]

    self.normal = [
        [ 0.0, 0.0,-1.0 ],
        [ 1.0, 0.0, 0.0 ],
        [ 0.0, 0.0, 1.0 ],
        [-1.0, 0.0, 0.0 ],
        [ 0.0,-1.0, 0.0 ],
        [ 0.0, 1.0, 0.0 ]]

  def setAngles(self, x, y, z, update=True):
    self.angleX = x
    self.angleY = y
    self.angleZ = z
    if update: self.update()

  def initView(self, w=320, h=320):
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
    glutInitWindowSize(w, h)
    glutCreateWindow("Box Viewer")

    glutDisplayFunc(self.draw)
    glutReshapeFunc(self.resize)
    glutKeyboardFunc(self.keyboard)
    #glutTimerFunc(100,timer, 0)

    #glClearColor(0.0, 0.0, 1.0, 0.0)
    glClearColor(0.3, 0.3, 0.3, 1.0)
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_CULL_FACE)
    glCullFace(GL_FRONT)

    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glEnable(GL_LIGHT1)
    glLightfv(GL_LIGHT1, GL_DIFFUSE, self.redColor)
    glLightfv(GL_LIGHT1, GL_SPECULAR, self.redColor)

    self.setAngles(0, 0, 0)

  def resize(self, w, h):
    glViewport(0, 0, w, h)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(30.0, w/h, 1.0, 100.0)
    glMatrixMode(GL_MODELVIEW)

  def line3D(self,x1, y1, z1, x2, y2, z2):
    glLineWidth(1.0)
    glBegin(GL_LINES)
    glVertex3f(x1,y1,z1)
    glVertex3f(x2,y2,z2)
    glEnd();

  def drawMeasure(self, measure, size):
    glDisable(GL_LIGHTING)
    glColor4f(0.5, 0.5, 0.5, 0.5)
    for x in range(measure):
      self.line3D(x*size-(size*measure/2),0,-(size*measure/2),
        x*size-(size*measure/2),0,measure*size-(size*measure/2))

    for y in range(measure):
      self.line3D(-(size*measure/2),0,y*size-(size*measure/2),
        measure*size-(size*measure/2),0,y*size-(size*measure/2))

    glDisable(GL_DEPTH_TEST)

    glColor4f(1.0, 0.0, 0.0, 1.0)
    self.line3D(0,0,0,(measure/2+2)*size,0,0)

    glColor4f(0.0, 1.0, 0.0, 1.0)
    self.line3D(0,0,0,0,(measure/2+2)*size,0)

    glColor4f(0.0, 0.0, 1.0, 1.0)
    self.line3D(0,0,0,0,0,(measure/2+2)*size)

    glEnable(GL_LIGHTING)
    glEnable(GL_DEPTH_TEST)

  def drawObject(self):
    # Draw object
    glRotated(self.angleZ, 0.0, 0.0, 1.0)
    glRotated(self.angleY, 0.0, 1.0, 0.0)
    glRotated(self.angleX, 1.0, 0.0, 0.0)

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, self.greenColor)

    glBegin(GL_QUADS)
    for j in range(0, 6):
        glNormal3dv(self.normal[j])
        for i in range(0, 4):
            glVertex(self.vertex[self.face[j][i]])
    glEnd()

  def draw(self):
    #glClearColor(0.0, 0.0, 1.0, 0.0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glLoadIdentity()
    gluLookAt(self.camX, self.camY,self.camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)

    glColor3f(1,1,1)
    self.drawMeasure(10,0.5)

    glLightfv(GL_LIGHT0, GL_POSITION, self.light0p)
    glLightfv(GL_LIGHT1, GL_POSITION, self.light1p)

    #-----
    self.drawObject()

    glFlush()
    glutSwapBuffers()

  def keyboard(self, key, x, y):
    if key=='q':
        sys.exit()
    elif key=='x':
        self.event_loop=False
    elif key=='h':
        self.angleY += 1.0
        glutPostRedisplay()
    elif key=='j':
        self.angleX += 1.0
        glutPostRedisplay()
    elif key=='k':
        self.angleZ += 1.0
        glutPostRedisplay()

    elif key=='y':
        self.camX += 1.0
        glutPostRedisplay()
    elif key=='Y':
        self.camX -= 1.0
        glutPostRedisplay()
    elif key=='u':
        self.camY += 1.0
        glutPostRedisplay()
    elif key=='U':
        self.camY -= 1.0
        glutPostRedisplay()
    elif key=='i':
        self.camZ += 1.0
        glutPostRedisplay()
    elif key=='I':
        self.camZ -= 1.0
        glutPostRedisplay()

    glutMainLoopEvent()

  def moveBox(self, x, y, z):
    self.angleX += x 
    self.angleY += y
    self.angleZ += z

    self.update()

  def timer(self,value):
    glutPostRedisplay()
    glutTimerFunc(100,self.timer, 0)

  def update(self):
    glutPostRedisplay()
    glutMainLoopEvent()

  def start_loop(self):
    glutMainLoop()

  def set_angles(self, vals):
    self.setAngles(vals[0], vals[1], vals[2], True)

  def start(self, imu):
    self.event_loop=True
    self.imu=imu
    while self.event_loop and self.imu :
      angles = self.imu.get_angles()
      self.set_angles( angles )
#      if self.graph :
#        acc = self.imu.get_global_acc()
#        self.graph.set_global_acc( acc )
#        val = self.imu.get_acc_magnitude()
#        self.graph.setValue(3, (val-1)*100)

  def run(self):
    self.event_loop=True
    while self.event_loop and self.imu :
      angles = self.imu.get_angles()
      self.set_angles( angles )

