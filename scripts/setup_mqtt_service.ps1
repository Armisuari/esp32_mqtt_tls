# ESP32-S3 MQTT TLS Broker Setup Script (PowerShell)
# This script sets up Mosquitto MQTT broker with TLS support

Write-Host "ESP32-S3 MQTT TLS Broker Setup (PowerShell)" -ForegroundColor Yellow
Write-Host "=============================================" -ForegroundColor Yellow

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")

if (-not $isAdmin) {
    Write-Host "Error: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Please run PowerShell as Administrator and try again." -ForegroundColor Red
    Write-Host ""
    Write-Host "Instructions:" -ForegroundColor Yellow
    Write-Host "1. Press Win+X and select 'Windows PowerShell (Admin)'" -ForegroundColor Cyan
    Write-Host "2. Navigate to: cd 'C:\Users\Noovo-Arief\firmware\esp32_mqtt_tls'" -ForegroundColor Cyan
    Write-Host "3. Run: .\setup_mqtt_broker.ps1" -ForegroundColor Cyan
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "✅ Running with administrator privileges" -ForegroundColor Green

# Find Mosquitto installation
Write-Host ""
Write-Host "Searching for Mosquitto installation..." -ForegroundColor Yellow

$mosquittoPath = $null
$possiblePaths = @(
    "C:\Program Files\mosquitto",
    "C:\Program Files (x86)\mosquitto", 
    "C:\mosquitto"
)

foreach ($path in $possiblePaths) {
    if (Test-Path "$path\mosquitto.exe") {
        $mosquittoPath = $path
        Write-Host "✅ Found at: $path" -ForegroundColor Green
        break
    }
}

if (-not $mosquittoPath) {
    Write-Host "❌ Mosquitto not found!" -ForegroundColor Red
    Write-Host "Please install Mosquitto from: https://mosquitto.org/download/" -ForegroundColor Yellow
    Write-Host "Looking in:" -ForegroundColor Yellow
    foreach ($path in $possiblePaths) {
        Write-Host "- $path" -ForegroundColor Cyan
    }
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Using Mosquitto at: $mosquittoPath" -ForegroundColor Cyan

# Create directories
Write-Host ""
Write-Host "Creating necessary directories..." -ForegroundColor Yellow

$directories = @("config", "certs", "logs", "data")
foreach ($dir in $directories) {
    $fullPath = Join-Path $mosquittoPath $dir
    if (-not (Test-Path $fullPath)) {
        Write-Host "Creating $dir directory..." -ForegroundColor Gray
        New-Item -ItemType Directory -Path $fullPath -Force | Out-Null
        Write-Host "✅ $dir directory created" -ForegroundColor Green
    } else {
        Write-Host "✅ $dir directory already exists" -ForegroundColor Green
    }
}

# Create certificate that matches ESP32 code
Write-Host ""
Write-Host "Creating TLS certificate matching ESP32 code..." -ForegroundColor Yellow

$caCert = @"
-----BEGIN CERTIFICATE-----
MIIDkzCCAnugAwIBAgIUK0361O+vXZyMt3BWW2nDlAtB6k4wDQYJKoZIhvcNAQEL
BQAwWTELMAkGA1UEBhMCVVMxDjAMBgNVBAgMBVN0YXRlMQ0wCwYDVQQHDARDaXR5
MRUwEwYDVQQKDAxPcmdhbml6YXRpb24xFDASBgNVBAMMC01vc3F1aXR0b0NBMB4X
DTI1MTIwMTE5MDcxMFoXDTM1MTEyOTE5MDcxMFowWTELMAkGA1UEBhMCVVMxDjAM
BgNVBAgMBVN0YXRlMQ0wCwYDVQQHDARDaXR5MRUwEwYDVQQKDAxPcmdhbml6YXRp
b24xFDASBgNVBAMMC01vc3F1aXR0b0NBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A
MIIBCgKCAQEA6L1Mtrg5M/9IMVobmiRhFHLRKmxy3jPrVRoYjxfVEuucqT0dlGSz
LF433QUoJf3lwXvc3RKwAEfFeKUnF3tbQaYU6KOK16PHOJA9SGm0UniQX68K7P3z
M/e+SH9T241yz9bfuE3Kfcrg4b4q+uiZGT92oMC/rbLOBkXRdbwrQnwcMG6zKjcc
CBQJKGaMA9f/SbLciGLOLRYeTOjhqIHypD7sZNE+CT1TGQ9lPJ5RHean+sVMfZCD
gFI1pMMIUA0kl0xT1bWujkPi+M8O/uAH87hqHtvvpZZK7+H4t9ErDclQYUYY/Ej8
Bynh4aORtmxRkpA1UnpNO1iFVYjf1gPE1wIDAQABo1MwUTAdBgNVHQ4EFgQUHUp+
jLEBNris2KkR6ZWUUOi9BkIwHwYDVR0jBBgwFoAUHUp+jLEBNris2KkR6ZWUUOi9
BkIwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAhmcBNt3jHvaF
lmau9ywX+K0zjVDkMO/tTjUSYDcogxcfMlJiyzJ1ua8sU/aRjAXJUbIPK7Pv2nP3
FmPJssjMDFD9MImPwB05BmG93HWyAqXafPYQtvj8FIp9CGaErnvcDcMB0Ke7iT5s
71qRWcpoCTW+CC09fH6JsRx2O1sKG2BWPh1pX5M2yKCYWAZYBV7C5QlLUlDt82QL
6RHUsvVyyD0KyNGn1926I6bT3lf7EkmVZ1bePvm/xdeaBHIBX/K1IIPSFQZlQKOy
46lrRUxHopyjWuZPy1UP7cvTLsIialD0OIhS7n7O7/l8QItMQFiSuhQpSduXDJzR
1TG9bA1P0Q==
-----END CERTIFICATE-----
"@

$serverKey = @"
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDovUy2uDkz/0gx
WhueJGEUctEqbHLeM+tVGpiPF9US65yrPQ2UZLMsXjfdBSgl/eXBe9zdErAAR8V4
pScXe1tBphTo44rXo8E4kD1IabRSepBfrwrs/fMz975If1PbjXLP1t+4Tcp9yuDh
vir66JkZP3agwL+tss4GRdF1vCtCfBwwbrMqNxwIFAkoZowD1/9JstyIYs4tFh5M
6OGogykPuxk0T4JPVMZDyU8nlEd5qf6xUx9kIOAUjWkwwhQDSSXTFPVta6OQ+L4
zw7+4AfzuGoe2++llkrv4fi30SsNyVBhRhj8SPwHKephyO8tFh5M6OGogelAPuxk
0T4JPVMZAgMBAAECggEBAMxUqWwKfcfB5t0a2zJnSHF1BsGW5J+sK0ZaRHjI=
-----END PRIVATE KEY-----
"@

try {
    $caCert | Out-File -FilePath (Join-Path $mosquittoPath "certs\ca.crt") -Encoding utf8
    $caCert | Out-File -FilePath (Join-Path $mosquittoPath "certs\server.crt") -Encoding utf8
    $serverKey | Out-File -FilePath (Join-Path $mosquittoPath "certs\server.key") -Encoding utf8
    Write-Host "✅ Certificates created" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to create certificates: $($_.Exception.Message)" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Create configuration file
Write-Host ""
Write-Host "Creating Mosquitto configuration file..." -ForegroundColor Yellow

$configContent = @"
# Mosquitto MQTT Broker Configuration for ESP32-S3 TLS Example
# Generated automatically by setup script

# Basic settings
persistence true
persistence_location "$mosquittoPath\data\"
log_dest file "$mosquittoPath\logs\mosquitto.log"
log_type all

# Network settings
bind_address 0.0.0.0

# Standard MQTT port (non-encrypted)
port 1883

# MQTT over TLS/SSL port
listener 8883
protocol mqtt

# TLS settings for port 8883
cafile "$mosquittoPath\certs\ca.crt"
certfile "$mosquittoPath\certs\server.crt"
keyfile "$mosquittoPath\certs\server.key"
tls_version tlsv1.2
require_certificate false

# Security settings (allow anonymous for testing)
allow_anonymous true

# Max connections
max_connections 100

# Keepalive settings
max_keepalive 60

# Message size limits
message_size_limit 1048576
"@

try {
    $configContent | Out-File -FilePath (Join-Path $mosquittoPath "config\mosquitto.conf") -Encoding utf8
    Write-Host "✅ Configuration file created" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to create configuration file: $($_.Exception.Message)" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Stop existing service if running
Write-Host ""
Write-Host "Checking for existing Mosquitto service..." -ForegroundColor Yellow

$service = Get-Service -Name "Mosquitto" -ErrorAction SilentlyContinue
if ($service) {
    Write-Host "Found existing service, stopping and removing..." -ForegroundColor Gray
    try {
        Stop-Service -Name "Mosquitto" -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
        & sc.exe delete "Mosquitto" | Out-Null
        Start-Sleep -Seconds 2
        Write-Host "✅ Old service removed" -ForegroundColor Green
    } catch {
        Write-Host "⚠️ Warning: Could not remove old service completely" -ForegroundColor Yellow
    }
} else {
    Write-Host "✅ No existing service found" -ForegroundColor Green
}

# Install new service
Write-Host ""
Write-Host "Installing Mosquitto as Windows service..." -ForegroundColor Yellow
Write-Host "Service path: $mosquittoPath\mosquitto.exe" -ForegroundColor Gray
Write-Host "Config path: $mosquittoPath\config\mosquitto.conf" -ForegroundColor Gray

$binPath = "`"$mosquittoPath\mosquitto.exe`" -c `"$mosquittoPath\config\mosquitto.conf`" -v"
Write-Host "Full command: sc create Mosquitto binPath= `"$binPath`"" -ForegroundColor Gray

try {
    # Use Start-Process to run sc.exe with proper escaping
    $scArgs = @(
        "create",
        "Mosquitto",
        "binPath=",
        $binPath,
        "DisplayName=",
        "Mosquitto MQTT Broker",
        "start=",
        "auto"
    )
    
    $result = Start-Process -FilePath "sc.exe" -ArgumentList $scArgs -Wait -PassThru -NoNewWindow
    
    if ($result.ExitCode -eq 0) {
        Write-Host "✅ Service installed successfully!" -ForegroundColor Green
        
        # Configure service recovery
        Write-Host "Configuring service recovery options..." -ForegroundColor Gray
        & sc.exe failure "Mosquitto" reset= 0 actions= restart/5000/restart/5000/restart/5000 | Out-Null
        
        # Start the service
        Write-Host "Starting Mosquitto service..." -ForegroundColor Yellow
        
        try {
            Start-Service -Name "Mosquitto"
            Write-Host "✅ Service started successfully!" -ForegroundColor Green
            
            Write-Host ""
            Write-Host "==========================================" -ForegroundColor Green
            Write-Host "🎉 Setup Complete! 🎉" -ForegroundColor Green
            Write-Host "==========================================" -ForegroundColor Green
            Write-Host ""
            Write-Host "Broker is now running:" -ForegroundColor Yellow
            Write-Host "- MQTT (non-TLS): port 1883" -ForegroundColor Cyan
            Write-Host "- MQTTS (TLS): port 8883" -ForegroundColor Cyan
            Write-Host ""
            Write-Host "Your PC IP addresses:" -ForegroundColor Yellow
            Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.InterfaceAlias -notmatch "Loopback" } | ForEach-Object {
                Write-Host "  $($_.IPAddress)" -ForegroundColor Cyan
            }
            Write-Host ""
            Write-Host "Update your ESP32 code with your PC's IP:" -ForegroundColor Yellow
            Write-Host '#define MQTT_BROKER_URL "mqtts://YOUR_PC_IP:8883"' -ForegroundColor Cyan
            Write-Host ""
            Write-Host "Configuration: $mosquittoPath\config\mosquitto.conf" -ForegroundColor Gray
            Write-Host "Logs: $mosquittoPath\logs\mosquitto.log" -ForegroundColor Gray
            
        } catch {
            Write-Host "❌ Failed to start service: $($_.Exception.Message)" -ForegroundColor Red
            Write-Host "Check the logs at: $mosquittoPath\logs\mosquitto.log" -ForegroundColor Yellow
        }
        
    } else {
        Write-Host "❌ Failed to install service!" -ForegroundColor Red
        Write-Host "Exit code: $($result.ExitCode)" -ForegroundColor Red
        Write-Host "Trying alternative method..." -ForegroundColor Yellow
        
        # Alternative method using New-Service cmdlet
        try {
            $serviceName = "Mosquitto"
            $displayName = "Mosquitto MQTT Broker"
            $binaryPath = "`"$mosquittoPath\mosquitto.exe`" -c `"$mosquittoPath\config\mosquitto.conf`" -v"
            
            New-Service -Name $serviceName -DisplayName $displayName -BinaryPathName $binaryPath -StartupType Automatic
            Write-Host "✅ Service installed using alternative method!" -ForegroundColor Green
            
            # Start the service
            Start-Service -Name "Mosquitto"
            Write-Host "✅ Service started successfully!" -ForegroundColor Green
            
            Write-Host ""
            Write-Host "==========================================" -ForegroundColor Green
            Write-Host "🎉 Setup Complete! 🎉" -ForegroundColor Green
            Write-Host "==========================================" -ForegroundColor Green
            Write-Host ""
            Write-Host "Broker is now running:" -ForegroundColor Yellow
            Write-Host "- MQTT (non-TLS): port 1883" -ForegroundColor Cyan
            Write-Host "- MQTTS (TLS): port 8883" -ForegroundColor Cyan
            Write-Host ""
            Write-Host "Your PC IP addresses:" -ForegroundColor Yellow
            Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.InterfaceAlias -notmatch "Loopback" } | ForEach-Object {
                Write-Host "  $($_.IPAddress)" -ForegroundColor Cyan
            }
            Write-Host ""
            Write-Host "Update your ESP32 code with your PC's IP:" -ForegroundColor Yellow
            Write-Host '#define MQTT_BROKER_URL "mqtts://YOUR_PC_IP:8883"' -ForegroundColor Cyan
            Write-Host ""
            Write-Host "Configuration: $mosquittoPath\config\mosquitto.conf" -ForegroundColor Gray
            Write-Host "Logs: $mosquittoPath\logs\mosquitto.log" -ForegroundColor Gray
            
        } catch {
            Write-Host "❌ Failed to install service with alternative method: $($_.Exception.Message)" -ForegroundColor Red
        }
    }
    
} catch {
    Write-Host "❌ Failed to install service: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""
Read-Host "Press Enter to exit"