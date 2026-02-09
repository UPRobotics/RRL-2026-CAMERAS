# RTSP Camera Viewer

Ultra-low-latency multi-camera RTSP viewer for competition monitoring. Built in C++20 with SDL2 and FFmpeg; supports hardware-accelerated decoding when available.

## Features
- Multi-camera viewing in fullscreen, 2x2, or auto grid layouts
- Low-latency playback with optional VAAPI/VDPAU/NVDEC acceleration
- Per-camera high/low quality toggle and rotation support
- Keyboard-driven UI with toolbar and in-app console for logs
- Stats bar showing CPU, GPU, RAM, and camera latency
- Persistent window, console, and keybinding settings via config

## Requirements
- Ubuntu 22.04 (tested)
- CMake 3.25+ and a C++20 compiler (GCC 11+)
- FFmpeg, SDL2, SDL2_ttf, spdlog, nlohmann-json, concurrentqueue

Install build dependencies:
```
sudo apt update && sudo apt install -y \
  cmake g++ make pkg-config \
  libsdl2-dev libsdl2-ttf-dev \
  libavcodec-dev libavformat-dev libswscale-dev libswresample-dev \
  libspdlog-dev nlohmann-json3-dev libconcurrentqueue-dev
```

## Build
```
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```
The binary is build/RTSPCameraViewer.

## Run
From the repository root after building:
```
./build/RTSPCameraViewer
```
Start cameras from the toolbar. Use arrow keys to cycle cameras in fullscreen mode.

## Configure cameras and behavior
Primary settings live in [config/settings.json](config/settings.json). Edit camera URLs, enable or disable feeds, adjust streaming timeouts, and set display/keybinding preferences.

Example excerpt:
```json
{
  "cameras": [
    {
      "id": "camera1",
      "name": "Camera 1",
      "enabled": true,
      "url_highres": "rtsp://192.168.0.200:554/stream1",
      "url_lowres": "rtsp://192.168.0.200:554/stream2",
      "rotation_deg": 0
    }
  ],
  "display": { "fullscreen_on_startup": false, "remember_window_position": true },
  "streaming": { "frame_buffer_size": 1, "connection_timeout_ms": 10000 }
}
```
A legacy [config/config.ini](config/config.ini) is included for compatibility, but the C++ viewer reads settings.json.

## Controls (defaults)
| Key | Action |
| --- | --- |
| Q / Esc | Quit |
| C | Toggle console |
| F | Toggle fullscreen |
| 1 | Fullscreen single camera |
| 2 | 2x2 grid |
| 3 | Auto grid (NxN) |
| Left / Right | Switch camera when fullscreen |

## Repository layout
```
config/        Runtime settings (JSON, INI)
docs/          Build, architecture, development notes
include/       Headers
src/           Source files
tools/         Utilities (e.g., mediamtx)
```

## Troubleshooting
- Build fails: verify dependency installation and rerun cmake from a clean build directory.
- GPU usage shows 0: ensure an NVIDIA GPU with nvidia-smi available; otherwise GPU stats will remain 0 and CPU decoding is used.
- High CPU usage: build in Release and keep frame_buffer_size at 1 for low latency.

## Documentation
More detail in [docs/BUILDING.md](docs/BUILDING.md), [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md), and [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md).

## License
MIT. See [LICENSE](LICENSE).