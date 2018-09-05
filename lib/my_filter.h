/*


*/
#ifndef __FILTER_H__
#define __FILTER_H__

#include <math.h>

#define SQRT2  1.4142135623730950488

class MyFilter
{
public:
  double xv[3];
  double yv[3];

  double by[3];
  double ax[3];
  double QcRaw;
  double QcWarp;
  double gain;


  MyFilter(double Wn, int type);
  void genLowPass(double Wn);
  void genHighPass(double Wn);

  double fitFilter(double val);

};

#endif
