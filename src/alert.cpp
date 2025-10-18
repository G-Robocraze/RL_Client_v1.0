
#include <Arduino.h>
#include "alert.h"

static SirenMode currentMode = SIREN_OFF;
static unsigned long lastToggle = 0;
static bool sirenState = false;

void initAlert() {
    pinMode(SIREN_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(SIREN_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_STATUS_PIN, LOW);
}

void setSirenMode(SirenMode mode) {
    if (currentMode != mode) {
        currentMode = mode;
        lastToggle = millis();
        sirenState = false;
        digitalWrite(SIREN_PIN, LOW);
    }
}

void updateSiren() {
    unsigned long now = millis();
    unsigned long interval = 0;
    switch (currentMode) {
        case SIREN_OFF: digitalWrite(SIREN_PIN, LOW); return;
        case SIREN_STEADY: digitalWrite(SIREN_PIN, HIGH); return;
        case SIREN_FAST_BLINK: interval = 200; break;
        case SIREN_SLOW_BLINK: interval = 600; break;
        case SIREN_BURST: interval = sirenState ? 100 : 900; break;
    }
    if (interval > 0 && now - lastToggle >= interval) {
        sirenState = !sirenState;
        digitalWrite(SIREN_PIN, sirenState ? HIGH : LOW);
        lastToggle = now;
    }
}

void setLEDStatus(bool powerOn, bool panicOn) {
    digitalWrite(LED_PIN, powerOn ? HIGH : LOW);
    digitalWrite(LED_STATUS_PIN, panicOn ? HIGH : LOW);
}
