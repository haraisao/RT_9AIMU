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
    filter_type="Low-pass";
    genHighPass(Wn);
  }else{
    filter_type="High-pass";
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
    printf("%2.8lf ", bx[i]);
  }
  printf("\n");

  printf("  ay: ");
  for(i=0;i <= order;i++){ 
    printf("%2.8lf ", ay[i]);
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

