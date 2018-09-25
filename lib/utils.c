/*
  Utilities...

*/
#include "utils.h"

void displayArray(double *data, int n){
   int m=5;
   printf("\n{\n");
   for(int i=0;i<n;i++){
     if((i % m) == 0 ) printf("\n");
     printf("%lf ", data[i]);
   }
   printf("\n}\n");
}


double *
shift_and_push(double val, double *data, int n){
   memmove(&data[1], &data[0], sizeof(double)*(n-1));
   data[0]=val;
   return data;
}

float
calc_global_acc(double ax, double ay, double az,
   double roll, double pitch, double yaw, float acc[3])
{
  double cph, cth, cps, sph, sth, sps;
  double acc_mag;

  cph=cos(yaw);
  sph=sin(yaw);
  cth=cos(pitch);
  sth=sin(pitch);
  cps=cos(roll);
  sps=sin(roll);

  acc[0] = cph*cth*ax + (cph*sth*sps-sph*cps)*ay + cph*sth*cps*az+sph*sps*az;
  acc[1] = sph*cth*ax + (sph*sth*sps+cph*cps)*ay + (sph*sth*cps-cph*sps)*az;
  acc[2] = -sth*ax + cth*sps*ay + cth*cps*az;

  acc_mag = sqrt(acc[0]*acc[0]+acc[1]*acc[1]+acc[2]*acc[2]);

  return  acc_mag;
}

