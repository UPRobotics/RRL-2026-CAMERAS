# RTSP Camera Viewer

**Ultra-low latency multi-camera RTSP viewer built with C++, SDL2, and FFmpeg**

A high-performance camera monitoring system designed for robotics competitions, supporting multiple simultaneous RTSP streams with hardware-accelerated decoding and a modern UI.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Platform](https://img.shields.io/badge/platform-Ubuntu%2022.04-orange)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange)

## ✨ Features

- 🎥 **Multi-Camera Support** - View up to 16 cameras simultaneously
- ⚡ **Ultra-Low Latency** - Hardware-accelerated decoding (VAAPI, VDPAU)
- 🎨 **Modern UI** - Clean interface with toolbar, stats panel, and console
- 📐 **Multiple View Modes** - Fullscreen, 2x2 grid, or NxN auto-layout
- 🔄 **Quality Switching** - Toggle between high-res and low-res streams
- ⌨️ **Keyboard Controls** - Quick navigation and view mode switching
- 📊 **Real-time Stats** - FPS, bitrate, and connection status

## 🚀 Quick Start

### Prerequisites

- **Ubuntu 22.04 LTS** (64-bit)
- **GCC 11.4+** or compatible C++20 compiler
- **CMake 3.25+**
- **Git**

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/UPRobotics/RRL-2026-CAMERAS.git
   cd RRL-2026-CAMERAS
   ```

2. **Install dependencies**
   ```bash
   sudo apt update && sudo apt install -y \
       cmake g++ make pkg-config \
       libsdl2-dev libsdl2-ttf-dev \
       libavcodec-dev libavformat-dev libswscale-dev libswresample-dev \
       libspdlog-dev nlohmann-json3-dev libconcurrentqueue-dev
   ```

3. **Build the application**
   ```bash
   ./build.sh
   ```
   
   Or manually:
   ```bash
   mkdir -p build && cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make -j$(nproc)
   ```

4. **Launch the application**
   ```bash
   ./build/RTSPCameraViewer
   ```

That's it! See [Building Guide](docs/BUILDING.md) for detailed instructions.

## 🎮 Usage

### Basic Controls

| Key | Action |
|-----|--------|
| `Q` or `Esc` | Quit application |
| `C` | Toggle console window |
| `F` | Toggle fullscreen |
| `1` | Fullscreen view mode |
| `2` | 2x2 grid view |
| `3` | NxN auto-layout grid |
| `←` `→` | Navigate cameras (fullscreen mode) |

### Starting Cameras

1. Click the **"Start Cameras"** button in the toolbar
2. Cameras will appear in the grid layout

## ⚙️ Configuration

Edit `config/settings.json` to customize:

```json
{
  "display": {
    "fullscreen_on_startup": false,
    "remember_window_position": true,
    "last_window_width": 1280,
    "last_window_height": 720
  },
  "keybindings": {
    "quit": "Q",
    "toggle_console": "C",
    "toggle_fullscreen": "F",
    "view_mode_fullscreen": "1",
    "view_mode_2x2": "2",
    "view_mode_grid": "3"
  }
}
```

## 📁 Project Structure

```
RRL-2026-CAMERAS/
├── build.ps1                   # Build script
├── CMakeLists.txt              # Build configuration
├── vcpkg.json                  # Dependencies
│
├── config/
│   └── settings.json           # Application settings
│
├── include/                    # Header files
│   ├── main_window.h          # Main window
│   ├── camera_grid.h          # Camera layout
│   ├── console_window.h       # Debug console
│   ├── settings_manager.h     # Settings
│   └── ...
│
├── src/                        # Source files
│   ├── main.cpp               # Entry point
│   ├── main_window.cpp
│   ├── camera_grid.cpp
│   └── ...
│
├── docs/                       # Documentation
│   ├── BUILDING.md            # Build instructions
│   ├── ARCHITECTURE.md        # System design
│   └── DEVELOPMENT.md         # Developer guide

```

## 🏗️ Architecture

The application uses a component-based architecture:

```
┌─────────────────────────────────────────┐
│           MainWindow                     │
│  - SDL2 window & renderer                │
│  - Event handling                        │
│  - UI layout management                  │
└─────────────────────────────────────────┘
           │
    ┌──────┴──────┬──────────────┐
    ▼             ▼              ▼
┌─────────┐  ┌──────────┐  ┌──────────┐
│ Toolbar │  │  Camera  │  │  Stats   │
│         │  │   Grid   │  │  Panel   │
└─────────┘  └──────────┘  └──────────┘
                   │
       ┌───────────┼───────────┐
       ▼           ▼           ▼
   Camera 1    Camera 2    Camera N
   [Future: RTSP streaming]
```

See [Architecture Guide](docs/ARCHITECTURE.md) for details.

## 📊 Performance

Target specifications:
- **Latency**: <100ms glass-to-glass per stream
- **CPU Usage**: <50% with 4 cameras (hardware decoding)
- **Memory**: <500MB with 8 cameras
- **Framerate**: Up to 30 FPS per camera

## 🔧 Development

### Building from Source

```bash
# Debug build (with symbols)
mkdir -p build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Release build (optimized)
cd ../build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Project Status

**Current Phase: UI Foundation** ✅
- [x] Build system and dependencies
- [x] SDL2 window and rendering
- [x] UI components (toolbar, camera grid, stats, console)
- [x] Settings management
- [ ] RTSP stream decoding (next phase)

See [Development Guide](docs/DEVELOPMENT.md) for contributing.

## 🐛 Troubleshooting

### Build Issues

**"FFmpeg not found"**
```bash
sudo apt update && sudo apt install -y \
   libavcodec-dev libavformat-dev libswscale-dev libswresample-dev
```

### Runtime Issues

**High CPU usage**
- Ensure you built Release version, not Debug
- Debug builds are 10-50x slower

For more help, see [Troubleshooting Guide](docs/TROUBLESHOOTING.md).

## 📚 Documentation

- [Building Guide](docs/BUILDING.md) - Detailed build instructions
- [Architecture Guide](docs/ARCHITECTURE.md) - System design and components
- [Development Guide](docs/DEVELOPMENT.md) - Contributing and code structure
- [Troubleshooting Guide](docs/TROUBLESHOOTING.md) - Common issues and solutions

## 🤝 Contributing

Contributions welcome! Please:
1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Submit pull requests to `main` branch

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built for **UP Robotics** robotics competition team
- Based on Python version from RRL-2025 project
- Uses [FFmpeg](https://ffmpeg.org/), [SDL2](https://www.libsdl.org/), and [spdlog](https://github.com/gabime/spdlog)

---

**Questions?** Open an issue or contact the UP Robotics team.
