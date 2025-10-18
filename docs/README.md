# SOS Client Documentation

> **Viewing Instructions:**
> - Install the "Markdown Preview Enhanced" extension in VS Code
> - Press `Ctrl+K V` to open preview
> - Or use `Ctrl+Shift+V` for side-by-side preview
> - Code blocks and formatting will be properly highlighted

## Overview
This is an Arduino-based SOS client device designed for deployment in landslide-prone areas. It provides real-time monitoring through IMU sensors, manual panic button input, and LoRaWAN communication capabilities.

## System Architecture

### Core Components
1. **IMU Sensor (MPU9250)**
   - Monitors tilt and vibration
   - Detects potential landslide conditions
   - Provides crash/movement detection

2. **Panic Button**
   - Manual emergency trigger
   - Debounced input handling
   - Pattern detection for false trigger prevention

3. **Alert System**
   - Dual LED indicators (Power and Status)
   - Configurable siren with multiple patterns
   - Visual and audible emergency notifications

4. **LoRaWAN Communication**
   - Over-the-air device configuration
   - Real-time status reporting
   - Remote panic triggering capability
   - Configurable send intervals

5. **Power Management**
   - Battery voltage monitoring
   - Low battery detection
   - Power status indication

## Module Details

### 1. Configuration (config.h)
- Device identification
- Pin assignments
- Timing constants
- LoRaWAN credentials
- Alert thresholds

### 2. IMU Module (imu.h/cpp)
- Functions:
  - `setupIMU()`: Initializes IMU sensor
  - `updateIMU()`: Updates sensor readings
- Variables:
  - `vibration`: Current vibration level
  - `tiltPercentage`: Current tilt angle
  - `crashDetected`: Crash detection flag

### 3. Panic Switch (panic_switch.h/cpp)
- Debounced button input handling
- Functions:
  - `isPanicPressed()`: Returns current button state
  - Pattern detection for valid triggers

### 4. Alert System (alert.h/cpp)
- Siren modes:
  - OFF
  - STEADY
  - FAST_BLINK
  - SLOW_BLINK
  - BURST
- LED status control
- Functions:
  - `setSirenMode()`: Controls siren pattern
  - `updateSiren()`: Updates siren state
  - `setLEDStatus()`: Controls LED indicators

### 5. Priority Management (priority.h/cpp)
- Panic source tracking
- Escalation logic
- Panic states:
  - NONE
  - TRIGGERED
  - ESCALATED
- Source priorities:
  - IMU (highest)
  - Button
  - LoRaWAN (lowest)

### 6. LoRaWAN Communication (lorawan.h/cpp)
- Functions:
  - `setupLoRa()`: Initializes LoRa modem
  - `sendLoRaPayload()`: Sends status updates
  - `processDownlink()`: Handles incoming commands
- Payload structure:
  - Timestamp
  - Device ID
  - SOS flag and reason
  - Tilt and vibration data
  - Battery percentage
  - Retry count

### 7. Battery Management (battery.h/cpp)
- Functions:
  - `readBatteryVoltage()`: Reads current voltage
  - `getBatteryPercentage()`: Calculates battery level
  - `isLowBattery()`: Checks for low battery condition

## Operation Flow

### Initialization
1. Configure pins and peripherals
2. Initialize IMU sensor
3. Setup LoRaWAN connection
4. Initialize alert system

### Main Loop
1. Check panic sources:
   - Read panic button state
   - Check IMU readings
   - Process LoRaWAN downlinks

2. Update system state:
   - Process panic triggers
   - Handle escalation timing
   - Update alert system

3. Communicate status:
   - Send periodic LoRaWAN updates
   - Update LED indicators
   - Control siren patterns

### Panic State Management
1. **Trigger Sources**
   - Manual button press
   - IMU crash detection
   - Remote LoRaWAN command

2. **State Progression**
   - Normal → Triggered (initial panic)
   - Triggered → Escalated (after timeout)
   - Escalated → Normal (after resolution)

## Configuration Parameters

### Device Settings
```cpp
#define DEVICE_ID "DEV_001"
#define DEFAULT_SEND_INTERVAL_MS 30000
```

### LoRaWAN Credentials
```cpp
const char LORA_APP_EUI[] = "0000000000000000";
const char LORA_APP_KEY[] = "00000000000000000000000000000000";
const char LORA_DEV_EUI[] = "0000000000000000";
```

### Hardware Pins
```cpp
#define SIREN_PIN 2
#define LED_PIN 3
#define LED_STATUS_PIN 4
#define PANIC_BUTTON_PIN 5
#define BATTERY_MONITOR_PIN A1
```

## Notes for Deployment
1. Update LoRaWAN credentials before deployment
2. Calibrate IMU thresholds for specific terrain
3. Test battery monitoring with actual battery pack
4. Verify siren and LED visibility in deployment area
5. Configure send interval based on power requirements