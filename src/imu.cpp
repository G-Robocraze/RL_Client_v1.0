
#include "imu.h"
#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <math.h>

MPU9250_asukiaaa imuSensor(0x68);
float vibration = 0.0f;
float tiltPercentage = 0.0f;
bool crashDetected = false;

void setupIMU() {
    Wire.begin();
    imuSensor.beginAccel();
    imuSensor.beginGyro();
}

void updateIMU() {
    imuSensor.accelUpdate();
    float ax = imuSensor.accelX();
    float ay = imuSensor.accelY();
    float az = imuSensor.accelZ();
    float mag = sqrt(ax * ax + ay * ay + az * az);
    vibration = mag;
    tiltPercentage = (fabs(ax) + fabs(ay)) / MAX_TILT_DEGREES * 100.0f;
    crashDetected = mag > SHOCK_ACCEL_THRESHOLD;
}

