# Debug RTSP Webcam Server Launcher
# This script starts an RTSP server streaming your webcam for testing

param(
    [string]$DeviceName = "HP Wide Vision HD Camera",
    [int]$Width = 1280,
    [int]$Height = 720,
    [int]$FPS = 30,
    [int]$Port = 8554
)

Write-Host "=== Debug RTSP Webcam Server ===" -ForegroundColor Cyan
Write-Host ""

# Check if FFmpeg is available
$ffmpegPath = $null
$vcpkgFFmpeg = "C:\vcpkg\installed\x64-windows\tools\ffmpeg\ffmpeg.exe"

if (Test-Path $vcpkgFFmpeg) {
    $ffmpegPath = $vcpkgFFmpeg
    Write-Host "Using FFmpeg from vcpkg: $ffmpegPath" -ForegroundColor Green
} elseif (Get-Command ffmpeg -ErrorAction SilentlyContinue) {
    $ffmpegPath = (Get-Command ffmpeg).Source
    Write-Host "Using FFmpeg from PATH: $ffmpegPath" -ForegroundColor Green
} else {
    Write-Host "ERROR: FFmpeg not found!" -ForegroundColor Red
    Write-Host "Please install FFmpeg or ensure vcpkg FFmpeg is available" -ForegroundColor Yellow
    exit 1
}

# List available DirectShow devices
Write-Host ""
Write-Host "Detecting available webcams..." -ForegroundColor Yellow
& $ffmpegPath -list_devices true -f dshow -i dummy 2>&1 | Select-String -Pattern "DirectShow|video devices" -Context 0,10

Write-Host ""
Write-Host "Configuration:" -ForegroundColor Cyan
Write-Host "  Device Name: $DeviceName"
Write-Host "  Resolution: ${Width}x${Height}"
Write-Host "  Frame Rate: ${FPS} fps"
Write-Host "  RTSP Port: $Port"
Write-Host "  RTSP URL: rtsp://127.0.0.1:${Port}/webcam"
Write-Host ""

# Check for MediaMTX
$mediamtxPath = ".\tools\mediamtx\mediamtx.exe"
$mediamtxConfigPath = ".\tools\mediamtx\mediamtx.yml"
$mediamtxRunning = $false

if (Test-Path $mediamtxPath) {
    Write-Host "MediaMTX found. Starting RTSP server..." -ForegroundColor Green
    
    # Create simple config for MediaMTX
    $config = @"
# MediaMTX configuration for debug webcam streaming
rtspAddress: :${Port}
rtmpAddress: :1935
hlsAddress: :8888
webrtcAddress: :8889

paths:
  webcam:
    runOnReady: >
      $ffmpegPath
      -f dshow
      -video_size ${Width}x${Height}
      -framerate ${FPS}
      -i video="${DeviceName}"
      -c:v libx264
      -preset ultrafast
      -tune zerolatency
      -b:v 4M
      -maxrate 4M
      -bufsize 4M
      -g ${FPS}
      -bf 0
      -f rtsp
      rtsp://localhost:${Port}/webcam
    runOnReadyRestart: yes
"@
    
    $config | Out-File -FilePath $mediamtxConfigPath -Encoding UTF8
    
    Write-Host "Starting MediaMTX RTSP server..." -ForegroundColor Yellow
    Start-Process -FilePath $mediamtxPath -ArgumentList $mediamtxConfigPath -WindowStyle Hidden
    $mediamtxRunning = $true
    Start-Sleep -Seconds 2
    
} else {
    Write-Host "MediaMTX not found. Using FFmpeg direct streaming..." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "NOTE: For better RTSP support, download MediaMTX from:" -ForegroundColor Cyan
    Write-Host "  https://github.com/bluenviron/mediamtx/releases" -ForegroundColor Cyan
    Write-Host "  Extract to: .\tools\mediamtx\" -ForegroundColor Cyan
    Write-Host ""
}

# Build FFmpeg command for webcam streaming
Write-Host "Starting webcam capture and RTSP stream..." -ForegroundColor Green
Write-Host ""

$ffmpegArgs = @(
    "-loglevel", "error",
    "-f", "dshow",
    "-video_size", "${Width}x${Height}",
    "-framerate", "$FPS",
    "-i", "video=${DeviceName}",
    "-c:v", "libx264",
    "-preset", "ultrafast",
    "-tune", "zerolatency",
    "-b:v", "4M",
    "-maxrate", "4M",
    "-bufsize", "4M",
    "-g", "$FPS",
    "-bf", "0",
    "-flags", "low_delay",
    "-fflags", "nobuffer",
    "-strict", "experimental",
    "-f", "rtsp",
    "rtsp://127.0.0.1:${Port}/webcam"
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "RTSP stream available at:" -ForegroundColor Green
Write-Host "  rtsp://127.0.0.1:${Port}/webcam" -ForegroundColor White
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

try {
    & $ffmpegPath @ffmpegArgs
} catch {
    Write-Host "Error occurred: $_" -ForegroundColor Red
} finally {
    if ($mediamtxRunning) {
        Write-Host ""
        Write-Host "Stopping MediaMTX..." -ForegroundColor Yellow
        Get-Process mediamtx -ErrorAction SilentlyContinue | Stop-Process -Force
    }
    Write-Host "Stream stopped." -ForegroundColor Red
}
