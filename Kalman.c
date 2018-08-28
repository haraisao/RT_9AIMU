/*



*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void get_angle_from_acc(double acc[3], double y[2]){
  y[0] = atan2( -acc[0], sqrt(acc[1]*acc[1]+acc[2]*acc[2]));
  y[1] = atan2( acc[1], acc[2]);
  return;
}

void get_valiance(double gain[4], double P[4], double newP[4]){
  newP[0] = (1-gain[0]) * P[0] - gain[1]*P[2] ;
  newP[1] = (1-gain[0]) * P[1]  -gain[1]*P[3];

  newP[2] = -gain[2] * P[0] + (1 - gain[3]) * P[2] ;
  newP[3] = -gain[2] * P[1] + (1 - gain[3]) * P[3];

  return;
}

void get_kalman_gain(double P[4], double r[4], double gain[4]){

  double v=(P[0]+r[0])*(P[3]+r[3])-(P[1]+r[1])*(P[2]+r[2]);

//  if(v < 0.0001){
//    gain[0] = gain[1] =gain[2]=gain[3]=0.0;
//  }else{
    gain[0] = ( P[0]*(P[3]+r[3])-P[1]*(P[1]+r[1]))/v;
    gain[1] = (-P[0]*(P[2]+r[2])+P[1]*(P[0]+r[0]))/v;
    gain[2] = ( P[2]*(P[3]+r[3])-P[3]*(P[1]+r[1]))/v;
    gain[3] = (-P[2]*(P[2]+r[2])+P[3]*(P[0]+r[0]))/v;
//  }
 
  return;
}

#if 0
void get_pre_estimation(double x[2], double gyro[3], double Ts, double x_[2]){
  x_[0]=x[0] + (sin_psi*gyro[1]-cos_psi*gyro[2])*Ts;
  x_[1]=x[1] + (gyro[0] + (cos_psi*gyro[1]+sin_psi*gyro[2])*tan_th)*Ts;

  return;
}

void get_pre_variance(double gyro[3], double P[4], double q[4], double Ts, double P_[4]){
  double v1, v2, v3, v4;
  v1 = (sin_psi*gyro[1] + cos_psi*gyro[2])*Ts;
  v2 = (cos_psi*gyro[1] - sin_psi*gyro[2])*Ts;
  v3 = v1/cos_th/cos_th;
  v4 = 1+v2*tan_th;
  
  P_[0] = P[0] - (P[1]+P[2]+P[3]*v1)*v1 + q[0];
  P_[1] = P[0]*v3 + P[1]*v4 - (P[2]*v3+P[3]*v4)*v1 + q[1];
  P_[2] = (P[0] - P[1]*v1)*v3 + (P[2] -P[3]*v1)*v4 + q[2];
  P_[3] = (P[0]*v3 + P[1]*v4 + P[2]*v4)*v3 +P[3]*v4*v4 + q[3];

  return;
}
#endif

//void apply_ext_kalman_filter(double x[2], double acc[3], double gyro[3],
//  double q[4], double r[4], double P[4], double Ts)
void apply_kalman_filter(double x[2], short acc[3], short gyro[3],
   double P[4], double Ts, double *yaw)
{
   double x_[2], P_[4], gain[4];
   double s_p_wy, c_p_wy, s_p_wz, c_p_wz, cos_th,tan_th;

   double q[4],r[4];

   //q[0]=q[3]=1.74E-3 * Ts * Ts;
   q[0]=q[3]=0.00174 * Ts * Ts;
   q[1]=q[2]=0;

   r[0]=r[3]= Ts * Ts;
   r[1]=r[2]=0;

   double g[3];
   double a[3];
   //double m[3];

   /* for IMU 9A */
   for (int i=0; i<3; i++){
     g[i]=gyro[i]*3.1415926525/2952.0;
     a[i]=acc[i]/2048.0;
     //m[i]=mag[i]*0.3;
   }


   // x=[theta, psi]
   s_p_wy = sin(x[1])*g[1];
   c_p_wy = cos(x[1])*g[1];
   s_p_wz = sin(x[1])*g[2];
   c_p_wz = cos(x[1])*g[2];
   tan_th = tan(x[0]);
   cos_th = cos(x[0]);
   //get_pre_estimation(x, gyro, Ts, x_);
   //  estimate X_n+1
   x_[0]=x[0] + (s_p_wy-c_p_wz)*Ts;
   x_[1]=x[1] + (g[0] + (c_p_wy + s_p_wz)*tan_th)*Ts;

   //get_pre_varance(gyro, P, q, Ts, P_);
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

   // calc Kalman gain
   get_kalman_gain(P_, r, gain);

   double y[2];
   get_angle_from_acc(a, y);
   y[0] -= x_[0];
   y[1] -= x_[1];

   double new_x[2], new_P[4];
   x[0] = x[0]+ gain[0]*y[0]+gain[1]*y[1];
   x[1] = x[1]+ gain[2]*y[0]+gain[3]*y[1];

   get_valiance(gain, P_, P);
   
  *yaw += (s_p_wy + c_p_wz)/cos_th * Ts;

   return;
}
