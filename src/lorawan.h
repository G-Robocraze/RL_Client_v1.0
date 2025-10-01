#ifndef LORAWAN_H
#define LORAWAN_H

#include <Arduino.h>
#include <MKRWAN.h>
#include "init_lora.h"

extern bool panicFlag;
extern int panicReason;
// Data structure for SOS Payload
struct SosPayload {
  uint32_t timestamp;        // 4 bytes
  char deviceId[6];          // 6 bytes
  uint8_t sosFlag;           // 1 byte
  uint8_t sosReason;         // 1 byte
  int16_t tilt;              // 2 bytes (×100)
  int16_t vibration;         // 2 bytes (×100)
  uint8_t batteryPercent;    // 1 byte
  uint8_t retryCount;        // 1 byte
};

// Function to send binary payload over LoRaWAN
bool sendLoRaPayload(SosPayload &payload);

// (Optional) Utility: encode to byte array
void encodePayload(SosPayload &data, uint8_t *buffer, size_t &length);
bool isLoRaConnected();
void reconnectLoRa();
#endif // LORAWAN_H
