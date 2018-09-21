/*


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


