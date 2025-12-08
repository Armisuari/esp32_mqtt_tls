@echo off
REM ESP32-S3 MQTT TLS Example - Windows Build Script

echo ESP32-S3 MQTT TLS Example - Build Script
echo ========================================

REM Auto-setup ESP-IDF environment
echo Setting up ESP-IDF environment...

REM Check if ESP-IDF is installed in common locations
set IDF_FOUND=0
if exist "c:\Users\%USERNAME%\esp\v5.3.2\esp-idf\export.ps1" (
    set ESP_IDF_PATH=c:\Users\%USERNAME%\esp\v5.3.2\esp-idf
    set IDF_FOUND=1
) else if exist "c:\Espressif\esp-idf\export.ps1" (
    set ESP_IDF_PATH=c:\Espressif\esp-idf
    set IDF_FOUND=1
) else if exist "%USERPROFILE%\Desktop\esp-idf\export.ps1" (
    set ESP_IDF_PATH=%USERPROFILE%\Desktop\esp-idf
    set IDF_FOUND=1
)

if %IDF_FOUND%==0 (
    echo Error: ESP-IDF installation not found!
    echo Please install ESP-IDF to one of these locations:
    echo - c:\Users\%USERNAME%\esp\v5.3.2\esp-idf\
    echo - c:\Espressif\esp-idf\
    echo - %USERPROFILE%\Desktop\esp-idf\
    echo Download from: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
    pause
    exit /b 1
)

REM Setup ESP-IDF environment using PowerShell
echo Found ESP-IDF at: %ESP_IDF_PATH%
echo Running ESP-IDF export script...
powershell -Command "& '%ESP_IDF_PATH%\export.ps1'; if ($LASTEXITCODE -eq 0) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to setup ESP-IDF environment!
    pause
    exit /b 1
)

REM Add Python environment to PATH for this session
if exist "c:\Users\%USERNAME%\.espressif\python_env\idf5.3_py3.11_env\Scripts" (
    set PATH=c:\Users\%USERNAME%\.espressif\python_env\idf5.3_py3.11_env\Scripts;%PATH%
)

REM Verify ESP-IDF is now available
where idf.py >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: ESP-IDF tools still not found after setup!
    echo Please check your ESP-IDF installation.
    pause
    exit /b 1
)

echo ✅ ESP-IDF environment ready!
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