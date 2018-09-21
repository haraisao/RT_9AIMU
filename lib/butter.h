/*
  Copyright(C) 2018 Isao Hara.
  All right reserved.

*/
#ifndef __BUTTER_FILTER_H__
#define __BUTTER_FILTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQRT2  1.4142135623730950488
#define SQRT3  1.7320508075688772935
#define SQRT5  2.2360679774997896964
#define SQRT6  2.4494897427831783


#define M_PI_DIV_2  M_PI*0.5

/*
  Butterworth filter
*/
class ButterFilter
{
public:
  int order;
  double Wn;
  const char *filter_type;

  double *xv;
  double *yv;

  double *ay;
  double *bx;


  double QcW;
  double gain;


  ButterFilter(int n, double Wn, int type);
  ~ButterFilter(){
    delete xv;
    delete yv;
    delete ay;
    delete bx;
  };

  void genLowPass(double Wn);
  void genHighPass(double Wn);

  void showFilter();

  double lfilter(double val);
  double *filtfilt(double *vals, int n);

};

#endif
