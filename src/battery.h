#include<Arduino.h>

#define BAT_MONITOR_PIN A1
#define VOLTAGE_DIVIDER_RATIO (3.3 / (5.0 + 3.3))  // 0.3976

float readBatteryVoltage() {
  int raw = analogRead(BAT_MONITOR_PIN);
  float vOut = (raw / 1023.0) * 3.3;
  float vBat = vOut / VOLTAGE_DIVIDER_RATIO;
  return vBat;
}

int batteryLevel2S(float voltage) {
  if (voltage >= 8.4) return 100;
  if (voltage <= 6.0) return 0;
  return int((voltage - 6.0) / (8.4 - 6.0) * 100);
}
