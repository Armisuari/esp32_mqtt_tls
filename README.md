# ESP32-S3 MQTT TLS Example

This is an ESP32-S3 implementation of MQTT over TLS/SSL connecting to test.mosquitto.org public broker.

## Prerequisites

1. **ESP-IDF v5.3.4** installed and configured
2. **ESP32-S3** development board
3. **WiFi network** access

## Hardware Requirements

- ESP32-S3 DevKit or compatible board
- USB-C cable for programming and power
- WiFi network with internet access

## Configuration

1. **WiFi Settings**: Update `sdkconfig.defaults`:
   ```
   CONFIG_EXAMPLE_WIFI_SSID="YOUR_WIFI_SSID"
   CONFIG_EXAMPLE_WIFI_PASSWORD="YOUR_WIFI_PASSWORD"
   ```

2. **MQTT Broker**: Uses `test.mosquitto.org:8883` (TLS/SSL port)

## Features

- ✅ **Secure MQTT over TLS/SSL** 
- ✅ **ESP32-S3 optimized** with PSRAM support
- ✅ **Auto-reconnection** and error handling
- ✅ **Temperature sensor simulation**
- ✅ **Command echo functionality**
- ✅ **JSON formatted messages**

## Topics

- **Publish**: `esp32s3/sensor/temperature` - JSON sensor data
- **Subscribe**: `esp32s3/commands` - Commands to echo

## Building and Flashing

1. **Set ESP-IDF target**:
   ```bash
   cd esp32_mqtt_tls
   idf.py set-target esp32s3
   ```

2. **Configure project** (update WiFi credentials):
   ```bash
   idf.py menuconfig
   ```
   Navigate to: `Example Configuration` → Set WiFi SSID and Password

3. **Build project**:
   ```bash
   idf.py build
   ```

4. **Flash to ESP32-S3**:
   ```bash
   idf.py -p COMx flash monitor
   ```

## Usage

1. **Monitor output**: Watch serial console for connection status and published data
2. **Send commands**: Use any MQTT client to publish to `esp32s3/commands`
3. **View responses**: Check `esp32s3/sensor/temperature` topic for echoed commands

## Example MQTT Client Commands

Using `mosquitto_pub`:
```bash
# Send a command
mosquitto_pub -h test.mosquitto.org -p 8883 --cafile ca.crt -t "esp32s3/commands" -m "Hello ESP32-S3"

# Monitor responses  
mosquitto_sub -h test.mosquitto.org -p 8883 --cafile ca.crt -t "esp32s3/sensor/temperature"
```

## Expected Output

```
I (1234) MQTT_TLS_EXAMPLE: ESP32-S3 MQTT TLS Example Starting...
I (1240) MQTT_TLS_EXAMPLE: Chip: ESP32-S3 with 2 CPU core(s), WiFi
I (5678) MQTT_TLS_EXAMPLE: WiFi connected successfully
I (5890) MQTT_TLS_EXAMPLE: MQTT client started with ID: ESP32S3_AB1234
I (6123) MQTT_TLS_EXAMPLE: MQTT_EVENT_CONNECTED
I (6456) MQTT_TLS_EXAMPLE: Published sensor data: {"device":"ESP32-S3","sensor":"temperature","value":25.67,"unit":"C","count":1}
```

## Troubleshooting

- **WiFi connection fails**: Check SSID/password in `sdkconfig`
- **TLS connection fails**: Verify internet connection and certificate
- **Build errors**: Ensure ESP-IDF v5.3.4 is properly installed
- **Flash errors**: Check USB-C cable and ESP32-S3 board connection