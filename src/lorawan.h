
#ifndef LORAWAN_H
#define LORAWAN_H

#include <Arduino.h>
#include "config.h"

extern bool panicFlag;
extern int panicReason;
extern bool downlinkUpdated;

struct SosPayload {
    unsigned long timestamp;
    char deviceId[8];
    int sosFlag;
    int sosReason;
    int tilt;
    int vibration;
    int batteryPercent;
    int retryCount;
};

void setupLoRa();
bool isLoRaConnected();
void reconnectLoRa();
bool sendLoRaPayload(const SosPayload &payload);
void setLoRaEUI(const char* eui);
void setSendInterval(unsigned long intervalMs);
void processDownlink();



#endif // LORAWAN_H
