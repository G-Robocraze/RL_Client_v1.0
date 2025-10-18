# SOS Client System Flow Chart

> **Viewing Instructions:**
> - Install the "Markdown Preview Enhanced" extension in VS Code
> - Press `Ctrl+K V` to open preview
> - Or use `Ctrl+Shift+V` for side-by-side preview
> - The Mermaid diagram below will automatically render in the preview

```mermaid
flowchart TD
    subgraph Init["Initialization"]
        A[Setup Pins] --> B[Initialize Serial]
        B --> C[Setup LoRa]
        C --> D[Setup IMU]
    end

    subgraph MainLoop["Main Loop"]
        E[Read Sensors] --> F{Check Panic Sources}
        F -->|Button| G[Process Button Panic]
        F -->|IMU| H[Process IMU Panic]
        F -->|LoRaWAN| I[Process Remote Panic]
        
        G --> J[Update Panic State]
        H --> J
        I --> J
        
        J --> K{Check State}
        K -->|Normal| L[Normal Operation]
        K -->|Triggered| M[Level 1 Alert]
        K -->|Escalated| N[Level 2 Alert]
        
        L --> O[Update Alerts]
        M --> O
        N --> O
        
        O --> P[Send LoRa Update]
        P --> Q[Delay]
        Q --> E
    end

    subgraph PanicHandling["Panic State Machine"]
        R[PANIC_NONE] -->|Trigger| S[PANIC_TRIGGERED]
        S -->|Timeout| T[PANIC_ESCALATED]
        T -->|Clear| R
        S -->|Clear| R
    end

    subgraph AlertSystem["Alert System"]
        U[Update Siren] --> V{Check Mode}
        V -->|Off| W[Siren Off]
        V -->|Slow| X[Slow Blink]
        V -->|Fast| Y[Fast Blink]
        V -->|Steady| Z[Steady On]
    end

    subgraph LoRaWAN["LoRaWAN Communication"]
        AA[Prepare Payload] --> AB[Add Status]
        AB --> AC[Add Sensor Data]
        AC --> AD[Add Battery Level]
        AD --> AE[Send Packet]
        AF[Receive Downlink] --> AG[Process Command]
    end

    Init --> MainLoop
    MainLoop --> PanicHandling
    MainLoop --> AlertSystem
    MainLoop --> LoRaWAN
```