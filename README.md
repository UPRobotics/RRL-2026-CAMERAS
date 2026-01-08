# RRL-2026-CAMERAS

C++ Multi-Camera RTSP Viewer - Ultra-Low Latency POE Camera Control System

## Project Overview

This is a high-performance C++ rewrite of the Python-based multi-camera RTSP viewer, designed for ultra-low latency viewing and control of multiple POE-powered IP cameras simultaneously. The goal is to minimize latency, maximize throughput, and provide a robust interface for robotics competitions.

## Project Goals

### Primary Objectives
- **Ultra-Low Latency**: Achieve <50ms glass-to-glass latency per camera stream
- **Multi-Stream Support**: Handle 4-8+ simultaneous RTSP camera streams
- **Hardware Acceleration**: Leverage VAAPI, NVENC, and Intel QSV for efficient decoding
- **True Parallelism**: Use native C++ threading for concurrent camera processing
- **Robust Error Handling**: Auto-reconnection, stream health monitoring, and graceful degradation
- **Dual-Quality Streaming**: Switch between high-res and low-res streams on-the-fly

### Performance Targets
- 20-40% lower CPU usage vs Python implementation
- 5-15ms latency reduction per camera
- Support for 8+ cameras at 30fps each
- Stable operation under network instability

## Technology Stack

### Core Libraries
- **FFmpeg (libav\*)**: RTSP stream handling and video decoding
  - `libavformat` - Stream demuxing
  - `libavcodec` - H.264/H.265 decoding
  - `libavutil` - Utilities and frame handling
  - `libswscale` - Color conversion and scaling
  
- **OpenCV**: Frame processing and display (C++ API)
  
- **C++17/20**: Modern C++ features for performance and safety

### Hardware Acceleration Support
- **VAAPI** (Intel/AMD integrated graphics)
- **NVENC/NVDEC** (NVIDIA GPUs)
- **Intel QSV** (Intel Quick Sync Video)

### Build System
- **CMake 3.15+**: Cross-platform build configuration
- **pkg-config**: Dependency management

### Optional Libraries
- **spdlog**: Fast structured logging
- **nlohmann/json**: JSON configuration parsing
- **SDL2**: Alternative low-latency display backend

## Architecture Overview

### Component Design

```
┌─────────────────────────────────────────────────────────────┐
│                        Main Viewer                           │
│  - Window management                                         │
│  - View modes (Grid/Fullscreen/Quad)                        │
│  - Keyboard input handling                                   │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│CameraStream 1│    │CameraStream 2│    │CameraStream N│
│  - std::thread│    │  - std::thread│    │  - std::thread│
│  - FFmpeg     │    │  - FFmpeg     │    │  - FFmpeg     │
│  - HW Accel   │    │  - HW Accel   │    │  - HW Accel   │
│  - Frame Queue│    │  - Frame Queue│    │  - Frame Queue│
└──────────────┘    └──────────────┘    └──────────────┘
```

### Key Classes

1. **CameraStream**
   - Manages individual RTSP stream connection
   - Runs in dedicated thread
   - Handles FFmpeg decoding pipeline
   - Hardware acceleration management
   - Frame queue management

2. **HardwareAccelerator**
   - Detects available hardware acceleration
   - Configures FFmpeg hwaccel context
   - Manages hardware decoder lifecycle

3. **FrameQueue**
   - Lock-free or mutex-protected queue
   - Ultra-low latency frame buffering (queue size = 1)
   - Frame dropping for latency management

4. **Viewer**
   - Main application controller
   - Multi-camera layout management
   - OpenCV window handling
   - User input processing

5. **Config**
   - JSON-based configuration loader
   - Camera URL management
   - Performance settings

## Project Structure

```
RRL-2026-CAMERAS/
├── CMakeLists.txt                 # Main CMake build file
├── README.md                      # This file
├── DEVELOPMENT.md                 # Development guide and instructions
├── LICENSE                        # MIT License
├── .gitignore
│
├── include/                       # Public headers
│   ├── camera_stream.h           # Individual camera stream handler
│   ├── hardware_accel.h          # Hardware acceleration detection/config
│   ├── frame_queue.h             # Thread-safe frame queue
│   ├── viewer.h                  # Main viewer/display manager
│   ├── config.h                  # Configuration loader
│   └── types.h                   # Common types and enums
│
├── src/                          # Implementation files
│   ├── camera_stream.cpp
│   ├── hardware_accel.cpp
│   ├── frame_queue.cpp
│   ├── viewer.cpp
│   ├── config.cpp
│   └── main.cpp                  # Entry point
│
├── config/                       # Configuration files
│   ├── cameras.json              # Camera RTSP URLs
│   └── settings.json             # App settings
│
├── docs/                         # Documentation
│   ├── ARCHITECTURE.md           # Detailed architecture
│   ├── HARDWARE_ACCEL.md         # Hardware acceleration guide
│   ├── API.md                    # API documentation
│   └── MIGRATION.md              # Migration from Python version
│
├── tests/                        # Unit tests (future)
│   └── ...
│
└── scripts/                      # Utility scripts
    ├── setup_deps.sh             # Install dependencies
    └── build.sh                  # Build script

```

