#include "priority.h"

bool isAnyPanic(const PanicStatus& status) {
    return status.state != PANIC_NONE;
}

PanicSource getPrioritySource(bool imu, bool button, bool lora) {
    if (imu) return PANIC_SOURCE_IMU;
    if (button) return PANIC_SOURCE_BUTTON;
    if (lora) return PANIC_SOURCE_LORAWAN;
    return PANIC_SOURCE_NONE;
}

void escalatePanic(PanicStatus &status, PanicSource source) {
    status.source = source;
    status.state = PANIC_ESCALATED;
}

void deescalatePanic(PanicStatus &status) {
    status.source = PANIC_SOURCE_NONE;
    status.state = PANIC_NONE;
    status.reason = 0;
}
