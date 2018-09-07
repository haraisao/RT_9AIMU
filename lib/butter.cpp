/*
  Butterworth filter

  Copyright(C) 2018 Isao Hara.
  All right reserved.
*/
#include "butter.h"

/*
  Function
   compute  nCm
*/
float
combination(int n, int m){
  int i;
  float res;

  for(i=0, res=1; i<m; i++){
    res *= (float)(n-i)/(i+1);
  }
  return res;
}

/***
   Butterworth

*/
ButterFilter::ButterFilter(int n, double Wn, int type)
{
  this->order=n;
  this->Wn=Wn;

  xv = new double[ n+1 ];
  yv = new double[ n+1 ];
  
  ay = new double[ n+1 ];
  bx = new double[ n+1 ];

  for(int i; i <= order; i++){
    xv[i] = 0.0;
    yv[i] = 0.0;
    ay[i] = 0.0;
    bx[i] = 0.0;
  }
  
  if (type == 1){
    filter_type="High-pass";
    genHighPass(Wn);
  }else{
    filter_type="Low-pass";
    genLowPass(Wn);
  }
}

/*
   generate Low-pass filter coeffients
*/
void ButterFilter::genLowPass(double Wn)
{
  //Lowpass
  QcW = 1.0/tan( M_PI_DIV_2 * Wn);

  ay[0] = 1;

  if (order == 1){
    gain = 1 / (1 + QcW);

    ay[1] = (1 - QcW) * gain;

  }else if (order == 2){
    double QcW_2 = QcW*QcW;
    double c1=SQRT2;
    gain = 1 / (1 + c1 * QcW + QcW_2);

    ay[1] = (2          - 2*QcW_2) * gain;
    ay[2] = (1 - c1*QcW +   QcW_2) * gain;

  }else if (order == 3){
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;

    gain = 1 / (1 + 2 * QcW + 2* QcW_2 + QcW_3);

    ay[1] = (3 + 2*QcW - 2*QcW_2 - 3*QcW_3) * gain;
    ay[2] = (3 - 2*QcW - 2*QcW_2 + 3*QcW_3) * gain;
    ay[3] = (1 - 2*QcW + 2*QcW_2 -   QcW_3) * gain;

  }else if (order == 4){
    double c1,c2,c3;
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;
    double QcW_4 = QcW_3*QcW;
    double A=sqrt(2 + SQRT2);
    c1 = c3 = SQRT2*A;
    c2 = 2+SQRT2;

    gain = 1 / (1 + c1*QcW + c2*QcW_2 + c3*QcW_3 + QcW_4);

    ay[1] = (4 +2*c1*QcW             -2*c3*QcW_3 -4*QcW_4) * gain;
    ay[2] = (6           -2*c2*QcW_2             +6*QcW_4) * gain;
    ay[3] = (4 -2*c1*QcW             +2*c3*QcW_3 -4*QcW_4) * gain;
    ay[4] = (1 -  c1*QcW +  c2*QcW_2 -  c3*QcW_3 +  QcW_4) * gain;

  }else if (order == 5){
    double c1,c2,c3,c4;
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;
    double QcW_4 = QcW_3*QcW;
    double QcW_5 = QcW_4*QcW;

    c1 = c4 = 1+sqrt(5);
    c2 = c3 = c1+2; // 3+sqrt(5)

    gain = 1 / (1 + c1*QcW + c2*QcW_2 + c3*QcW_3 + c4*QcW_4 + QcW_5);

    ay[1] = (5 +3*c1*QcW +  c2*QcW_2 -  c3*QcW_3 -3*c4*QcW_4 - 5*QcW_5) * gain;
    ay[2] = (10+2*c1*QcW -2*c2*QcW_2 -2*c3*QcW_3 +2*c4*QcW_4 +10*QcW_5) * gain;
    ay[3] = (10-2*c1*QcW -2*c2*QcW_2 +2*c3*QcW_3 +2*c4*QcW_4 -10*QcW_5) * gain;
    ay[4] = (5 -3*c1*QcW +  c2*QcW_2 +  c3*QcW_3 -3*c4*QcW_4 + 5*QcW_5) * gain;
    ay[5] = (1 -  c1*QcW +  c2*QcW_2 -  c3*QcW_3 +  c4*QcW_4 -   QcW_5) * gain;

  }else if (order == 6){
    double c1,c2,c3,c4,c5;
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;
    double QcW_4 = QcW_3*QcW;
    double QcW_5 = QcW_4*QcW;
    double QcW_6 = QcW_5*QcW;

    c1 = SQRT2 + sqrt(6);
    c2 = 4+2*sqrt(3);
    c3 = 3*SQRT2 + 2*sqrt(6);
    c4 = 4+2*sqrt(3);
    c5 = SQRT2 + sqrt(6);

    gain = 1/(1+c1*QcW+c2*QcW_2+c3*QcW_3+c4*QcW_4+c5*QcW_5+QcW_6);
    ay[1] = (6+4*c1*QcW+2*c2*QcW_2-2*c4*QcW_4-4*c5*QcW_5-6*QcW_6)*gain;
    ay[2] = (15+5*c1*QcW-c2*QcW_2-3*c3*QcW_3-c4*QcW_4+5*c5*QcW_5+15*QcW_6)*gain;
    ay[3] = (20-4*c2*QcW_2+4*c4*QcW_4-20*QcW_6)*gain;
    ay[4] = (15-5*c1*QcW-c2*QcW_2+3*c3*QcW_3-c4*QcW_4-5*c5*QcW_5+15*QcW_6)*gain;
    ay[5] = (6-4*c1*QcW+2*c2*QcW_2-2*c4*QcW_4+4*c5*QcW_5-6*QcW_6)*gain;
    ay[6] = (1-c1*QcW+c2*QcW_2-c3*QcW_3+c4*QcW_4-c5*QcW_5+QcW_6)*gain;

  }

  for(int i=0; i <= order; i++){
    bx[i] = combination(order, i) * gain;
  }
}

