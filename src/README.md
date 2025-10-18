# Project Map: SOS Client Device for Landslide-Prone Areas

## 1. **Main Application**
- **File:** `main.cpp`
- **Responsibilities:**
  - Device initialization and main loop
  - State management (panic, escalation, de-escalation)
  - Coordination of all modules

---

## 2. **IMU Sensor Module**
- **File:** `imu.cpp`, `imu.h`
- **Responsibilities:**
  - IMU initialization and calibration
  - Crash/fall detection logic
  - Reporting tilt and vibration data

---

## 3. **Emergency Switch Module**
- **File:** `panic_switch.cpp`, `panic_switch.h`
- **Responsibilities:**
  - Manual panic trigger detection
  - Pattern recognition for de-escalation (e.g., 101010 pattern)
  - Debouncing and timing logic

---

## 4. **LoRaWAN Communication Module**
- **File:** `lorawan.cpp`, `lorawan.h`
- **Responsibilities:**
  - Uplink: periodic status and panic messages
  - Downlink: receive and process remote commands (panic, de-escalation)
  - Connection management

---

## 5. **Siren & LED Control Module**
- **File:** `alert.cpp`, `alert.h`
- **Responsibilities:**
  - Siren activation (120dB) and pattern control
  - LED status indication (2 LEDs: power, panic/alert)
  - Escalation/de-escalation feedback

---

## 6. **Battery Management Module**
- **File:** `battery.cpp`, `battery.h`
- **Responsibilities:**
  - Voltage reading and battery percentage calculation
  - Low battery warning logic

---

## 7. **Priority & Escalation Logic**
- **File:** `priority.cpp`, `priority.h`
- **Responsibilities:**
  - Weighted priority assignment for panic sources (IMU, switch, LoRa)
  - Escalation and de-escalation state machine
  - Decision logic for alarm activation

---

## 8. **Configuration & Constants**
- **File:** `config.h`
- **Responsibilities:**
  - Pin assignments
  - Timing constants
  - Device ID and other static parameters

---

## 9. **README & Documentation**
- **File:** `README.md`
- **Responsibilities:**
  - Project overview, features, usage, and setup instructions

---

## 10. **(Optional) Test & Simulation**
- **File:** `test.cpp`, `test.h`
- **Responsibilities:**
  - Unit tests and simulation routines for core modules

---
