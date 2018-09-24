/*


*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

void displayArray(double *data, int n);
double *shift_and_push(double val, double *data, int n);
double calc_global_acc(double ax, double ay, double az,double roll, double pitch, double yaw, double acc[3]);

#ifdef __cplusplus
}
#endif