/*
   generate High-pass filter coeffients
*/
void ButterFilter::genHighPass(double Wn)
{
  //Highpass
  QcW = tan( M_PI_DIV_2 * Wn);

  ay[0] = 1;

  if (order == 1){
    gain = 1 / (1 + QcW);

    ay[1] = (QcW -1) * gain;

  }else if (order == 2){
    double QcW_2 = QcW*QcW;
    double c1=SQRT2;
    gain = 1 / (1 + c1 * QcW + QcW_2);

    ay[1] = (-2          + 2*QcW_2) * gain;
    ay[2] = ( 1 - c1*QcW +   QcW_2) * gain;

  }else if (order == 3){
    double QcW_2 = QcW  *QcW;
    double QcW_3 = QcW_2*QcW;

    gain = 1 / (1 + 2* QcW + 2*QcW_2 + QcW_3);

    ay[1] = (-3 - 2*QcW + 2*QcW_2 + 3*QcW_3) * gain;
    ay[2] = ( 3 - 2*QcW - 2*QcW_2 + 3*QcW_3) * gain;
    ay[3] = (-1 + 2*QcW - 2*QcW_2 +   QcW_3) * gain;

  }else if (order == 4){
    double c1,c2,c3;
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;
    double QcW_4 = QcW_3*QcW;
    double A=sqrt(2 + SQRT2);
    c1 = c3 = SQRT2*A;
    c2 = 2+SQRT2;

    gain = 1 / (1 + c1*QcW + c2*QcW_2 + c3*QcW_3 + QcW_4);

    ay[1] = (-4 -2*c1*QcW             +2*c3*QcW_3 +4*QcW_4) * gain;
    ay[2] = ( 6           -2*c2*QcW_2             +6*QcW_4) * gain;
    ay[3] = (-4 +2*c1*QcW             -2*c3*QcW_3 +4*QcW_4) * gain;
    ay[4] = ( 1 -  c1*QcW +  c2*QcW_2 -  c3*QcW_3 +  QcW_4) * gain;

  }else if (order == 5){
    double c1,c2,c3,c4;
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;
    double QcW_4 = QcW_3*QcW;
    double QcW_5 = QcW_4*QcW;

    c1 = c4 = 1+sqrt(5);
    c2 = c3 = c1+2; // 3+sqrt(5)

    gain = 1 / (1 + c1*QcW + c2*QcW_2 + c3*QcW_3 + c4*QcW_4 + QcW_5);

    ay[1] = (-5 -3*c1*QcW -  c2*QcW_2 +  c3*QcW_3 +3*c4*QcW_4 + 5*QcW_5) * gain;
    ay[2] = ( 10+2*c1*QcW -2*c2*QcW_2 -2*c3*QcW_3 +2*c4*QcW_4 +10*QcW_5) * gain;
    ay[3] = (-10+2*c1*QcW +2*c2*QcW_2 -2*c3*QcW_3 -2*c4*QcW_4 +10*QcW_5) * gain;
    ay[4] = ( 5 -3*c1*QcW +  c2*QcW_2 +  c3*QcW_3 -3*c4*QcW_4 + 5*QcW_5) * gain;
    ay[5] = (-1 +  c1*QcW -  c2*QcW_2 +  c3*QcW_3 -  c4*QcW_4 +   QcW_5) * gain;

  }else if (order == 6){
    double c1,c2,c3,c4,c5;
    double QcW_2 = QcW*QcW;
    double QcW_3 = QcW_2*QcW;
    double QcW_4 = QcW_3*QcW;
    double QcW_5 = QcW_4*QcW;
    double QcW_6 = QcW_5*QcW;

    c1 = SQRT2 + sqrt(6);
    c2 = 4+2*sqrt(3);
    c3 = 3*SQRT2 + 2*sqrt(6);
    c4 = 4+2*sqrt(3);
    c5 = SQRT2 + sqrt(6);

    gain = 1/(1+c1*QcW+c2*QcW_2+c3*QcW_3+c4*QcW_4+c5*QcW_5+QcW_6);
    ay[1] = (-6-4*c1*QcW-2*c2*QcW_2+2*c4*QcW_4+4*c5*QcW_5+6*QcW_6)*gain;
    ay[2] = (15+5*c1*QcW-c2*QcW_2-3*c3*QcW_3-c4*QcW_4+5*c5*QcW_5+15*QcW_6)*gain;
    ay[3] = (-20+4*c2*QcW_2-4*c4*QcW_4+20*QcW_6)*gain;
    ay[4] = (15-5*c1*QcW-c2*QcW_2+3*c3*QcW_3-c4*QcW_4-5*c5*QcW_5+15*QcW_6)*gain;
    ay[5] = (-6+4*c1*QcW-2*c2*QcW_2+2*c4*QcW_4-4*c5*QcW_5+6*QcW_6)*gain;
    ay[6] = (1-c1*QcW+c2*QcW_2-c3*QcW_3+c4*QcW_4-c5*QcW_5+QcW_6)*gain;

  }

  //  compute ax
  for(int i=0; i <= order; i++){
    int sign = -1;
    if(i % 2 == 0){ sign = 1; }

    bx[i] = sign*combination(order, i) * gain;
   
  }
}

