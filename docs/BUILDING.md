# Building RTSP Camera Viewer on Ubuntu 22.04

Ultra-low latency RTSP multi-camera viewer written in modern C++20.

## 🚀 Quick Setup

### Prerequisites

**Required:**
- **Ubuntu 22.04 LTS** (64-bit)
- **GCC 11.4+** or compatible C++20 compiler
- **CMake 3.25+**
- **Git**

**Optional (for hardware acceleration):**
- NVIDIA GPU with VAAPI support for hardware-accelerated video decoding
- Mesa drivers for Intel/AMD GPU acceleration

### Automated Setup (Recommended)

The quickest way to build:

```bash
# Clone the repository
git clone https://github.com/UPRobotics/RRL-2026-CAMERAS.git
cd RRL-2026-CAMERAS

# Run the build script
./build.sh
```

This script will:
1. ✅ Check for all required dependencies
2. ✅ Notify you of any missing packages
3. ✅ Configure CMake build system
4. ✅ Compile the application with optimization flags

### Manual Setup

If you prefer manual control or need to customize the build:

#### 1. Install System Dependencies

All dependencies are available through Ubuntu's package manager:

```bash
sudo apt update && sudo apt install -y \
    cmake g++ make pkg-config \
    libsdl2-dev libsdl2-ttf-dev \
    libavcodec-dev libavformat-dev libswscale-dev libswresample-dev \
    libspdlog-dev nlohmann-json3-dev libconcurrentqueue-dev
```

**Package descriptions:**
- `cmake` - Build system generator (v4.2.3+)
- `g++` - GNU C++ compiler with C++20 support
- `make` - Build automation tool
- `pkg-config` - Library metadata helper
- `libsdl2-dev` - Simple DirectMedia Layer for graphics/input
- `libsdl2-ttf-dev` - TrueType font rendering for SDL2
- `libavcodec-dev`, `libavformat-dev`, `libswscale-dev`, `libswresample-dev` - FFmpeg libraries for video processing
- `libspdlog-dev` - Fast C++ logging library
- `nlohmann-json3-dev` - JSON for Modern C++
- `libconcurrentqueue-dev` - Lock-free concurrent queue

#### 2. Configure CMake

```bash
# Navigate to project directory
cd RRL-2026-CAMERAS

# Create build directory
mkdir -p build && cd build

# Configure CMake with Release build type
cmake -DCMAKE_BUILD_TYPE=Release ..
```

CMake will automatically:
- Detect your compiler (GCC/Clang)
- Find all installed dependencies via pkg-config
- Generate optimized build files with LTO enabled
- Create compile_commands.json for IDE support

#### 3. Build the Application
cd "C:\Users\Chumbi\OneDrive - up.edu.mx\Documents\GitHub\RRL-2026-CAMERAS\cpp_viewer"

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64
```

## 🔨 Building

### Using Visual Studio

#### 3. Build the Application

```bash
# Build with all available CPU cores
make -j$(nproc)

# The compiled executable will be at:
# build/RTSPCameraViewer
```

**Build options:**
- **Release** (default): Optimized with `-O3`, `-march=native`, and LTO enabled
- **Debug**: Add `-DCMAKE_BUILD_TYPE=Debug` to CMake for debugging symbols

#### 4. Run the Application

```bash
# From project root
./build/RTSPCameraViewer

# Or from build directory
cd build && ./RTSPCameraViewer
```

## 🔧 Build Customization

### Debug Build

For development with debug symbols:

```bash
mkdir -p build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Custom Compiler

```bash
# Use Clang instead of GCC
CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Verbose Build Output

```bash
make VERBOSE=1
```

## ⚙️ Configuration

Edit [config/config.ini](../config/config.ini) to set your camera URLs:

```ini
[cameras]
camera1_url = rtsp://192.168.0.200:554/stream1
camera2_url = rtsp://192.168.0.201:554/stream1
# Add more cameras...

[display]
window_width = 1920
window_height = 1080
max_fps = 120

