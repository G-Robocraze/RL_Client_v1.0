
#include <Arduino.h>
#include "panic_switch.h"

static unsigned long lastPressTime = 0;
static int patternCount = 0;
static unsigned long patternStartTime = 0;

void initPanicSwitch() {
    pinMode(PANIC_BUTTON_PIN, INPUT);
}

bool isPanicPressed() {
    static bool lastState = LOW;
    bool currentState = digitalRead(PANIC_BUTTON_PIN);
    if (currentState != lastState) {
        delay(20); // debounce
        lastState = currentState;
    }
    return currentState == HIGH;
}

bool checkDeescalatePattern() {
    bool pressed = isPanicPressed();
    unsigned long now = millis();
    if (pressed) {
        if (patternCount == 0) patternStartTime = now;
        patternCount++;
        lastPressTime = now;
    }
    if (patternCount >= 6 && (now - patternStartTime <= DEESCALATE_PATTERN_MAX_TIME_MS)) {
        patternCount = 0;
        return true;
    }
    if (now - lastPressTime > 2000) {
        patternCount = 0;
    }
    return false;
}
