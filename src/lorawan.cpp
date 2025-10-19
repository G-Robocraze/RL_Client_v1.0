// LoRaWAN implementation
#include "lorawan.h"
#include <ArduinoJson.h>
#include <MKRWAN.h>

LoRaModem modem;

static char currentEUI[24] = LORA_EUI;
static unsigned long sendInterval = DEFAULT_SEND_INTERVAL_MS;
bool panicFlag = false;
int panicReason = 0;
bool downlinkUpdated = false;

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
  modem.poll();
  if (!modem.available()) {
    Serial.println("[LoRa] No pending downlink before uplink.");
  }
  modem.beginPacket();
  modem.write(buffer, length);
  int result = modem.endPacket(true);
  if (result <= 0) {
    Serial.print("[LoRa] Uplink failed, error code ");
    Serial.println(result);
    return false;
  }
  Serial.print("[LoRa] Uplink sent (bytes=");
  Serial.print(length);
  Serial.println(")");
  modem.poll();
  return true;
}

void processDownlink() {
  modem.poll();
  int availableBytes = modem.available();
  if (availableBytes > 0) {
    Serial.print("[Downlink] Bytes available: ");
    Serial.println(availableBytes);
    String downlink;
    downlink.reserve(availableBytes + 1);
    size_t readCount = 0;
    unsigned long deadline = millis() + 100;
    while (millis() <= deadline) {
      while (modem.available()) {
        char c = static_cast<char>(modem.read());
        downlink += c;
        readCount++;
        deadline = millis() + 20; // extend slightly when data keeps arriving
      }
    }
    Serial.print("[Downlink] Read bytes: ");
    Serial.println(readCount);
    Serial.print("[Downlink] Raw payload: ");
    Serial.println(downlink);
    if (downlink.length() == 0) {
      Serial.println("[Downlink] No data read from modem buffer.");
      return;
    }
    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, downlink);
    if (error) {
      Serial.print("[Downlink] JSON error: ");
      Serial.println(error.c_str());
      return;
    }
    bool previousFlag = panicFlag;
    int previousReason = panicReason;

    JsonVariant flagVar = doc["panic_flag"];
    if (!flagVar.isNull()) {
      int flagInt = flagVar.as<int>();
      panicFlag = (flagInt != 0);
    } else {
      Serial.println("[Downlink] Missing panic_flag key.");
    }

    JsonVariant reasonVar = doc["panic_reason"];
    if (!reasonVar.isNull()) {
      panicReason = reasonVar.as<int>();
    } else {
      Serial.println("[Downlink] Missing panic_reason key.");
    }

    Serial.print("[Downlink] Parsed flag: ");
    Serial.print(panicFlag);
    Serial.print(" (prev ");
    Serial.print(previousFlag);
    Serial.print("), reason: ");
    Serial.print(panicReason);
    Serial.print(" (prev ");
    Serial.print(previousReason);
    Serial.println(")");
    downlinkUpdated = true;
  }
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
