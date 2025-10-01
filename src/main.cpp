#include <Arduino.h>
#include "init_lora.h"
#include "lorawan.h"
#include "battery.h"
#include "imu.h"

#define LED_PIN 5
#define PANIC_BUTTON 4
#define SIREN 2
#define SEND_INTERVAL_MS 30000UL

// === Enums ===
enum PanicSource {
  PANIC_SOURCE_NONE,
  PANIC_SOURCE_LORAWAN,
  PANIC_SOURCE_BUTTON,
  PANIC_SOURCE_IMU
};

enum PanicState {
  PANIC_NONE = 0,
  PANIC_TRIGGERED = 1,
  PANIC_ESCALATED = 2
};

enum SosReason {
  SOS_REASON_NONE = 0,
  SOS_MANUAL = 1,
  SOS_IMU_TRIGGER = 2,
  SOS_OPERATOR_TRIGGER = 3,
  SOS_ADMIN_TRIGGER = 4
};

enum SirenMode {
  SIREN_OFF,
  SIREN_STEADY,
  SIREN_FAST_BLINK,
  SIREN_SLOW_BLINK,
  SIREN_BURST,
};

// === Panic state variables ===
PanicState panicState = PANIC_NONE;
PanicSource panicSource = PANIC_SOURCE_NONE;
SosReason sosReason = SOS_REASON_NONE;

unsigned long panicStartTime = 0;
unsigned long imuPanicStartTime = 0;
unsigned long lastSendTime = 0;

// === Siren control ===
SirenMode currentSirenMode = SIREN_OFF;
unsigned long lastSirenToggleTime = 0;
bool sirenState = false;

// === Constants ===
const unsigned long PANIC_LEVEL_1_TIME = 3000;
const unsigned long PANIC_LEVEL_2_TIME = 10000;

// === Function Declarations ===
void checkButtonPanic();
void checkIMUPanic();
void updateSirenState();
void handleSirenPattern();
void setSirenMode(SirenMode mode);
bool isAnyPanic();
void applyLoRaDownlink(bool flag, int reason);

