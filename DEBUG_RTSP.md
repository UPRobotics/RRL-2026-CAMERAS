# Debug RTSP Webcam Server

This debug feature allows you to test the camera viewer application using your laptop's webcam as an RTSP source.

## Quick Start

### Step 1: Start the Webcam RTSP Server

Run the PowerShell script in a separate terminal:

```powershell
.\debug-webcam-rtsp.ps1
```

**Options:**
```powershell
# Custom resolution and framerate
.\debug-webcam-rtsp.ps1 -Width 1280 -Height 720 -FPS 30

# Use different webcam device
.\debug-webcam-rtsp.ps1 -DeviceIndex 1

# Custom RTSP port
.\debug-webcam-rtsp.ps1 -Port 8555
```

### Step 2: Run the Application

In another terminal, launch the application with debug flag for helpful instructions:

```powershell
.\build\RTSPCameraViewer.exe --debug
```

Or run normally:
```powershell
.\build\RTSPCameraViewer.exe
```

## Recommended Settings

### For 1080p @ 30fps (High Quality)
- **Resolution**: 1920x1080
- **Frame Rate**: 30 fps
- **Bitrate**: 4 Mbps
- **Codec**: H.264 (baseline profile)
- **Preset**: ultrafast
- **Tune**: zerolatency
- **GOP Size**: 30 (1 second)
- **B-Frames**: 0 (disabled for low latency)

### For 720p @ 30fps (Lower Bandwidth)
- **Resolution**: 1280x720
- **Frame Rate**: 30 fps
- **Bitrate**: 2 Mbps
- **Same codec settings as above**

## Configuration for Testing

Add this to your `config/config.ini` to test with the debug webcam:

```ini
[cameras]
camera1_url = rtsp://127.0.0.1:8554/webcam
camera2_url = rtsp://127.0.0.1:8554/webcam
camera3_url = rtsp://127.0.0.1:8554/webcam
camera4_url = rtsp://127.0.0.1:8554/webcam
```

## Troubleshooting

### Webcam Not Detected

List available devices:
```powershell
ffmpeg -list_devices true -f dshow -i dummy
```

### Resolution Not Supported

Try lower resolutions:
- 1280x720 @ 30fps
- 640x480 @ 30fps

### High Latency

The settings are optimized for low latency:
- No B-frames
- Small GOP size (1 second)
- Zero latency tune
- Ultra-fast preset
- Baseline profile

### Using External RTSP Server (Advanced)

For better RTSP support, download **MediaMTX**:
1. Download from: https://github.com/bluenviron/mediamtx/releases
2. Extract to: `.\tools\mediamtx\`
3. The script will automatically use it if detected

## Network Configuration

The debug server binds to `127.0.0.1` (localhost only) by default. To allow network access:

1. Change bind address to `0.0.0.0` in the script
2. Add firewall rule for port 8554
3. Use your machine's IP address: `rtsp://YOUR_IP:8554/webcam`

## FFmpeg Command Reference

Manual FFmpeg command for webcam streaming:

```bash
ffmpeg -f dshow -video_size 1920x1080 -framerate 30 \
  -i "video=@device_pv_0" \
  -c:v libx264 -preset ultrafast -tune zerolatency \
  -profile:v baseline -b:v 4M -maxrate 4M -bufsize 4M \
  -g 30 -bf 0 -flags low_delay -fflags nobuffer \
  -f rtsp rtsp://127.0.0.1:8554/webcam
```

## Performance Notes

- **CPU Usage**: Ultrafast preset minimizes encoding CPU usage (~10-20% on modern CPUs)
- **Latency**: Typically 100-300ms end-to-end
- **Bandwidth**: ~4 Mbps for 1080p, ~2 Mbps for 720p
- **RAM Usage**: Minimal (~50-100 MB)

## Integration with Main Application

When launched with `--debug`, the application shows helpful instructions:
```
[2026-01-08 04:00:00] === C++ RTSP Camera Viewer ===
[2026-01-08 04:00:00] Starting application...
[2026-01-08 04:00:00] ========================================
[2026-01-08 04:00:00] DEBUG MODE ENABLED
[2026-01-08 04:00:00] ========================================
[2026-01-08 04:00:00] To start webcam RTSP server, run in another terminal:
[2026-01-08 04:00:00]   .\debug-webcam-rtsp.ps1
[2026-01-08 04:00:00]
[2026-01-08 04:00:00] Then update config/config.ini with:
[2026-01-08 04:00:00]   camera1_url = rtsp://127.0.0.1:8554/webcam
[2026-01-08 04:00:00] ========================================
```

The `--debug` flag simply shows instructions. The RTSP server runs independently via the PowerShell script.
