
#ifndef IMU_H
#define IMU_H

#include <MPU9250_asukiaaa.h>
#include <Arduino.h>

extern MPU9250_asukiaaa imuSensor;
extern float vibration;
extern float tiltPercentage;
extern bool crashDetected;

#define MAX_TILT_DEGREES 90.0f
#define SHOCK_ACCEL_THRESHOLD 2.5f

void setupIMU();
void updateIMU();

#endif // IMU_H
