# Check MQTT Broker Status Script
Write-Host "Checking MQTT Broker Status..." -ForegroundColor Cyan

# Check if Mosquitto process is running
$process = Get-Process mosquitto -ErrorAction SilentlyContinue
if ($process) {
    Write-Host "Process: Running (PID: $($process.Id))" -ForegroundColor Green
} else {
    Write-Host "Process: Not running" -ForegroundColor Red
}

# Check if MQTT ports are listening
$ports = netstat -an | Select-String ":1883|:8883"
if ($ports) {
    Write-Host "Ports: Listening" -ForegroundColor Green
    $ports | ForEach-Object { Write-Host "  $($_.Line.Trim())" -ForegroundColor Gray }
} else {
    Write-Host "Ports: Not listening" -ForegroundColor Red
}

# Check Windows service status
$service = Get-Service mosquitto -ErrorAction SilentlyContinue
if ($service) {
    if ($service.Status -eq "Running") {
        Write-Host "Service: Running" -ForegroundColor Green
    } else {
        Write-Host "Service: $($service.Status)" -ForegroundColor Yellow
    }
} else {
    Write-Host "Service: Not installed" -ForegroundColor Blue
}

Write-Host ""

# Auto-start logic
if (-not $process) {
    Write-Host "Starting broker automatically..." -ForegroundColor Yellow
    
    $configFile = "..\\mosquitto.conf"
    if (Test-Path $configFile) {
        try {
            $brokerPath = "C:\Program Files\mosquitto\mosquitto.exe"
            Start-Process -FilePath $brokerPath -ArgumentList "-c", $configFile, "-v" -WindowStyle Hidden
            
            Start-Sleep 2
            
            $newProcess = Get-Process mosquitto -ErrorAction SilentlyContinue
            if ($newProcess) {
                Write-Host "Broker started successfully!" -ForegroundColor Green
                Write-Host ""
                Write-Host "Available endpoints:" -ForegroundColor Cyan
                Write-Host "  mqtt://localhost:1883 (Plain)" -ForegroundColor White
                Write-Host "  mqtts://localhost:8883 (TLS)" -ForegroundColor White
            } else {
                Write-Host "Failed to start broker" -ForegroundColor Red
                Write-Host ""
                Write-Host "Manual start command:" -ForegroundColor Blue
                Write-Host "& `"C:\Program Files\mosquitto\mosquitto.exe`" -c `"mosquitto.conf`" -v" -ForegroundColor White
            }
        } catch {
            Write-Host "Error starting broker: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "Config file not found: $configFile" -ForegroundColor Red
    }
} else {
    Write-Host "Broker is already running!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Available endpoints:" -ForegroundColor Cyan
    Write-Host "  mqtt://localhost:1883 (Plain)" -ForegroundColor White
    Write-Host "  mqtts://localhost:8883 (TLS)" -ForegroundColor White
}