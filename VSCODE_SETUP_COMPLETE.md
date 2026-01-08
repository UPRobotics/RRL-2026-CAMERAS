# VSCode Setup Complete! 🎉

## ✅ What's Configured

### Project Structure
- ✅ Reorganized to root-level project (removed cpp_viewer subfolder)
- ✅ Clean directory structure with src/, include/, config/, docs/, tests/
- ✅ Build directory properly configured

### VSCode Configuration Files
- ✅ `.vscode/settings.json` - CMake and editor settings
- ✅ `.vscode/tasks.json` - Build tasks
- ✅ `.vscode/launch.json` - Debug configurations
- ✅ `.vscode/c_cpp_properties.json` - IntelliSense configuration
- ✅ `.vscode/extensions.json` - Recommended extensions
- ✅ `RRL-2026-CAMERAS.code-workspace` - Workspace file

### Build System
- ✅ CMake with NMake Makefiles generator
- ✅ vcpkg integration for dependencies
- ✅ Visual Studio 2026 compiler (MSVC 19.50)
- ✅ PowerShell build script (`build.ps1`)

### Dependencies Installed
- ✅ FFmpeg 8.0.1 (with avcodec, avformat, swscale, swresample)
- ✅ SDL2 2.32.10
- ✅ spdlog 1.17.0
- ✅ nlohmann-json 3.12.0
- ✅ concurrentqueue 1.0.4

## 🚀 How to Use VSCode

### First Time Setup

1. **Open the project in VSCode**:
   ```powershell
   cd C:\Github\RRL-2026-CAMERAS
   code .
   ```
   Or open the workspace file: `RRL-2026-CAMERAS.code-workspace`

2. **Install recommended extensions** (VSCode will prompt you):
   - C/C++ (ms-vscode.cpptools)
   - CMake Tools (ms-vscode.cmake-tools)
   - CMake (twxs.cmake)
   - PowerShell (ms-vscode.powershell)

3. **Configure CMake**:
   - Press `Ctrl+Shift+P`
   - Type "CMake: Configure"
   - Select "NMake Makefiles"

### Building

**Method 1: Quick Build (Recommended)**
- Press `Ctrl+Shift+B`
- Select "CMake: Build"

**Method 2: Build Script**
- Open terminal (`Ctrl+~`)
- Run: `.\build.ps1`

**Method 3: CMake Tools**
- Press `F7`
- Or click "Build" in the status bar

### Running

**Method 1: From VSCode**
- Press `F5` (with debugger)
- Or `Ctrl+F5` (without debugger)

**Method 2: From Terminal**
```powershell
.\build\RTSPCameraViewer.exe
```

### Debugging

1. Set breakpoints by clicking left of line numbers (or `F9`)
2. Press `F5` to start debugging
3. Use debug toolbar or these shortcuts:
   - `F5` - Continue
   - `F10` - Step over
   - `F11` - Step into
   - `Shift+F11` - Step out
   - `Shift+F5` - Stop

### Build Commands

```powershell
# Quick build
.\build.ps1

# Clean and rebuild
.\build.ps1 -Rebuild

# Configure only
.\build.ps1 -Configure

# Clean build directory
.\build.ps1 -Clean

# Debug build
.\build.ps1 -BuildType Debug
```

## 📁 Final Project Structure

```
C:\Github\RRL-2026-CAMERAS\
├── .vscode/                      # VSCode configuration
│   ├── settings.json
│   ├── tasks.json
│   ├── launch.json
│   ├── c_cpp_properties.json
│   └── extensions.json
├── src/                          # C++ source files
│   └── main.cpp
├── include/                      # C++ header files
│   └── types.h
├── config/                       # Configuration files
│   └── config.ini
├── build/                        # Build output (git ignored)
├── docs/                         # Documentation
├── tests/                        # Unit tests (future)
├── CMakeLists.txt               # CMake configuration
├── vcpkg.json                   # vcpkg dependencies
├── build.ps1                    # Build script
├── .gitignore                   # Git ignore rules
├── README.md                    # Project overview
├── VSCODE_GUIDE.md              # VSCode usage guide
├── BUILDING.md                  # Build instructions
├── SETUP_GUIDE.md               # Setup instructions
└── CPP_REWRITE_PLAN.md          # Development roadmap
```

## 🎯 Quick Reference

### Keyboard Shortcuts
| Shortcut | Action |
|----------|--------|
| `F7` | Build |
| `Ctrl+Shift+B` | Show build tasks |
| `F5` | Start debugging |
| `Ctrl+F5` | Run without debugging |
| `F9` | Toggle breakpoint |
| `F10` | Step over |
| `F11` | Step into |
| `Ctrl+~` | Open terminal |

### IntelliSense Features
- ✅ Auto-completion
- ✅ Go to definition (`F12`)
- ✅ Find all references (`Shift+F12`)
- ✅ Peek definition (`Alt+F12`)
- ✅ Error squiggles
- ✅ Parameter hints

## 📚 Documentation

- **VSCODE_GUIDE.md** - Detailed VSCode usage
- **BUILDING.md** - Build instructions
- **SETUP_GUIDE.md** - Initial setup
- **CPP_REWRITE_PLAN.md** - Development roadmap
- **README.md** - Project overview

## 🔧 Troubleshooting

**Build fails in VSCode**:
1. Open terminal in VSCode (`Ctrl+~`)
2. Run `.\build.ps1` to see detailed errors

**IntelliSense not working**:
1. Press `Ctrl+Shift+P`
2. Type "C/C++: Reset IntelliSense Database"
3. Reload window (`Ctrl+R`)

**CMake can't find compiler**:
1. Close VSCode
2. Open "Developer PowerShell for VS 2026"
3. Run: `code .` from the project directory

## ✨ Next Steps

1. **Start coding!** Edit `src/main.cpp`
2. **Follow the roadmap** in `CPP_REWRITE_PLAN.md`
3. **Current Phase**: Phase 1 - Single Camera Prototype
   - Implement RTSP connection
   - Add FFmpeg H.264 decoding
   - Create SDL2 rendering window

## 🎊 You're All Set!

Everything is configured and ready to go. Just open VSCode and start developing!

```powershell
cd C:\Github\RRL-2026-CAMERAS
code .
```

Happy coding! 🚀
