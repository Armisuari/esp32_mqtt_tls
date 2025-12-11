# ESP32-S3 MQTT TLS Client

A secure MQTT client implementation for ESP32-S3 with TLS encryption, demonstrating secure IoT communication between ESP32 and MQTT broker.

## Features

- 🔐 **TLS/SSL Encryption** - Secure MQTT communication over TLS v1.2
- 📱 **ESP32-S3 Compatible** - Optimized for ESP32-S3 with ESP-IDF v5.3.x
- 🏗️ **Automated Build** - Windows build script with ESP-IDF environment setup
- 🛡️ **Certificate Management** - Self-signed certificate generation and management
- 🖥️ **Mosquitto Integration** - Complete broker setup for Windows testing

## Quick Start

### Prerequisites
- ESP-IDF v5.3.x installed
- Mosquitto MQTT broker
- OpenSSL for certificate generation
- ESP32-S3 development board

### Setup

1. **Generate TLS Certificates**:
   ```powershell
   .\scripts\generate_certificates.ps1
   ```

2. **Configure WiFi Settings**:
   Edit `main/mqtt_tls_client.c` and update:
   ```c
   #define WIFI_SSID "Your_WiFi_SSID"
   #define WIFI_PASS "Your_WiFi_Password"
   ```

3. **Update Broker IP**:
   Update the broker IP in `main/mqtt_tls_client.c`:
   ```c
   #define MQTT_BROKER_URL "mqtts://YOUR_PC_IP:8883"
   ```

4. **Start MQTT Broker**:
   ```powershell
   .\broker.ps1 start
   ```

5. **Build and Flash ESP32**:
   ```powershell
   .\build.bat
   ```

## Quick Commands

```powershell
.\broker.ps1 start    # Start broker (auto-detects if already running)
.\broker.ps1 stop     # Stop broker
.\broker.ps1 status   # Check status only
.\broker.ps1 certs    # Generate new certificates
.\broker.ps1 help     # Show help
```

## Project Structure

```
esp32_mqtt_tls/
├── main/
│   ├── mqtt_tls_client.c      # Main ESP32 application
│   └── CMakeLists.txt         # Component build configuration
├── certificates/              # TLS certificates
│   ├── ca.crt                # Certificate Authority
│   ├── server.crt            # Server certificate
│   └── server.key            # Server private key
├── scripts/                   # PowerShell automation scripts
│   ├── mqtt_broker.ps1       # Auto-start MQTT broker
│   ├── generate_certificates.ps1 # Certificate generation
│   └── setup_mqtt_service.ps1 # Windows service setup
├── mosquitto.conf            # Mosquitto broker configuration
├── broker.ps1               # Main broker management script
├── build.bat                # Windows build script
├── build.sh                 # Linux/macOS build script
└── README.md                # This file
```

## Configuration

### MQTT Settings
- **Secure Port**: 8883 (MQTTS)
- **Plain Port**: 1883 (MQTT)
- **Topic**: `test/esp32`
- **QoS**: 1
- **Keep Alive**: 120 seconds

### Security
- **TLS Version**: 1.2
- **Certificate Verification**: Server certificate validation
- **Client Authentication**: Optional (disabled by default)

## Monitoring

The ESP32 will:
- Connect to WiFi network
- Establish secure MQTT connection
- Subscribe to `test/esp32` topic
- Publish periodic messages every 10 seconds
- Log all activities with timestamps

## Troubleshooting

### Connection Issues
1. Verify WiFi credentials
2. Check broker IP address
3. Ensure certificates are properly generated
4. Confirm firewall settings allow MQTT ports

### Certificate Errors
1. Regenerate certificates: `.\generate_certificates.ps1`
2. Update CA certificate in ESP32 code
3. Restart broker with new certificates

## License

This project is provided as-is for educational and development purposes.