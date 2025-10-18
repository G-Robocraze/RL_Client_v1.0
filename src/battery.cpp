
#include <Arduino.h>
#include "battery.h"

float readBatteryVoltage() {
    int raw = analogRead(BATTERY_MONITOR_PIN);
    float vOut = (raw / 1023.0) * 3.3;
    float vBat = vOut / (3.3 / (5.0 + 3.3));
    return vBat;
}

int getBatteryPercentage(float voltage) {
    if (voltage >= BATTERY_FULL_VOLTAGE) return 100;
    if (voltage <= BATTERY_EMPTY_VOLTAGE) return 0;
    return int((voltage - BATTERY_EMPTY_VOLTAGE) / (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE) * 100);
}

bool isLowBattery(float voltage) {
    return voltage < (BATTERY_EMPTY_VOLTAGE + 0.2f);
}
