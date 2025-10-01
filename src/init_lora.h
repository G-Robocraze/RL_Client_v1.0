#ifndef INIT_LORA_H
#define INIT_LORA_H

#include <Arduino.h>
#include <MKRWAN.h>

// Declare the LoRaWAN keys as extern
extern const char *devEui;
extern const char *appEui;
extern const char *appKey;

// LoRaWAN region
#define LORA_REGION IN865

extern LoRaModem modem;

void setupLoRa();
void printLoRaConfig();

#endif // INIT_LORA_H
