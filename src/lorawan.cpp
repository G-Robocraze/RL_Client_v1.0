#include "lorawan.h"
#include <ArduinoJson.h>
bool panicFlag;
int panicReason;
bool decodeDownlinkPayload(Stream &stream, bool &panicFlag, int &panicReason) {
  const size_t capacity = JSON_OBJECT_SIZE(2) + 50;
  StaticJsonDocument<capacity> doc;

  // Read full JSON string from stream
  String jsonString;
  while (stream.available()) {
    char c = stream.read();
    jsonString += c;
  }

  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print("[JSON] Deserialize failed: ");
    Serial.println(error.c_str());
    return false;
  }

  // Extract values
  if (doc.containsKey("panic_flag") && doc.containsKey("panic_reason")) {
    panicFlag = doc["panic_flag"];
    panicReason = doc["panic_reason"];
    Serial.print("[JSON] panic_flag: ");
    Serial.print(panicFlag);
    Serial.print(", panic_reason: ");
    Serial.println(panicReason);
    return true;
  } else {
    Serial.println("[JSON] Required keys missing");
    return false;
  }
}
bool sendLoRaPayload(SosPayload &payload) {
  uint8_t buffer[32];
  size_t length = 0;

  encodePayload(payload, buffer, length);

  modem.beginPacket();
  modem.write(buffer, length);
  int result = modem.endPacket(true);  // true = async

  if (result > 0) {
    // Serial.println("[LoRa] Packet sent successfully.");
    // === Try to receive downlink ===
    int size = modem.parsePacket();
    if (size > 0) {
      Serial.print("[LoRa] Downlink received (bytes: ");
      Serial.print(size);
      Serial.println("):");

      if (decodeDownlinkPayload(modem, panicFlag, panicReason)) {
        Serial.println("[LoRa] ✅ Downlink JSON decoded successfully!");
        // Optional: handle the panic logic here
        if (panicFlag) {
          Serial.println("[LoRa] Panic triggered from downlink!");
          // ... set internal panic state ...
        }
      } else {
        Serial.println("[LoRa] ❌ Failed to decode downlink JSON.");
      }
    }
    return true;
  } else {
    // Serial.println("[LoRa] Failed to send packet.");
    return false;
  }
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

  int connected = modem.joinOTAA(appEui, appKey, devEui);
  if (connected) {
    Serial.println("[LoRa] Reconnected successfully.");
  } else {
    Serial.println("[LoRa] Reconnect failed.");
  }
}