/*
   Display filter coefficients
*/
void
ButterFilter::showFilter(){
  int i;

  printf("\n=[%s]= Order: %d, Wn: %lf ", filter_type, order, Wn);
  printf("\n  bx: ");
  for(i=0;i <= order;i++){ 
    printf("%e ", bx[i]);
  }
  printf("\n");

  printf("  ay: ");
  for(i=0;i <= order;i++){ 
    printf("%e ", ay[i]);
  }
  printf("\n");
  return;
}

/*
  Foward filtering process
*/
double
ButterFilter::lfilter(double val){
  int i;
  for(i=order; i > 0;i--){
    xv[i] = xv[i-1];
    yv[i] = yv[i-1];
  }
  xv[0] = val;
  
  yv[0] = bx[0]*xv[0];
  for(i=1; i <= order;i++){
    yv[0] += (bx[i]*xv[i] - ay[i]*yv[i]);
  }

  return yv[0];
}

/*
  Zero-phase filtering process
*/
double *
ButterFilter::filtfilt(double *vals, int n){
  int i;
  double *y = new double[n];
  double *out = new double[n];

  for(i=0;i<n;i++){
    y[n-i-1] = lfilter(vals[i]); 
  }

  for(i=0;i<n;i++){
    out[n-i-1] = lfilter(y[i]); 
  }

  delete y;

  return out;
}

