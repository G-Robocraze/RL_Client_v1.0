
#ifndef PANIC_SWITCH_H
#define PANIC_SWITCH_H

#include "config.h"

void initPanicSwitch();
bool isPanicPressed();
bool checkDeescalatePattern(); // Detect 101010 pattern for de-escalation

#endif // PANIC_SWITCH_H
