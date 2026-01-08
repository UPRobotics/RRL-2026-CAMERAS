# Debug Mode Guide

## Overview
Debug mode automatically launches an RTSP server streaming your webcam for testing the camera viewer application without requiring actual RTSP camera hardware.

**All processes run headless** - no extra console windows are opened. RTSP server output is redirected to the application's console (press `C` to view).

## Quick Start

### 1. Launch Application in Debug Mode
```powershell
.\build\RTSPCameraViewer.exe --debug
```

This will:
- Automatically launch the debug RTSP server (headless, no console windows)
- Start streaming your webcam to `rtsp://127.0.0.1:8554/webcam`
- Load debug settings from `config/settings.json`
- Redirect RTSP server logs to the application console (press `C` to view)

### 2. Start Camera Feed
In the application window:
1. Click the **"Start Cameras"** button in the toolbar
2. The application will create virtual camera slots based on the `virtual_camera_count` setting
3. Camera 1 will be configured to use the debug RTSP stream

## Configuration

### Debug Settings (config/settings.json)
```json
{
  "debug": {
    "rtsp_url": "rtsp://127.0.0.1:8554/webcam",
    "virtual_camera_count": 1
  }
}
```

- **rtsp_url**: The RTSP stream URL from the debug server
- **virtual_camera_count**: Number of virtual cameras to create (1-16)
  - Use higher numbers to test grid layouts
  - All cameras will display the same stream (for now)

## Viewing RTSP Server Logs

The RTSP server runs headless (no console window). To view logs:

1. Press `C` in the application to open the console window
2. RTSP server messages are prefixed with `[RTSP Server]`
3. Only important messages are logged (errors, status updates)
4. FFmpeg verbose output is suppressed (only errors shown)

## Debug RTSP Server

### Webcam Configuration
The debug server script (`debug-webcam-rtsp.ps1`) can be customized:

```powershell
# Use different webcam
.\debug-webcam-rtsp.ps1 -DeviceName "OMEN Cam Voice"

# Change resolution
.\debug-webcam-rtsp.ps1 -Width 640 -Height 480

# Change frame rate
.\debug-webcam-rtsp.ps1 -FPS 15

# Change RTSP port
.\debug-webcam-rtsp.ps1 -Port 9000
```

### Available Webcams
To see available webcams:
```powershell
ffmpeg -list_devices true -f dshow -i dummy 2>&1 | Select-String "video"
```

## Testing Multiple Camera Layouts

### Test 2x2 Grid
1. Update `config/settings.json`:
   ```json
   "virtual_camera_count": 4
   ```
2. Restart application with `--debug`
3. Click "Start Cameras"
4. Press `2` key to switch to 2x2 grid view

### Test NxN Grid
1. Set `virtual_camera_count` to 6, 9, or 16
2. Press `3` key to switch to NxN grid view

### Test Fullscreen View
1. Press `1` key for fullscreen view of selected camera
2. Use arrow keys (Left/Right) to switch between cameras

## Cleanup
When you close the application:
- The RTSP server process tree is automatically terminated via Windows Job Object
- All child processes (MediaMTX, FFmpeg, PowerShell) are killed together
- Redundant `taskkill` fallback ensures no orphaned processes
- Works for all exit methods: Q key, Escape key, window X button, or force kill

**Process Hierarchy:**
```
RTSPCameraViewer.exe
 └─ Job Object (JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE)
     ├─ powershell.exe
     ├─ mediamtx.exe
     └─ ffmpeg.exe
```
Closing the job object automatically terminates the entire process tree.

## Manual RTSP Server Control

### Start Server Manually
```powershell
.\debug-webcam-rtsp.ps1
```

### Stop Server Manually
If you started the server manually:
```powershell
Get-Process ffmpeg, mediamtx -ErrorAction SilentlyContinue | Stop-Process -Force
```

**Note**: When launched via debug mode, the server runs headless and is automatically managed by the application.

### Test Stream with VLC
```
Media -> Open Network Stream
rtsp://127.0.0.1:8554/webcam
```

### Test Stream with FFplay
```powershell
ffplay rtsp://127.0.0.1:8554/webcam
```

## Troubleshooting

### No Console Output Visible
- Press `C` key in the application to toggle the console window
- RTSP server logs are prefixed with `[RTSP Server]`
- FFmpeg only logs errors (verbose output is suppressed)

### Server Fails to Start
- **Check if webcam is in use**: Close other applications using the webcam
- **Check resolution**: Your webcam may not support 1280x720
  - List supported formats: `ffmpeg -f dshow -list_options true -i video="HP Wide Vision HD Camera"`
  - Update script with supported resolution

### Port Already in Use
```powershell
# Use different port
.\debug-webcam-rtsp.ps1 -Port 9554
```
Then update `config/settings.json`:
```json
"rtsp_url": "rtsp://127.0.0.1:9554/webcam"
```

### Application Shows "No cameras active"
1. Check that you clicked "Start Cameras" button
2. Verify debug mode is enabled with `--debug` flag
3. Check `virtual_camera_count` in settings.json is > 0

## Next Steps

Once RTSP stream decoding is implemented, the cameras will display actual video feeds. For now, the system creates camera slots and logs the RTSP URLs that would be connected.

## Files Modified
- `src/main.cpp` - Launches RTSP server in debug mode
- `src/main_window.cpp` - Creates virtual cameras from settings
- `src/settings_manager.cpp` - Loads/saves debug settings
- `config/settings.json` - Debug configuration
- `debug-webcam-rtsp.ps1` - RTSP server launcher script
