/*
   Kalman filter for posture estimation by using RT-USB-9AXIS-00


*/

#ifndef __KALMAN_H__
#define __KALMAN_H__
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef sign
#define sign(a)	( ((a)<0) ? -1 : ((a)>0))
#endif

#ifndef M_PI_2
#define M_PI_2 M_PI*2
#endif

#ifdef __cplusplus
extern "C" {
#endif

void get_angle_from_acc(double acc[3], double y[2]);
void get_valiance(double gain[4], double P[4], double newP[4]);

void get_kalman_gain(double P[4], double r[4], double gain[4]);

void apply_kalman_filter(short acc[3], short gyro[3], short mag[3],
   double x[2], double *yaw, double P[4], double *p, double Ts, int flag);

double correct_pitch(double pitch, short acc[3]);

#ifdef __cplusplus
}
#endif
#endif
