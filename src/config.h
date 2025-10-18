
#ifndef CONFIG_H
#define CONFIG_H

// Device Info
#define DEVICE_ID "RL15C3" // Change per device

// LoRaWAN Credentials (replace with actual values)
#define LORA_REGION IN865
#define LORA_EUI "0004A30B001C0530"
const char LORA_APP_EUI[] = "be7d8b422668287a";
const char LORA_APP_KEY[] = "cbf941503c0a56929b340824940a3534";
const char LORA_DEV_EUI[] = "a8610a343345800d";

// Pin Assignments
#define LED_PIN 5
#define LED_STATUS_PIN 6
#define PANIC_BUTTON_PIN 4
#define SIREN_PIN 2
#define BATTERY_MONITOR_PIN A0
#define IMU_SDA_PIN 21
#define IMU_SCL_PIN 22

// Timing Constants
#define DEFAULT_SEND_INTERVAL_MS 30000UL
#define PANIC_LEVEL_1_TIME_MS 3000
#define PANIC_LEVEL_2_TIME_MS 10000
#define DEESCALATE_PATTERN_MAX_TIME_MS 10000

// Siren
#define SIREN_DB_LEVEL 120

// Battery
#define BATTERY_FULL_VOLTAGE 8.4f
#define BATTERY_EMPTY_VOLTAGE 6.0f

#endif // CONFIG_H
