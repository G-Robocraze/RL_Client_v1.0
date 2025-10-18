#ifndef ALERT_H
#define ALERT_H

#include "config.h"

enum SirenMode {
    SIREN_OFF,
    SIREN_STEADY,
    SIREN_FAST_BLINK,
    SIREN_SLOW_BLINK,
    SIREN_BURST,
};

void initAlert();
void setSirenMode(SirenMode mode);
void updateSiren();
void setLEDStatus(bool powerOn, bool panicOn);

#endif // ALERT_H
