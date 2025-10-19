// Main application for SOS client device
#include <Arduino.h>
#include "config.h"

void applyLoRaDownlink(bool flag, int reason);
#include "lorawan.h"
#include "battery.h"
#include "imu.h"
#include "alert.h"
#include "panic_switch.h"
#include "priority.h"

// Panic state variables
PanicStatus panicStatus = {PANIC_NONE, PANIC_SOURCE_NONE, 0};
unsigned long panicStartTime = 0;
unsigned long lastSendTime = 0;
SirenMode currentSirenMode = SIREN_OFF;
unsigned long lastSirenToggleTime = 0;
bool sirenState = false;
bool lastDownlinkPanicFlag = false;
int lastDownlinkPanicReason = 0;
PanicState lastLoggedState = PANIC_NONE;
PanicSource lastLoggedSource = PANIC_SOURCE_NONE;
bool inputsSuppressed = false;
bool forceImmediateSend = false;
bool downlinkHoldoffActive = false;

void setup() {
    pinMode(SIREN_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(LED_STATUS_PIN, OUTPUT);
    pinMode(PANIC_BUTTON_PIN, INPUT);
    pinMode(BATTERY_MONITOR_PIN, INPUT);

    digitalWrite(SIREN_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_STATUS_PIN, LOW);

    Serial.begin(9600);
    setupLoRa();

  digitalWrite(LED_PIN, HIGH); delay(500); digitalWrite(LED_PIN, LOW);
  setupIMU();
  digitalWrite(LED_STATUS_PIN, HIGH); delay(500); digitalWrite(LED_STATUS_PIN, LOW);
}

void loop() {
    unsigned long currentTime = millis();

    processDownlink();
    bool downlinkEvent = downlinkUpdated || panicFlag != lastDownlinkPanicFlag || panicReason != lastDownlinkPanicReason;
    if (downlinkEvent) {
        Serial.print("[Main] Downlink change detected: flag=");
        Serial.print(panicFlag);
        Serial.print(", reason=");
        Serial.println(panicReason);
        applyLoRaDownlink(panicFlag, panicReason);
        lastDownlinkPanicFlag = panicFlag;
        lastDownlinkPanicReason = panicReason;
        downlinkUpdated = false;
        forceImmediateSend = true;
        Serial.println("[Main] Forcing immediate uplink to report downlink state.");
    }

    if (downlinkHoldoffActive) {
        bool sensorsCleared = !isPanicPressed() && !crashDetected;
        if (sensorsCleared) {
            downlinkHoldoffActive = false;
            panicStatus.source = PANIC_SOURCE_NONE;
            panicStatus.state = PANIC_NONE;
            panicStatus.reason = 0;
            Serial.println("[Main] Downlink hold-off released; local inputs restored.");
        }
    }

    // Check panic sources
    if (panicStatus.source != PANIC_SOURCE_LORAWAN && !downlinkHoldoffActive) {
        if (inputsSuppressed) {
            Serial.println("[Main] Local inputs re-enabled (LoRaWAN override cleared).");
            inputsSuppressed = false;
        }
        if (isPanicPressed()) {
            panicStatus.source = PANIC_SOURCE_BUTTON;
            panicStatus.state = PANIC_TRIGGERED;
            panicStatus.reason = 1;
            panicStartTime = currentTime;
            Serial.println("[Main] Panic button pressed, entering TRIGGERED state.");
        } else if (crashDetected) {
            panicStatus.source = PANIC_SOURCE_IMU;
            panicStatus.state = PANIC_TRIGGERED;
            panicStatus.reason = 2;
            panicStartTime = currentTime;
            Serial.println("[Main] IMU crash detected, entering TRIGGERED state.");
        }
    } else if (!inputsSuppressed) {
        Serial.println("[Main] Local inputs suppressed due to LoRaWAN override/hold-off.");
        inputsSuppressed = true;
    }

    // Escalation logic
    if (panicStatus.state == PANIC_TRIGGERED && (currentTime - panicStartTime > PANIC_LEVEL_2_TIME_MS)) {
        panicStatus.state = PANIC_ESCALATED;
    }

    // Siren and LED
    if (panicStatus.state == PANIC_ESCALATED) {
        setSirenMode(SIREN_FAST_BLINK);
        setLEDStatus(true, true);
    } else if (panicStatus.state == PANIC_TRIGGERED) {
        setSirenMode(SIREN_SLOW_BLINK);
        setLEDStatus(true, false);
    } else {
        setSirenMode(SIREN_OFF);
        setLEDStatus(true, false);
    }
    updateSiren();

    if (panicStatus.state != lastLoggedState || panicStatus.source != lastLoggedSource) {
        Serial.print("[Main] Panic state updated: state=");
        Serial.print(panicStatus.state);
        Serial.print(", source=");
        Serial.print(panicStatus.source);
        Serial.print(", reason=");
        Serial.println(panicStatus.reason);
        unsigned long intervalMs = (panicStatus.state != PANIC_NONE) ? PANIC_SEND_INTERVAL_MS : DEFAULT_SEND_INTERVAL_MS;
        Serial.print("[Main] Uplink interval set to ");
        Serial.print(intervalMs / 1000);
        Serial.println("s");
        lastLoggedState = panicStatus.state;
        lastLoggedSource = panicStatus.source;
    }

    unsigned long targetSendInterval = (panicStatus.state != PANIC_NONE) ? PANIC_SEND_INTERVAL_MS : DEFAULT_SEND_INTERVAL_MS;

    // LoRaWAN send
    if (forceImmediateSend || (currentTime - lastSendTime >= targetSendInterval)) {
        if (!isLoRaConnected()) reconnectLoRa();
        lastSendTime = currentTime;
        float voltage = readBatteryVoltage();
        int batteryPercentage = getBatteryPercentage(voltage);

        SosPayload payload;
        payload.timestamp = currentTime;
        strncpy(payload.deviceId, DEVICE_ID, sizeof(payload.deviceId));
        payload.sosFlag = (panicStatus.state != PANIC_NONE) ? 1 : 0;
        payload.sosReason = panicStatus.reason;
  payload.tilt = int16_t(tiltPercentage * 100);
  payload.vibration = int16_t(vibration * 100);
        payload.batteryPercent = batteryPercentage;
        payload.retryCount = 0;

        sendLoRaPayload(payload);
        forceImmediateSend = false;
    }

    updateIMU();
    delay(100);
}

void checkIMUPanic() {
  if (panicStatus.source == PANIC_SOURCE_LORAWAN) return;  // LoRaWAN override

  if (crashDetected) {
    if (panicStatus.source != PANIC_SOURCE_IMU) {
      panicStatus.source = PANIC_SOURCE_IMU;
      panicStatus.state = PANIC_TRIGGERED;
      panicStatus.reason = 2; // IMU
      panicStartTime = millis();
      Serial.println("[IMU PANIC] Crash detected!");
    } else {
      unsigned long duration = millis() - panicStartTime;
      if (duration >= PANIC_LEVEL_2_TIME_MS && panicStatus.state < PANIC_ESCALATED) {
        panicStatus.state = PANIC_ESCALATED;
        Serial.println("[IMU PANIC] Escalated to LEVEL 2!");
      } else if (duration >= PANIC_LEVEL_1_TIME_MS && panicStatus.state == PANIC_TRIGGERED) {
        Serial.println("[IMU PANIC] Level 1 (held >3s)");
      }
    }
  } else if (panicStatus.source == PANIC_SOURCE_IMU) {
    Serial.println("[IMU PANIC] Cleared.");
    panicStatus.source = PANIC_SOURCE_NONE;
    if (!isAnyPanic(panicStatus)) {
      panicStatus.state = PANIC_NONE;
      panicStatus.reason = 0;
    }
  }
}



// === Panic Helpers ===

// === LoRaWAN Downlink Handling ===

void applyLoRaDownlink(bool flag, int reason) {
  if (flag) {
    // Trigger panic from LoRaWAN
    panicStatus.source = PANIC_SOURCE_LORAWAN;
    panicStatus.state = PANIC_ESCALATED;
    panicStatus.reason = reason;
    panicStartTime = millis();
    downlinkHoldoffActive = false;
    Serial.println("[LORAWAN PANIC] External panic override triggered!");
  } else {
    panicStatus.source = PANIC_SOURCE_LORAWAN;
    panicStatus.state = PANIC_NONE;
    panicStatus.reason = 0;
    downlinkHoldoffActive = true;
    Serial.println("[LORAWAN PANIC] External override cleared panic (hold-off active until sensors reset).");
  }
}
