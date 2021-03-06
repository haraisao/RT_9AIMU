/*
   Kalman filter for posture estimation by using RT-USB-9AXIS-00

*/
#include "KalmanFilter.h"


float conv_normal(float v){
  if(v > M_PI){
    return v - M_PI*2;
  }else if (v < -M_PI){
    return v + M_PI*2;
  }else{
    return v;
  }
}

void
KalmanFilter::update(double acc[3], double gyro[3], double mag[3], double Ts)
{
  apply_kalman_filter(acc, gyro, mag, this->Pitch_Roll,
              &this->Est_Yaw,this->CovMat_P,&this->CovValue_py,Ts,this->flag);

  this->roll  = conv_normal(M_PI-Pitch_Roll[1]);
  this->pitch = -correct_pitch(Pitch_Roll[0], acc);
  this->yaw   = Est_Yaw;

  return;
}

/*
  Compute roll and pitch angles from the acceleration sensor
  Input: acc (m/s^2)
  Output: y (rad)
*/
void get_angle_from_acc(double acc[3], double y[2])
{
  y[0] = atan2( -acc[0], sqrt(acc[1]*acc[1]+acc[2]*acc[2])); // pitch
  y[1] = atan2( acc[1], acc[2]); //roll
  return;
}

/*
  Compute a new covariance matrix from Kalman Gain.
*/
void get_valiance(double gain[4], double P[4], double newP[4])
{
  newP[0] = (1-gain[0]) * P[0] - gain[1]*P[2] ;
  newP[1] = (1-gain[0]) * P[1]  -gain[1]*P[3];

  newP[2] = -gain[2] * P[0] + (1 - gain[3]) * P[2] ;
  newP[3] = -gain[2] * P[1] + (1 - gain[3]) * P[3];

  return;
}

/*
   Compute Kalman Gain.
*/
void get_kalman_gain(double P[4], double r[4], double gain[4])
{
  double v=(P[0]+r[0])*(P[3]+r[3])-(P[1]+r[1])*(P[2]+r[2]);

  gain[0] = ( P[0]*(P[3]+r[3])-P[1]*(P[1]+r[1]))/v;
  gain[1] = (-P[0]*(P[2]+r[2])+P[1]*(P[0]+r[0]))/v;
  gain[2] = ( P[2]*(P[3]+r[3])-P[3]*(P[1]+r[1]))/v;
  gain[3] = (-P[2]*(P[2]+r[2])+P[3]*(P[0]+r[0]))/v;
 
  return;
}

/*
 Correct pitch angle
*/
double correct_pitch(double pitch, double acc[3])
{
  if(acc[2] > 0){
   if (pitch > 0) return (M_PI - pitch);
   else return -M_PI-pitch;
  }
  return pitch;
}



/*
  Apply Kalman filter.
*/
void apply_kalman_filter(
    double a[3], double g[3], double m[3],
      double x[2], double *yaw, double P[4], double *p, double Ts, int flag)
{
   double x_[2], P_[4], gain[4];
   double s_p_wy, c_p_wy, s_p_wz, c_p_wz, cos_th,tan_th;

   double qy,ry;
   double q[4],r[4];
   double p_, g_, yaw_;
   int i;

   q[0]=q[3]=0.00175 * Ts * Ts;
   q[1]=q[2]=0;
   qy=0.0175 * Ts * Ts;

   r[0]=r[3]=Ts * Ts;
   r[1]=r[2]=0;
   ry=0.5*Ts * Ts;

   a[2] = -a[2];

   // x=[theta, psi]
   s_p_wy = sin(x[1])*g[1];
   c_p_wy = cos(x[1])*g[1];
   s_p_wz = sin(x[1])*g[2];
   c_p_wz = cos(x[1])*g[2];
   tan_th = tan(x[0]);
   cos_th = cos(x[0]);

   //  estimate X_n+1
   x_[0]=x[0] + (s_p_wy-c_p_wz)*Ts;
   x_[1]=x[1] + (g[0] + (c_p_wy + s_p_wz)*tan_th)*Ts;

   yaw_ = *yaw + (s_p_wy + c_p_wz)/cos_th * Ts;

   //  calc pre variance
   double v1, v2, v3, v4;
   v1 = (s_p_wy + c_p_wz)*Ts;
   v2 = (c_p_wy - s_p_wz)*Ts;
   v3 = v1/cos_th/cos_th;
   v4 = 1+v2*tan_th;
  
   P_[0] = P[0] - (P[1]+P[2]+P[3]*v1)*v1 + q[0];
   P_[1] = P[0]*v3 + P[1]*v4 - (P[2]*v3+P[3]*v4)*v1 + q[1];
   P_[2] = (P[0] - P[1]*v1)*v3 + (P[2] -P[3]*v1)*v4 + q[2];
   P_[3] = (P[0]*v3 + P[1]*v4 + P[2]*v4)*v3 +P[3]*v4*v4 + q[3];

   p_ = *p + qy;

   // calc Kalman gain
   get_kalman_gain(P_, r, gain);
   g_ = p_ / (p_+ry);

   ///// get new estimatuins
   double y[2];
   get_angle_from_acc(a, y);
   y[0] -= x_[0];
   y[1] -= x_[1];

   //// set new X and P
   x[0] = x[0] + gain[0]*y[0]+gain[1]*y[1];
   x[1] = x[1] + gain[2]*y[0]+gain[3]*y[1];

   get_valiance(gain, P_, P);
   
  // calc yaw, 
  /// Yaw = atan2(m[0],m[1]);
  if (flag == 0){
    s_p_wy = sin(x[1])*g[1];
    c_p_wz = cos(x[1])*g[2];
    cos_th = cos(x[0]);

    //  estimate X_n+1
    *yaw = *yaw + (s_p_wy + c_p_wz)/cos_th * Ts;

    if (*yaw > 6.28318){
      *yaw = *yaw - 6.28318;
    }
    if (*yaw < 0){
      *yaw = *yaw + 6.28318;
    }
  }else{
    double yy;
    if (yaw_ > M_PI){
      yaw_ -= M_PI_2;
    }else if (yaw_ < -M_PI){
      yaw_ += M_PI_2;
    }

    yy = atan2(m[0], m[1]) - yaw_;
    *yaw = *yaw + g_ * yy; 
    p_ = (1 - g_) * p_;
  }

  return;
}


/*

*/

static double Pitch_Roll[2]={0.0,0.0};  // pitch, roll
static double CovMat_P[4]={0.0,0.0,0.0,0.0};  // covaiance matrix
static double Est_Yaw=0.0;
static double CovValue_py=0.0;

void
kalman_updateIMU(
  double acc[3], double gyro[3], double mag[3], double Ts,
       double *_roll, double *_pitch, double *_yaw)
{
  apply_kalman_filter(acc,gyro,mag,Pitch_Roll,
                     &Est_Yaw,CovMat_P,&CovValue_py,Ts,0);

  *_pitch = correct_pitch(Pitch_Roll[0], acc);
  *_roll  = Pitch_Roll[1];
  *_yaw   = Est_Yaw;

  return;
}
