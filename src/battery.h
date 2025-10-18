

#ifndef BATTERY_H
#define BATTERY_H

#include "config.h"

float readBatteryVoltage();
int getBatteryPercentage(float voltage);
bool isLowBattery(float voltage);

#endif // BATTERY_H
