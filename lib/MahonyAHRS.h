//=====================================================================================================
// MahonyAHRS.h
//=====================================================================================================
//
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author			Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=====================================================================================================
#ifndef MahonyAHRS_h
#define MahonyAHRS_h

#include <stdio.h>
#include <math.h>

//----------------------------------------------------------------------------------------------------
// Variable declaration
class Mahony{
private:
  float sampleFreq;	
  float invSampleFreq;	
  float twoKp;			// 2 * proportional gain (Kp)
  float twoKi;			// 2 * integral gain (Ki)
  float q0, q1, q2, q3;	// quaternion of sensor frame relative to auxiliary frame
  float integralFBx, integralFBy, integralFBz;
  float roll, pitch, yaw;
  char  anglesComputed;
  void  computeAngles();

//---------------------------------------------------------------------------------------------------
public:
// Function declarations
    Mahony(float sampleFreq, float Kp, float Ki);

    void begin(float sampleFreq){ invSampleFreq=1.0f/sampleFreq; }
    void update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
    void updateIMU(float gx, float gy, float gz, float ax, float ay, float az);
    float invSqrt(float x);

    float getRoll() {
        if (!anglesComputed) computeAngles();
        return roll * 57.29578f;
    }
    float getPitch() {
        if (!anglesComputed) computeAngles();
        return pitch * 57.29578f;
    }
    float getYaw() {
        if (!anglesComputed) computeAngles();
        return yaw * 57.29578f + 180.0f;
    }
    float getRollRadians() {
        if (!anglesComputed) computeAngles();
        return roll;
    }
    float getPitchRadians() {
        if (!anglesComputed) computeAngles();
        return pitch;
    }
    float getYawRadians() {
        if (!anglesComputed) computeAngles();
        return yaw;
    }
};

#endif
//=====================================================================================================
// End of file
//=====================================================================================================
