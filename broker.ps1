# MQTT Broker Manager
# Quick access script for common MQTT broker operations

param(
    [Parameter(Position=0)]
    [ValidateSet("start", "stop", "status", "certs", "help")]
    [string]$Action = "start"
)

function Show-Help {
    Write-Host "MQTT Broker Manager" -ForegroundColor Cyan
    Write-Host "==================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\broker.ps1 [action]" -ForegroundColor White
    Write-Host ""
    Write-Host "Actions:" -ForegroundColor Yellow
    Write-Host "  start   - Start/check MQTT broker (default)" -ForegroundColor White
    Write-Host "  stop    - Stop MQTT broker" -ForegroundColor White
    Write-Host "  status  - Show broker status only" -ForegroundColor White
    Write-Host "  certs   - Generate new certificates" -ForegroundColor White
    Write-Host "  help    - Show this help" -ForegroundColor White
}

switch ($Action) {
    "start" {
        & ".\scripts\mqtt_broker.ps1"
    }
    "stop" {
        $process = Get-Process mosquitto -ErrorAction SilentlyContinue
        if ($process) {
            Write-Host "Stopping MQTT broker..." -ForegroundColor Yellow
            Stop-Process -Name mosquitto -Force
            Write-Host "Broker stopped." -ForegroundColor Green
        } else {
            Write-Host "Broker is not running." -ForegroundColor Blue
        }
    }
    "status" {
        $process = Get-Process mosquitto -ErrorAction SilentlyContinue
        if ($process) {
            Write-Host "Broker is running (PID: $($process.Id))" -ForegroundColor Green
            $ports = netstat -an | Select-String ":1883|:8883"
            if ($ports) {
                Write-Host "Ports listening:" -ForegroundColor Green
                $ports | ForEach-Object { Write-Host "  $($_.Line.Trim())" -ForegroundColor Gray }
            }
        } else {
            Write-Host "Broker is not running" -ForegroundColor Red
        }
    }
    "certs" {
        & ".\scripts\generate_certificates.ps1"
    }
    "help" {
        Show-Help
    }
    default {
        Show-Help
    }
}