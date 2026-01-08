# Visual Studio Code Development Guide

## 🎯 Quick Start

### Prerequisites
All dependencies are already installed! Just make sure you have these VSCode extensions:
- **C/C++** (ms-vscode.cpptools)
- **CMake Tools** (ms-vscode.cmake-tools)
- **CMake** (twxs.cmake)

VSCode will prompt you to install these when you open the project.

### Building in VSCode

#### Method 1: Using CMake Tools (Recommended)
1. Open VSCode in this directory: `C:\Github\RRL-2026-CAMERAS`
2. Press `Ctrl+Shift+P` and type "CMake: Configure"
3. Select "NMake Makefiles" as the generator
4. Press `F7` or click "Build" in the CMake Tools status bar

#### Method 2: Using Tasks
1. Press `Ctrl+Shift+B` (Build Task)
2. Select "CMake: Build" for quick build
3. Select "Build (Full)" for clean configure + build

#### Method 3: Using Terminal
Open integrated terminal (`Ctrl+~`) and run:
```powershell
.\build.ps1
```

### Running the Application

#### Method 1: Using Debugger (F5)
1. Press `F5` to launch with debugger
2. Or select "Launch RTSP Camera Viewer" from Run menu

#### Method 2: Using Terminal
```powershell
.\build\RTSPCameraViewer.exe
```

### Debugging

Two debug configurations available:
- **Launch RTSP Camera Viewer** - Run normally
- **Debug RTSP Camera Viewer** - Stop at entry point

Press `F5` to start debugging.

## 📁 Project Structure

```
RRL-2026-CAMERAS/
├── .vscode/               # VSCode configuration
│   ├── settings.json      # Workspace settings
│   ├── tasks.json         # Build tasks
│   ├── launch.json        # Debug configurations
│   └── c_cpp_properties.json  # IntelliSense config
├── src/                   # Source files (.cpp)
├── include/               # Header files (.h)
├── config/                # Configuration files
│   └── config.ini         # Runtime configuration
├── build/                 # Build output (generated)
├── docs/                  # Documentation
├── tests/                 # Unit tests (future)
├── CMakeLists.txt         # CMake build configuration
├── vcpkg.json            # Dependencies manifest
├── build.ps1             # Quick build script
└── README.md             # Main documentation
```

## 🔧 VSCode Configuration Details

### IntelliSense
- Configured for C++20 standard
- Includes FFmpeg, SDL2, and all vcpkg dependencies
- MSVC compiler integration
- Automatic symbol navigation

### Build System
- **Generator**: NMake Makefiles
- **Compiler**: MSVC 19.50 (Visual Studio 2026)
- **Toolchain**: vcpkg (C:\vcpkg)
- **Build Type**: Release

### Keyboard Shortcuts
- `F7` - Build
- `Ctrl+Shift+B` - Show build tasks
- `F5` - Start debugging
- `Ctrl+F5` - Run without debugging
- `Shift+F5` - Stop debugging
- `F9` - Toggle breakpoint
- `F10` - Step over
- `F11` - Step into

## 🐛 Troubleshooting

### Build Fails
1. Ensure Visual Studio Developer Tools are in PATH
2. Try running from "Developer PowerShell for VS 2026"
3. Or use the `build.ps1` script which sets up environment

### IntelliSense Not Working
1. Press `Ctrl+Shift+P`
2. Type "C/C++: Reset IntelliSense Database"
3. Reload window

### CMake Configuration Issues
1. Delete `build/` directory
2. Press `Ctrl+Shift+P` → "CMake: Delete Cache and Reconfigure"

## 📚 Next Steps

See `CPP_REWRITE_PLAN.md` for the development roadmap.

Current Phase: **Phase 1 - Foundation**
- Single camera RTSP connection
- FFmpeg H.264 decoding
- SDL2 rendering

## 🔗 Useful Links

- [FFmpeg Documentation](https://ffmpeg.org/doxygen/trunk/)
- [SDL2 Documentation](https://wiki.libsdl.org/SDL2/FrontPage)
- [C++20 Reference](https://en.cppreference.com/w/cpp/20)
- [vcpkg Documentation](https://learn.microsoft.com/en-us/vcpkg/)
