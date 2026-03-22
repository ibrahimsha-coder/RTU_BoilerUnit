Remote Telemetry Unit (RTU) – ESP32-S3 (ESP-IDF)
🚀 Overview

This project demonstrates a Remote Telemetry Unit (RTU) built using ESP32-S3 and ESP-IDF for monitoring and controlling a boiler system.

The system reads environmental parameters such as temperature, pressure, and water level, performs control logic for actuators (pump & heater), stores data locally during network failures, and transmits data to the cloud.

🎯 Key Features
✅ Real-time sensor monitoring
✅ Boiler control logic (Pump & Heater automation)
✅ Cloud integration (ThingSpeak)
✅ Remote command handling (TalkBack)
✅ Offline data storage using Flash (Circular Buffer)
✅ Automatic data sync after reconnection
✅ Layered firmware architecture
✅ Fault-tolerant and scalable design
⚙️ System Architecture


🚧 Limitations / Improvements
TalkBack requires manual command creation
→ Can be replaced with custom backend (MQTT/REST API)
Sensor values are currently simulated
→ Real sensors will provide accurate calibrated data
WiFi reconnection requires manual restart
→ Can be improved with auto-reconnect logic
Flash handling is inside RTU process
→ Should be moved to MCAL/Service layer
RTU state machine can be further refactored for readability
🛠️ Setup Instructions
🔧 Prerequisites
ESP-IDF installed
ESP32-S3 board
ThingSpeak account
