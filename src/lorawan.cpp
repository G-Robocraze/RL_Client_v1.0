// LoRaWAN implementation
#include "lorawan.h"
#include <ArduinoJson.h>
#include <MKRWAN.h>

LoRaModem modem;

#define LORA_REGION EU868
static char currentEUI[24] = LORA_EUI;
static unsigned long sendInterval = DEFAULT_SEND_INTERVAL_MS;
bool panicFlag = false;
int panicReason = 0;

void setupLoRa() {
  Serial.println("[LoRa] Initializing modem...");
  if (!modem.begin(LORA_REGION)) {
    Serial.println("[LoRa] Modem init failed");
    return;
  }
  int connected = modem.joinOTAA(LORA_APP_EUI, LORA_APP_KEY, LORA_DEV_EUI);
  if (connected) {
    Serial.println("[LoRa] Joined network");
  } else {
    Serial.println("[LoRa] Join failed");
  }
}

// ...existing code...

void setLoRaEUI(const char* eui) {
  strncpy(currentEUI, eui, sizeof(currentEUI)-1);
  currentEUI[sizeof(currentEUI)-1] = '\0';
}

void setSendInterval(unsigned long intervalMs) {
  sendInterval = intervalMs;
}


static void encodePayload(const SosPayload &data, uint8_t *buffer, size_t &length) {
  length = 0;
  memcpy(&buffer[length], &data.timestamp, 4); length += 4;
  memcpy(&buffer[length], data.deviceId, 8);   length += 8;
  buffer[length++] = data.sosFlag;
  buffer[length++] = data.sosReason;
  memcpy(&buffer[length], &data.tilt, 2);      length += 2;
  memcpy(&buffer[length], &data.vibration, 2); length += 2;
  buffer[length++] = data.batteryPercent;
  buffer[length++] = data.retryCount;
}

bool sendLoRaPayload(const SosPayload &payload) {
  uint8_t buffer[32];
  size_t length = 0;
  encodePayload(payload, buffer, length);
  modem.beginPacket();
  modem.write(buffer, length);
  int result = modem.endPacket(true);
  return (result > 0);
}

void processDownlink() {
  if (modem.available()) {
    String downlink = modem.readString();
    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, downlink);
    if (error) {
      Serial.print("[Downlink] JSON error: ");
      Serial.println(error.c_str());
      return;
    }
  if (doc["panic_flag"].is<bool>()) {
    panicFlag = doc["panic_flag"].as<bool>();
  }
  if (doc["panic_reason"].is<int>()) {
    panicReason = doc["panic_reason"].as<int>();
  }
    Serial.print("[Downlink] panicFlag: ");
    Serial.print(panicFlag);
    Serial.print(", panicReason: ");
    Serial.println(panicReason);
  }
}

bool sendLoRaPayload(SosPayload &payload) {
  // TODO: Implement LoRaWAN send logic here. Example:
  // encodePayload(payload, buffer, length);
  // modem.beginPacket();
  // modem.write(buffer, length);
  // int result = modem.endPacket(true);  // true = async
  // return (result > 0);
  return true;
}

void encodePayload(SosPayload &data, uint8_t *buffer, size_t &length) {
  length = 0;

  memcpy(&buffer[length], &data.timestamp, 4); length += 4;
  memcpy(&buffer[length], data.deviceId, 6);   length += 6;
  buffer[length++] = data.sosFlag;
  buffer[length++] = data.sosReason;
  memcpy(&buffer[length], &data.tilt, 2);      length += 2;
  memcpy(&buffer[length], &data.vibration, 2); length += 2;
  buffer[length++] = data.batteryPercent;
  buffer[length++] = data.retryCount;
}


bool isLoRaConnected() {
  return modem.connected(); // true if joined to network
}

void reconnectLoRa() {
  Serial.println("[LoRa] Attempting reconnect...");
  if (!modem.begin(LORA_REGION)) {
    Serial.println("[LoRa] Failed to restart modem.");
    return;
  }

  int connected = modem.joinOTAA(LORA_APP_EUI, LORA_APP_KEY, LORA_DEV_EUI);
  if (connected) {
    Serial.println("[LoRa] Reconnected successfully.");
  } else {
    Serial.println("[LoRa] Reconnect failed.");
  }
}