## Development Roadmap

### Phase 1: Core Infrastructure (Week 1)
- [ ] Project structure and CMake setup
- [ ] Basic FFmpeg integration (single camera)
- [ ] Frame decoding and OpenCV display
- [ ] Configuration file parsing

### Phase 2: Multi-Camera Support (Week 2)
- [ ] Thread-per-camera architecture
- [ ] Frame queue implementation
- [ ] Multi-camera grid layout
- [ ] Synchronization and thread safety

### Phase 3: Hardware Acceleration (Week 3)
- [ ] VAAPI integration
- [ ] NVENC/NVDEC support
- [ ] Intel QSV support
- [ ] Auto-detection and fallback

### Phase 4: Features & Polish (Week 4)
- [ ] Dual-stream quality switching
- [ ] Camera rotation support
- [ ] View modes (Grid/Fullscreen/Quad)
- [ ] Error handling and auto-reconnect
- [ ] Health monitoring and statistics

### Phase 5: Optimization & Testing
- [ ] Performance profiling
- [ ] Memory leak detection
- [ ] Latency benchmarking
- [ ] Stress testing with 8+ cameras

## Quick Start

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    libswscale-dev \
    libopencv-dev \
    libva-dev \
    libva-drm2 \
    vainfo
```

**Optional Dependencies:**
```bash
# For NVIDIA support
sudo apt-get install nvidia-cuda-toolkit

# For logging
sudo apt-get install libspdlog-dev

# For SDL2 display backend
sudo apt-get install libsdl2-dev
```

### Building

```bash
# Clone the repository
git clone https://github.com/UPRobotics/RRL-2026-CAMERAS.git
cd RRL-2026-CAMERAS

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run
./camera_viewer
```

### Configuration

Edit `config/cameras.json`:
```json
{
  "cameras": [
    {
      "id": 1,
      "name": "Camera 1",
      "url_high": "rtsp://192.168.0.200:554/stream1",
      "url_low": "rtsp://192.168.0.200:554/stream2"
    },
    {
      "id": 2,
      "name": "Camera 2",
      "url_high": "rtsp://192.168.0.201:554/stream1",
      "url_low": "rtsp://192.168.0.201:554/stream2"
    }
  ],
  "performance": {
    "enable_hw_accel": true,
    "max_queue_size": 1,
    "target_fps": 30
  }
}
```

## Controls (Target Interface)

- **ESC / q**: Quit application
- **SPACE**: Toggle view mode (Grid → Quad → Fullscreen)
- **LEFT/RIGHT arrows**: Navigate cameras in fullscreen
- **R**: Rotate current camera 90° clockwise
- **T**: Reset all camera rotations
- **L**: Switch stream quality (high-res ↔ low-res)
- **H**: Show health status
- **P**: Restart all cameras
- **Numpad 7/9/1/3**: Assign cameras to quad positions

## Performance Monitoring

The application will provide real-time statistics:
- FPS per camera
- Latency measurements
- CPU/GPU usage
- Network bandwidth
- Frame drops and reconnection events

## Contributing

1. Follow C++ Core Guidelines
2. Use clang-format for code formatting
3. Write unit tests for new features
4. Document public APIs
5. Keep latency as the primary concern

## Comparison with Python Version

| Metric | Python | C++ (Target) |
|--------|---------|--------------|
| Latency | ~70-100ms | ~40-60ms |
| CPU Usage (4 cams) | ~80% | ~40-50% |
| Memory | ~500MB | ~200MB |
| Startup Time | ~8s | ~2-3s |
| Thread Efficiency | GIL-limited | Native |

## License

MIT License - See LICENSE file

## References

- Python version: `UPRobotics/RRL-2025`
- FFmpeg Documentation: https://ffmpeg.org/doxygen/trunk/
- OpenCV C++ API: https://docs.opencv.org/4.x/
- Hardware Accel Guide: `docs/HARDWARE_ACCEL.md`

## Contact

For questions or issues, please open a GitHub issue or contact the UPRobotics team.
