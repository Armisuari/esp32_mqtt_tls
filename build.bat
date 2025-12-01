@echo off
REM ESP32-S3 MQTT TLS Example - Windows Build Script

echo ESP32-S3 MQTT TLS Example - Build Script
echo ========================================

REM Check if ESP-IDF is available
where idf.py >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: ESP-IDF not found in PATH!
    echo Please install ESP-IDF v5.3.4 and run install.bat/export.bat
    echo Download from: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
    pause
    exit /b 1
)

echo ESP-IDF found in system PATH
idf.py --version

REM Set target to ESP32-S3
echo.
echo Setting target to ESP32-S3...
idf.py set-target esp32s3

REM Configuration reminder
echo.
echo IMPORTANT: Configure WiFi settings!
echo Run: idf.py menuconfig
echo Navigate to: Example Configuration -^> Set WiFi SSID and Password
echo.

REM Build project
echo Building project...
idf.py build

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Build successful!
    echo.
    echo Next steps:
    echo 1. Connect your ESP32-S3 board
    echo 2. Run: idf.py -p COMx flash monitor
    echo 3. Replace COMx with your actual COM port
    echo.
) else (
    echo ❌ Build failed!
    echo Check the error messages above
)

pause