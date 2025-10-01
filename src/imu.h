#ifndef IMU_H
#define IMU_H

#include <Arduino.h>

void initIMU();
void updateIMU();

float getAccelX();
float getAccelY();
float getAccelZ();
float getTiltPercentage();
float getVibrationLevel();
bool  isCrashDetected();
void calibrateTiltBaseline();

#endif