void setup() {
  pinMode(1, OUTPUT);
  pinMode(SIREN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PANIC_BUTTON, INPUT);
  pinMode(BAT_MONITOR_PIN, INPUT);

  digitalWrite(SIREN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  setupLoRa();

  digitalWrite(LED_PIN, HIGH); delay(500); digitalWrite(LED_PIN, LOW);
  initIMU();
  calibrateTiltBaseline();
  digitalWrite(LED_PIN, HIGH); delay(500); digitalWrite(LED_PIN, LOW);
}

void loop() {
  unsigned long currentTime = millis();

  checkButtonPanic();
  checkIMUPanic();
  updateSirenState();
  handleSirenPattern();
  digitalWrite(LED_PIN, HIGH);

  if (currentTime - lastSendTime >= SEND_INTERVAL_MS) {
    if (!isLoRaConnected()) {
      reconnectLoRa();
    }

    digitalWrite(LED_PIN, LOW);
    lastSendTime = currentTime;

    float voltage = readBatteryVoltage();
    int batteryPercentage = batteryLevel2S(voltage);

    Serial.print("Battery Voltage: ");
    Serial.print(voltage, 2);
    Serial.print(" V, Battery Level: ");
    Serial.print(batteryPercentage);
    Serial.println(" %");

    SosPayload payload;
    payload.timestamp = currentTime;
    strncpy(payload.deviceId, "RL15C3", 6);
    payload.sosFlag = isAnyPanic() ? 1 : 0;
    payload.sosReason = sosReason;
    payload.tilt = int16_t(getTiltPercentage() * 100);
    payload.vibration = int16_t(getVibrationLevel() * 100);
    payload.batteryPercent = batteryPercentage;
    payload.retryCount = 0;

    bool result = sendLoRaPayload(payload);

    if (result) {
      // Check for new downlink (already parsed in lorawan.cpp)
      if (panicFlag != isAnyPanic()) {
        applyLoRaDownlink(panicFlag, panicReason);
      }
    }
  }

  updateIMU();
  delay(100);
}

// === Panic logic ===

void checkButtonPanic() {
  bool pressed = digitalRead(PANIC_BUTTON) == HIGH;

  if (pressed && panicSource != PANIC_SOURCE_BUTTON) {
    panicStartTime = millis();
    panicSource = PANIC_SOURCE_BUTTON;
    panicState = PANIC_TRIGGERED;
    sosReason = SOS_MANUAL;
    Serial.println("[PANIC] Manual button panic triggered!");
  }

  if (pressed && panicSource == PANIC_SOURCE_BUTTON) {
    unsigned long duration = millis() - panicStartTime;
    if (duration >= PANIC_LEVEL_2_TIME && panicState < PANIC_ESCALATED) {
      panicState = PANIC_ESCALATED;
      Serial.println("[PANIC] Button escalated to LEVEL 2!");
    } else if (duration >= PANIC_LEVEL_1_TIME && panicState == PANIC_TRIGGERED) {
      Serial.println("[PANIC] Button panic LEVEL 1 (held >3s)");
    }
  }

  if (!pressed && panicSource == PANIC_SOURCE_BUTTON) {
    Serial.println("[PANIC] Button released, clearing panic.");
    panicSource = PANIC_SOURCE_NONE;
    if (!isAnyPanic()) {
      panicState = PANIC_NONE;
      sosReason = SOS_REASON_NONE;
    }
  }
}

void checkIMUPanic() {
  if (panicSource == PANIC_SOURCE_LORAWAN) return;  // LoRaWAN override

  if (isCrashDetected()) {
    if (panicSource != PANIC_SOURCE_IMU) {
      panicSource = PANIC_SOURCE_IMU;
      panicState = PANIC_TRIGGERED;
      imuPanicStartTime = millis();
      sosReason = SOS_IMU_TRIGGER;
      Serial.println("[IMU PANIC] Crash detected!");
    } else {
      unsigned long duration = millis() - imuPanicStartTime;
      if (duration >= PANIC_LEVEL_2_TIME && panicState < PANIC_ESCALATED) {
        panicState = PANIC_ESCALATED;
        Serial.println("[IMU PANIC] Escalated to LEVEL 2!");
      } else if (duration >= PANIC_LEVEL_1_TIME && panicState == PANIC_TRIGGERED) {
        Serial.println("[IMU PANIC] Level 1 (held >3s)");
      }
    }
  } else if (panicSource == PANIC_SOURCE_IMU) {
    Serial.println("[IMU PANIC] Cleared.");
    panicSource = PANIC_SOURCE_NONE;
    if (!isAnyPanic()) {
      panicState = PANIC_NONE;
      sosReason = SOS_REASON_NONE;
    }
  }
}

// === Siren logic ===

void updateSirenState() {
  switch (panicState) {
    case PANIC_TRIGGERED: setSirenMode(SIREN_SLOW_BLINK); break;
    case PANIC_ESCALATED: setSirenMode(SIREN_FAST_BLINK); break;
    default: setSirenMode(SIREN_OFF); break;
  }
}

void setSirenMode(SirenMode mode) {
  if (currentSirenMode != mode) {
    currentSirenMode = mode;
    lastSirenToggleTime = millis();
    sirenState = false;
    digitalWrite(SIREN, LOW);
  }
}

void handleSirenPattern() {
  unsigned long now = millis();
  unsigned long interval = 0;

  switch (currentSirenMode) {
    case SIREN_OFF: digitalWrite(SIREN, LOW); return;
    case SIREN_STEADY: digitalWrite(SIREN, HIGH); return;
    case SIREN_FAST_BLINK: interval = 200; break;
    case SIREN_SLOW_BLINK: interval = 600; break;
    case SIREN_BURST: interval = sirenState ? 100 : 900; break;
  }

  if (now - lastSirenToggleTime >= interval) {
    sirenState = !sirenState;
    digitalWrite(SIREN, sirenState ? HIGH : LOW);
    lastSirenToggleTime = now;
  }
}

// === Panic Helpers ===

bool isAnyPanic() {
  return panicState != PANIC_NONE;
}

// === LoRaWAN Downlink Handling ===

void applyLoRaDownlink(bool flag, int reason) {
  if (flag) {
    // Trigger panic from LoRaWAN
    panicSource = PANIC_SOURCE_LORAWAN;
    panicState = PANIC_ESCALATED;
    sosReason = static_cast<SosReason>(reason);
    Serial.println("[LORAWAN PANIC] External panic override triggered!");
  } else if (panicSource == PANIC_SOURCE_LORAWAN) {
    // Only override if current panic is from LoRaWAN
    panicSource = PANIC_SOURCE_NONE;
    panicState = PANIC_NONE;
    sosReason = SOS_REASON_NONE;
    Serial.println("[LORAWAN PANIC] External override cleared panic.");
  }
}
