/*
  Copyright(C) 2018 Isao Hara.
  All right reserved.

*/
#ifndef __BUTTER_FILTER_H__
#define __BUTTER_FILTER_H__

#include <stdio.h>
#include <math.h>

#define SQRT2  1.4142135623730950488
#define M_PI_DIV_2  M_PI*0.5

class ButterFilter
{
public:
  int order;
  double *xv;
  double *yv;

  double *by;
  double *ax;


  double QcWarp;
  double gain;


  ButterFilter(int n, double Wn, int type);
  void genLowPass(double Wn);
  void genHighPass(double Wn);

  void showFilter();

  double lfilter(double val);

};

#endif
