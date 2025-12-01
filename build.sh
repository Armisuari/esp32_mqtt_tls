#!/bin/bash

# ESP32-S3 MQTT TLS Example - Quick Build Script
# Make sure ESP-IDF is installed and sourced

echo "ESP32-S3 MQTT TLS Example - Build Script"
echo "========================================"

# Check if ESP-IDF is sourced
if [ -z "$IDF_PATH" ]; then
    echo "Error: ESP-IDF environment not found!"
    echo "Please source the ESP-IDF environment:"
    echo "source \$HOME/esp/esp-idf/export.sh"
    exit 1
fi

echo "ESP-IDF Path: $IDF_PATH"
echo "ESP-IDF Version: $(idf.py --version)"

# Set target
echo "Setting target to ESP32-S3..."
idf.py set-target esp32s3

# Show configuration reminder
echo ""
echo "IMPORTANT: Configure WiFi settings!"
echo "Run: idf.py menuconfig"
echo "Navigate to: Example Configuration → Set WiFi SSID and Password"
echo ""

# Build the project
echo "Building project..."
idf.py build

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "Next steps:"
    echo "1. Connect your ESP32-S3 board"
    echo "2. Run: idf.py -p /dev/ttyUSB0 flash monitor"
    echo "3. Replace /dev/ttyUSB0 with your actual port"
    echo ""
    echo "Windows users use: idf.py -p COMx flash monitor"
else
    echo "❌ Build failed!"
    echo "Check the error messages above"
fi