[performance]
hardware_acceleration = vaapi  # or vdpau, none
ultra_low_latency = true
```

## 🎮 Controls

| Key | Action |
|-----|--------|
| **ESC** / **Q** | Quit application |
| **SPACE** | Toggle grid/fullscreen view |
| **←** / **→** | Navigate cameras (fullscreen) |
| **L** | Switch stream quality |
| **R** | Rotate camera |
| **S** | Show statistics |
| **P** | Restart cameras |

## 📁 Project Structure

```
cpp_viewer/
├── CMakeLists.txt          # Build configuration
├── vcpkg.json              # Dependency manifest
├── config.ini              # Runtime configuration
├── setup.ps1               # Automated setup script
│
├── include/                # Header files
│   ├── rtsp_camera.h       # Camera capture class
│   ├── display_manager.h   # Display and rendering
│   ├── config.h            # Configuration parser
│   ├── utils.h             # Utilities
│   └── types.h             # Common types
│
├── src/                    # Source files
│   ├── main.cpp            # Entry point
│   ├── rtsp_camera.cpp     # Camera implementation
│   ├── display_manager.cpp # Display implementation
│   ├── config.cpp          # Config parser
│   └── utils.cpp           # Utilities
│
└── build/                  # Build output (generated)
    └── Release/
        └── RTSPCameraViewer.exe
```

## 🐛 Troubleshooting

### "CMake Error: Could not find CMAKE_TOOLCHAIN_FILE"

Set the VCPKG_ROOT environment variable:
```powershell
setx VCPKG_ROOT "C:\vcpkg"
# Restart your terminal/IDE
```

### "FFmpeg not found" or pkg-config errors

Make sure vcpkg installation completed successfully:
```powershell
cd C:\vcpkg
.\vcpkg list
# Should show ffmpeg, sdl2, spdlog, etc.
```

If packages are missing, reinstall them:
```powershell
.\vcpkg install ffmpeg[core,avcodec,avformat,avutil,swscale]:x64-windows --recurse
```

### "Cannot open SDL2.dll"

SDL2.dll should be copied automatically to the build directory. If missing:
```powershell
copy "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" "build\Release\"
```

### Hardware Acceleration Issues

If hardware acceleration fails, the application will automatically fall back to software decoding. Check logs for details.

For NVIDIA GPUs:
- Update GPU drivers to latest version
- Verify NVDEC support: `ffmpeg -hwaccels` (should list `cuda`)

For Intel/AMD integrated graphics:
- Set `hardware_acceleration = d3d11va` in config.ini
- Update graphics drivers

### High CPU Usage

Make sure you're building in **Release** mode, not Debug:
```powershell
cmake --build . --config Release
```

Debug builds are 10-50x slower and should only be used for development.

## 📊 Performance Tips

1. **Use Release build** - Critical for performance
2. **Enable hardware acceleration** - Reduces CPU by 60-80%
3. **Lower camera resolution** - Use 720p or 480p streams if available
4. **Adjust max_fps** - Set to your monitor refresh rate (60, 120, 144)
5. **Reduce active cameras** - Start with 4 cameras, add more as needed

## 🔬 Development

### Building Debug Version

```powershell
cmake --build . --config Debug
```

Debug builds include:
- Debug symbols for breakpoints
- Additional logging
- Runtime checks (slower)
- No optimizations

### Running Tests

```powershell
cd build
ctest -C Release
```

### Code Formatting

Using clang-format:
```powershell
clang-format -i src/*.cpp include/*.h
```

## 📝 Current Status

**Phase 1: Foundation** (In Progress)
- [x] Build system setup
- [x] Dependency management
- [x] Project structure
- [ ] Single camera prototype
- [ ] Basic rendering
- [ ] Configuration parser

See [CPP_REWRITE_PLAN.md](../CPP_REWRITE_PLAN.md) for complete roadmap.

## 📚 Additional Resources

- **FFmpeg Documentation**: https://ffmpeg.org/doxygen/trunk/
- **SDL2 Wiki**: https://wiki.libsdl.org/
- **vcpkg GitHub**: https://github.com/microsoft/vcpkg
- **C++ Reference**: https://en.cppreference.com/

## ⚖️ License

Same license as the parent project.

---

**Ready to proceed?**  
After setup completes, proceed to implementing the single camera prototype (Phase 1).
