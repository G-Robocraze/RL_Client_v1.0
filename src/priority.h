#ifndef PRIORITY_H
#define PRIORITY_H

#include "config.h"

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

struct PanicStatus {
    PanicState state;
    PanicSource source;
    int reason;
};

// Weighted priorities (higher = more important)
#define PRIORITY_IMU 3
#define PRIORITY_BUTTON 2
#define PRIORITY_LORAWAN 1

// Function declarations
bool isAnyPanic(const PanicStatus& status);
PanicSource getPrioritySource(bool imu, bool button, bool lora);
void escalatePanic(PanicStatus &status, PanicSource source);
void deescalatePanic(PanicStatus &status);

#endif // PRIORITY_H
