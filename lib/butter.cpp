/*
  Butterworth filter

  Copyright(C) 2018 Isao Hara.
  All right reserved.
*/
#include "butter.h"

ButterFilter::ButterFilter(int n, double Wn, int type)
{
  order=n;
  xv = new double[ n+1 ];
  yv = new double[ n+1 ];
  
  ax = new double[ n+1 ];
  by = new double[ n+1 ];

  for(int i; i <= order; i++){
    xv[i] = 0.0;
    yv[i] = 0.0;
    ax[i] = 0.0;
    by[i] = 0.0;
  }
  
  if (type == 1){
    genHighPass(Wn);
  }else{
    genLowPass(Wn);
  }
}

void ButterFilter::genLowPass(double Wn)
{
  //Lowpass
  QcWarp = 1.0/tan( M_PI_DIV_2 * Wn);

  if (order == 1){
    gain = 1 / (1 + QcWarp);
    by[0] = 1;
    by[1] = (1 - QcWarp) * gain;
    ax[0] = 1 * gain;
    ax[1] = 1 * gain;

  }else if (order == 2){
    gain = 1 / (1 + SQRT2 * QcWarp + QcWarp*QcWarp);

    by[0] = 1;
    by[1] = (2 - 2*QcWarp*QcWarp) * gain;
    by[2] = (1 - SQRT2*QcWarp + QcWarp*QcWarp) * gain;
    ax[0] = 1 * gain;
    ax[1] = 2 * gain;
    ax[2] = 1 * gain;
  }
}

void ButterFilter::genHighPass(double Wn)
{
  //Highpass
  QcWarp = tan( M_PI_DIV_2 * Wn);

  if (order == 1){
    gain = 1 / (1 + QcWarp);
    by[0] = 1;
    by[1] = (QcWarp -1) * gain;
    ax[0] = 1 * gain;
    ax[1] = -1 * gain;

  }else if (order == 2){
    gain = 1 / (1 + SQRT2 * QcWarp + (QcWarp*QcWarp));

    by[0] = 1;
    by[1] = (2*QcWarp*QcWarp - 2.0) * gain;
    by[2] = (1 - SQRT2*QcWarp + QcWarp*QcWarp) * gain;
    ax[0] = 1 * gain;
    ax[1] = -2 * gain;
    ax[2] = 1 * gain;
  }
}

void
ButterFilter::showFilter(){
  int i;

  printf("\nax: ");
  for(i=0;i <= order;i++){ 
    printf("%2.8lf ", ax[i]);
  }
  printf("\n");

  printf("by: ");
  for(i=0;i <= order;i++){ 
    printf("%2.8lf ", by[i]);
  }
  printf("\n");
  return;
}


double
ButterFilter::lfilter(double val){
  int i;
  for(i=order; i > 0;i--){
    xv[i] = xv[i-1];
    yv[i] = yv[i-1];
  }
  xv[0] = val;
  
  yv[0] = ax[0]*xv[0];
  for(i=1; i <= order;i++){
    yv[0] += (ax[i]*xv[i] - by[i]*yv[i]);
  }

  return yv[0];
}


