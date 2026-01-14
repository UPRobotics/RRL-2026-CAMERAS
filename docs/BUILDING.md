# C++ RTSP Camera Viewer

Ultra-low latency RTSP multi-camera viewer written in modern C++20.

## 🚀 Quick Setup

### Prerequisites

**Required:**
- **Windows 10/11** (64-bit)
- **Visual Studio 2022** with "Desktop development with C++" workload
  - Download: https://visualstudio.microsoft.com/downloads/
- **CMake 3.25+**
  - Download: https://cmake.org/download/
- **Git**
  - Download: https://git-scm.com/download/win

**Optional (for NVIDIA hardware acceleration):**
- NVIDIA GPU with driver version 471.11 or later
- CUDA Toolkit 11.0+ (for NVDEC support)

### Automated Setup (Recommended)

Run the PowerShell setup script:

```powershell
# Open PowerShell in the cpp_viewer directory
cd cpp_viewer

# If you get execution policy errors, run:
# Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned

# Run setup
.\setup.ps1
```

This script will:
1. ✅ Check for required tools (CMake, Git, Visual Studio)
2. ✅ Install and configure vcpkg package manager
3. ✅ Download and compile all dependencies (FFmpeg, SDL2, etc.)
4. ✅ Configure CMake build system
5. ✅ Create Visual Studio solution

**Note:** First-time setup downloads and compiles FFmpeg, which can take 15-30 minutes depending on your CPU.

### Manual Setup

If you prefer manual setup or the script fails:

#### 1. Install vcpkg

```powershell
# Clone vcpkg
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Set environment variable
setx VCPKG_ROOT "C:\vcpkg"

# Integrate with Visual Studio
.\vcpkg integrate install
```

#### 2. Install Dependencies

```powershell
cd C:\vcpkg

# Install all required packages (this takes 15-30 minutes)
.\vcpkg install ffmpeg[core,avcodec,avformat,avutil,swscale,swresample]:x64-windows
.\vcpkg install sdl2:x64-windows
.\vcpkg install spdlog:x64-windows
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install concurrentqueue:x64-windows
```

#### 3. Configure CMake

```powershell
# Navigate to cpp_viewer directory
cd "C:\Users\Chumbi\OneDrive - up.edu.mx\Documents\GitHub\RRL-2026-CAMERAS\cpp_viewer"

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64
```

## 🔨 Building

### Using Visual Studio

1. Open `build/RTSPCameraViewer.sln` in Visual Studio 2022
2. Select **Release** configuration (important for performance!)
3. Build → Build Solution (Ctrl+Shift+B)
4. Executable: `build/Release/RTSPCameraViewer.exe`

### Using Command Line

```powershell
cd build

# Build Release version (optimized)
cmake --build . --config Release

# Or Debug version (for development)
cmake --build . --config Debug
```

## ⚙️ Configuration

Edit `config.ini` to set your camera URLs:

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
hardware_acceleration = auto  # or d3d11va, nvdec, none
ultra_low_latency = true
```

## 🎮 Controls (Planned)

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
