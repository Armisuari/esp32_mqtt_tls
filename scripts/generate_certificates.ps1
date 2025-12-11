# Generate MQTT TLS Certificates for ESP32
# This script creates a CA certificate, server certificate, and private key

if (-not (Test-Path "..\\certificates")) { New-Item -ItemType Directory -Path "..\\certificates" }; Set-Location "..\\certificates"

# Check if OpenSSL is available
$openssl = Get-Command openssl -ErrorAction SilentlyContinue
if (-not $openssl) {
    Write-Host "OpenSSL not found. Trying to use Git's OpenSSL..."
    $gitPath = Get-Command git -ErrorAction SilentlyContinue
    if ($gitPath) {
        $opensslPath = Join-Path (Split-Path (Split-Path $gitPath.Source)) "usr\bin\openssl.exe"
        if (Test-Path $opensslPath) {
            Set-Alias -Name openssl -Value $opensslPath
            Write-Host "Using Git's OpenSSL: $opensslPath"
        } else {
            Write-Error "OpenSSL not found. Please install OpenSSL or Git for Windows."
            exit 1
        }
    } else {
        Write-Error "Neither OpenSSL nor Git found. Please install one of them."
        exit 1
    }
}

Write-Host "🔐 Generating MQTT TLS certificates..."

# 1. Generate CA private key
Write-Host "1. Creating CA private key..."
openssl genrsa -out ca.key 2048

# 2. Generate CA certificate
Write-Host "2. Creating CA certificate..."
$caSubject = "/C=US/ST=State/L=City/O=MosquittoCA/CN=MosquittoCA"
openssl req -new -x509 -key ca.key -out ca.crt -days 3650 -subj $caSubject

# 3. Generate server private key
Write-Host "3. Creating server private key..."
openssl genrsa -out server.key 2048

# 4. Generate server certificate signing request
Write-Host "4. Creating server certificate request..."
$serverSubject = "/C=US/ST=State/L=City/O=MosquittoServer/CN=192.168.19.50"
openssl req -new -key server.key -out server.csr -subj $serverSubject

# 5. Sign server certificate with CA
Write-Host "5. Signing server certificate..."
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365

# Clean up CSR file
Remove-Item server.csr -ErrorAction SilentlyContinue

Write-Host "✅ Certificate generation completed!"
Write-Host ""
Write-Host "Generated files:"
Write-Host "  - ca.crt      (Certificate Authority - for ESP32)"
Write-Host "  - ca.key      (CA private key)"
Write-Host "  - server.crt  (Server certificate - for Mosquitto)"
Write-Host "  - server.key  (Server private key - for Mosquitto)"

# Show certificate details
Write-Host ""
Write-Host "📋 CA Certificate details:"
openssl x509 -in ca.crt -text -noout | Select-String "Subject:|Not Before:|Not After:"

Write-Host ""
Write-Host "📋 Server Certificate details:"
openssl x509 -in server.crt -text -noout | Select-String "Subject:|Not Before:|Not After:"

Set-Location ".."