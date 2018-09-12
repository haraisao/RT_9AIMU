/*
  Butterworth filter
  Copyright(c) 2018 Isao Hara, all rights reserveed.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


float
combination(int n, int m){
  int i;
  float res=1;

  if(n < m) return 0;
  for(i=0, res=1; i<m; i++){
    res *= (float)(n-i)/(i+1);
  }
  return res;
}


void
calc_coeff_sub(int n, int m,  double *c)
{
  double a;

  if (m <= 0){
    c[0] = 1;
    c[1] = 1;
    return;  

  }else if (m==2){
    c[0] = 1;
    c[1] = -2*cos((m+n-1)*M_PI/(2.0*n));
    c[2] = 1;

  }else{
    calc_coeff_sub(n, m-2, c);
    a = -2*cos((m+n-1)*M_PI/(2.0*n));
    c[m]=1;
    for(int i=m/2;i>=1;i--){
      c[2*i-1] = c[2*i-1]+c[2*i-2]*a+c[2*i-3];
      c[2*i-2] = c[2*i-2]+c[2*i-3]*a+c[2*i-4];
    }
  }

  return;
}

double *
calc_coeff_c(int n)
{
  int m = n/2;
  double *c=(double *)malloc(sizeof(double)*(n+1));

  c[0] = 1;
  c[n] = 1;

  if( (n % 2)  == 0){
    if (m==1){
      c[1] = -2*cos(3*M_PI/4);

    }else{
      double a= -2*cos((2.0*m+n-1)*M_PI/(2.0*n));
      calc_coeff_sub(n, n-2, c);

      for(int i=m;i>=1;i--){
        c[2*i-1] = c[2*i-1]+c[2*i-2]*a+c[2*i-3];
        c[2*i-2] = c[2*i-2]+c[2*i-3]*a+c[2*i-4];
      }
    }
  }else{
    if (m==0){
      return c;
    }else{
      calc_coeff_sub(n, n-1, c);

      for(int i=m;i>=1;i--){
        c[2*i] = c[2*i] + c[2*i-1];
        c[2*i-1] = c[2*i-1]+c[2*i-2];
      }
    }
  }
  return c; 
}

double *
calc_coeff_a(int n, int k)
{
  double *x;

  x=(double *)malloc(sizeof(double)*(n+1));
  for(int i=0; i<=n; i++){
    if(k % 2){
      x[i] = (-1)*combination(i, k) ;
    }else{
      x[i] = combination(i, k) ;
    }
    for(int j=1; j<k; j++){
      if (j % 2){
        x[i] -= combination(i, j)*combination(n-i,k-j) ;
      }else{
        x[i] += combination(i, j)*combination(n-i,k-j) ;
      }
    }
    x[i] += combination(n-i,k) ;
  }
  return x;
}

void
calc_filter_coefficients(int n, double Wn,  double *b, double *a, int flag)
{
  double *W, QcW, G;
  double gain;
  int i,k;

  double *c = calc_coeff_c(n);

  if(flag==0){
    QcW = 1.0/tan( M_PI/2.0 * Wn);
  }else{
    QcW = tan( M_PI/2.0 * Wn);
  }

  W = (double *)malloc(sizeof(double)*(n+1));
  W[0]=1;

  for(i=1;i<=n;i++){ W[i] = W[i-1]*QcW; }
  for(i=1,G=1;i<=n;i++){ G += c[i]*W[i]; }

  gain = 1.0 / G;

  ////////////////
  a[0]=1;

  for(k=1, a[k]=0; k <= n; k++){
    double *x=calc_coeff_a(n, k);
    for(i=0; i<=n; i++){
      if(flag==0){
        a[k] += x[i]*c[i]*W[i];
      }else{
        a[k] += x[n-i]*c[i]*W[i];
      }
    }
    a[k] *= gain;
    free(x);
  }

  ////////////////
  for(i=0; i <= n; i++){
    int sign = -1;
    if (flag == 0 || i % 2 == 0){ sign = 1; }
    b[i] = sign*combination(n, i) * gain;
  }

  free(W);
  free(c);
}

int
main(int argc, char **argv)
{
   int n, flag;
   double Wn;
   double *a,*b,*c;

   printf(" n> ");
   scanf("%d", &n); 
   printf(" Wn(0.0 - 1.0)> ");
   scanf("%lf", &Wn); 
   printf(" Type (low=0, high=1)> ");
   scanf("%d", &flag); 

   /////
   a=(double *)malloc(sizeof(double)*(n+1));
   b=(double *)malloc(sizeof(double)*(n+1));
   /////

   calc_filter_coefficients(n, Wn, b, a, flag);

   /////
   printf("\nb: ");
   for(int i=0; i<=n;i++){
     printf("%2.12lf ", b[i]);
   } 

   printf("\na: ");
   for(int i=0; i<=n;i++){
     printf("%2.12lf ", a[i]);
   } 
   printf("\n=================\n");
   printf("\n");
}
