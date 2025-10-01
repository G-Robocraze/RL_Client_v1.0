#include "imu.h"
#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <math.h>

// === Constants ===
constexpr uint8_t MPU_ADDRESS = 0x68;
constexpr float MAX_TILT_DEGREES = 90.0;
constexpr float MAX_VIBRATION_RAW = 1.0;
constexpr float SHOCK_ACCEL_THRESHOLD = 3.0;

static MPU9250_asukiaaa imuSensor(MPU_ADDRESS);

// Previous acceleration values
static float prevAccelX = 0, prevAccelY = 0, prevAccelZ = 0;

// Current sensor values
static float ax, ay, az;
static float roll, pitch;
static float tiltPercentage, vibration;
static bool crashDetected;


static float baseRollOffset = 0.0;
static float basePitchOffset = 0.0;


// === Private Utility Functions ===
static float computeTilt(float r, float p) {
  float tilt = sqrt(sq(r) + sq(p));
  tilt = constrain(tilt, 0, MAX_TILT_DEGREES);
  return (tilt / MAX_TILT_DEGREES) * 100.0;
}


static float computeVibration(float x, float y, float z) {
  float mag = sqrt(sq(x - prevAccelX) + sq(y - prevAccelY) + sq(z - prevAccelZ));
  prevAccelX = x;
  prevAccelY = y;
  prevAccelZ = z;
  return constrain(mag / MAX_VIBRATION_RAW, 0.0, 1.0) * 10.0;
}

static bool detectCrash(float x, float y, float z) {
  return sqrt(x*x + y*y + z*z) > SHOCK_ACCEL_THRESHOLD;
}

void calibrateTiltBaseline() {
  imuSensor.accelUpdate();
  imuSensor.gyroUpdate();

  float ax = imuSensor.accelX();
  float ay = imuSensor.accelY();
  float az = imuSensor.accelZ();

  baseRollOffset  = atan2(ay, az) * 180.0 / PI;
  basePitchOffset = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  Serial.print("[CALIBRATION] Base Roll: ");
  Serial.println(baseRollOffset);
  Serial.print("[CALIBRATION] Base Pitch: ");
  Serial.println(basePitchOffset);
}

// === Public API ===
void initIMU() {
  Wire.begin();
  imuSensor.setWire(&Wire);
  imuSensor.beginAccel();
  imuSensor.beginGyro();
  imuSensor.beginMag();
}

void updateIMU() {
    imuSensor.accelUpdate();
    imuSensor.gyroUpdate();

    ax = imuSensor.accelX();
    ay = imuSensor.accelY();
    az = imuSensor.accelZ();

    roll  = atan2(ay, az) * 180 / PI - baseRollOffset;
    pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 180 / PI - basePitchOffset;


    // Serial.println("roll before updation:" + String(roll));
    // Serial.println("pitch before updation:" + String(pitch));
    if (roll > 90)  roll = 180 - roll;
    if (roll < -90) roll = -180 - roll;
    // Serial.println("pitch after updation:" + String(roll));

    tiltPercentage = computeTilt(roll, pitch);
    vibration = computeVibration(ax, ay, az);
    crashDetected = detectCrash(ax, ay, az);
}

float getAccelX() { return ax; }
float getAccelY() { return ay; }
float getAccelZ() { return az; }
float getTiltPercentage() { return tiltPercentage; }
float getVibrationLevel() { return vibration; }
bool isCrashDetected() { return crashDetected; }
