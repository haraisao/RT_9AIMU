/*
   Kalman Filter 

*/

#ifndef __KELMAN_FILTER_H__
#define __KELMAN_FILTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef sign
#define sign(a)	( ((a)<0) ? -1 : ((a)>0))
#endif

#ifndef M_PI_2
#define M_PI_2 M_PI*2
#endif


class KalmanFilter
{
private:
  float sampleFreq;
  float roll;
  float pitch;
  float yaw;

  double Pitch_Roll[2];
  double CovMat_P[4];
  double Est_Yaw;
  double CovValue_py;
  int flag;

  


public:
  KalmanFilter(float sampleFreq){
    this->sampleFreq=sampleFreq;
    this->roll=this->pitch=this->yaw=0.0;
    this->Pitch_Roll[0]=this->Pitch_Roll[1]=0.0;
    this->CovMat_P[0]=0.0;
    this->CovMat_P[1]=0.0;
    this->CovMat_P[2]=0.0;
    this->CovMat_P[3]=0.0;
    this->Est_Yaw=0.0;
    this->CovValue_py=0.0;
    this->flag=0;
  };

  //void update(float gx, float gy, float gz, float ax, float ay, float az, float Ts);
  void update(short acc[3], short gyro[3], short mag[3], double Ts);

  float getRoll() {
    return this->roll * 57.29578f;
  }
  float getPitch() {
    return this->pitch * 57.29578f;
  }
  float getYaw() {
    return this->yaw * 57.29578f;
  }
  float getRollRadians() {
    return this->roll;
  }
  float getPitchRadians() {
    return this->pitch;
  }
  float getYawRadians() {
    return this->yaw;
  }


};






#ifdef __cplusplus
extern "C" {
#endif

void get_angle_from_acc(double acc[3], double y[2]);
void get_valiance(double gain[4], double P[4], double newP[4]);

void get_kalman_gain(double P[4], double r[4], double gain[4]);

void apply_kalman_filter(short acc[3], short gyro[3], short mag[3],
   double x[2], double *yaw, double P[4], double *p, double Ts, int flag);

double correct_pitch(double pitch, short acc[3]);

void kalman_updateIMU(short acc[3], short gyro[3], short mag[3], double Ts,
       double *_roll, double *_pitch, double *_yaw);

#ifdef __cplusplus
}
#endif

#endif
