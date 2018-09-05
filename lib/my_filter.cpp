/*

*/
#include "my_filter.h"

MyFilter::MyFilter(double Wn, int type)
{
  xv[0]=xv[1]=xv[2]=0.0;
  yv[0]=yv[1]=yv[2]=0.0;
  
  if (type == 1){
    genHighPass(Wn);
  }else{
    genLowPass(Wn);
  }

}

void MyFilter::genLowPass(double Wn)
{
  //Lowpass
  QcRaw  = M_PI * Wn / 2.0; // Find cutoff frequency in [0..PI]
  QcWarp = tan(QcRaw); // Warp cutoff frequency
  gain = 1 / (1 + SQRT2/QcWarp + 1/(QcWarp*QcWarp));

  by[0] = 1;
  by[1] = (2 - 2/(QcWarp*QcWarp)) * gain;
  by[2] = (1 - SQRT2/QcWarp + 1/(QcWarp*QcWarp)) * gain;
  ax[0] = 1 * gain;
  ax[1] = 2 * gain;
  ax[2] = 1 * gain;
}

void MyFilter::genHighPass(double Wn)
{
  //Highpass
  QcRaw  = M_PI * Wn / 2.0; // Find cutoff frequency in [0..PI]
  QcWarp = tan(QcRaw); // Warp cutoff frequency
  gain = 1 / (1 + SQRT2 * QcWarp + (QcWarp*QcWarp));

  by[0] = 1;
  by[1] = (2*(QcWarp*QcWarp) - 2.0) * gain;
  by[2] = (1 - SQRT2*QcWarp + (QcWarp*QcWarp)) * gain;
  ax[0] = 1 * gain;
  ax[1] = -2 * gain;
  ax[2] = 1 * gain;
}


double
MyFilter::fitFilter(double val){
  xv[2]=xv[1]; xv[1]=xv[0];
  xv[0] = val;

  yv[2]=yv[1]; yv[1]=yv[0];
  
  yv[0] = (ax[0]*xv[0] + ax[1]*xv[1] + ax[2] * xv[2]
           - by[1] *yv[1] - by[2] *yv[2]);

  return yv[0];

}


