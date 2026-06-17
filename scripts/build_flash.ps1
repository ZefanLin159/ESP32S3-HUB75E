# Build & flash Pixel Music Companion on ESP32-S3 via COM8
& "C:\Espressif\tools\Microsoft.v5.4.4.PowerShell_profile.ps1"
Set-Location "E:\thinking\ESP32S3-HUB75E"
Remove-Item Env:MSYSTEM -ErrorAction SilentlyContinue

$idfPy = "$env:IDF_PATH\tools\idf.py"
if (-not (Test-Path $idfPy)) {
    Write-Error "idf.py not found at $idfPy"
    exit 1
}

& python $idfPy -p COM8 build flash
