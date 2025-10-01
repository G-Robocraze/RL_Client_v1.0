#include "init_lora.h"

// Define the LoRaWAN credentials here
const char *devEui = "a8610a3339358316";
const char *appEui = "be7d8b422668287a";
const char *appKey = "cbf941503c0a56929b340824940a3534";

// LoRa modem instance
LoRaModem modem;

void setupLoRa() {
  Serial.println("[LoRa] Initializing modem...");
  if (!modem.begin(LORA_REGION)) {
    // Serial.println("[LoRa] Failed to start module.");
    while (1);
  }

  modem.sleep(); // Optional: wake up on join

  // Serial.println("[LoRa] Joining network (OTAA)...");
  int connected = modem.joinOTAA(appEui, appKey, devEui);

  if (!connected) {
    // Serial.println("[LoRa] OTAA join failed!");
    while (1);
  }

  // Serial.println("[LoRa] Joined successfully.");
  printLoRaConfig();
}

void printLoRaConfig() {
  // Serial.print("[LoRa] Device EUI: ");
  // Serial.println(modem.deviceEUI());
}
