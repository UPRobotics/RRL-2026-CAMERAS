# Troubleshooting Guide

Common issues and their solutions.

## Table of Contents
- [Build Issues](#build-issues)
- [Runtime Issues](#runtime-issues)
- [Performance Issues](#performance-issues)
- [Development Issues](#development-issues)

---

## Build Issues

### "vcpkg.exe not found" or "Could not find CMAKE_TOOLCHAIN_FILE"

**Cause:** vcpkg not installed or environment variable not set

**Solution:**
```powershell
# Check if vcpkg exists
Test-Path "C:\vcpkg\vcpkg.exe"

# If false, install vcpkg
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Set environment variable
setx VCPKG_ROOT "C:\vcpkg"

# IMPORTANT: Restart your terminal/IDE after setting env variable
```

### "FFmpeg packages not found"

**Cause:** FFmpeg dependencies not installed via vcpkg

**Solution:**
```powershell
cd C:\vcpkg

# Check what's installed
.\vcpkg list | Select-String "ffmpeg"

# If nothing shows, install FFmpeg
.\vcpkg install ffmpeg[core,avcodec,avformat,avutil,swscale]:x64-windows

# This takes 15-30 minutes on first install
```

### "SDL2.dll not found" when running

**Cause:** DLL not copied to build directory

**Solution:**
```powershell
# Copy manually
copy "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" "build\Release\"
copy "C:\vcpkg\installed\x64-windows\bin\SDL2_ttf.dll" "build\Release\"

# Or rebuild with build script
.\build.ps1
```

### Build hangs at "Installing dependencies"

**Cause:** FFmpeg compilation is CPU-intensive and takes time

**This is normal!** First-time FFmpeg build can take 20-30 minutes.

**Check progress:**
```powershell
# In a separate PowerShell window
cd C:\vcpkg
Get-Process | Where-Object {$_.ProcessName -like "*cl*"}

# If processes are running, it's still compiling
# Be patient and let it finish
```

### "Cannot open file 'avformat.lib'"

**Cause:** FFmpeg libraries not properly installed

**Solution:**
```powershell
cd C:\vcpkg

# Remove and reinstall
.\vcpkg remove ffmpeg:x64-windows
.\vcpkg install ffmpeg[core,avcodec,avformat,avutil,swscale,swresample]:x64-windows --recurse

# Clean and rebuild project
Remove-Item build -Recurse -Force
.\build.ps1
```

### CMake configuration fails

**Cause:** Various CMake issues

**Solution:**
```powershell
# Delete CMake cache
Remove-Item build\CMakeCache.txt -Force
Remove-Item build\CMakeFiles -Recurse -Force

# Reconfigure
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

### "Visual Studio 2022 not found"

**Cause:** Visual Studio not installed or wrong version

**Solution:**
1. Install Visual Studio 2022 (free Community edition)
2. During installation, select "Desktop development with C++"
3. Restart your terminal
4. Run build script again

---

## Runtime Issues

### Application crashes on startup

**Check 1 - DLLs present:**
```powershell
Get-ChildItem build\Release\*.dll
# Should show SDL2.dll, SDL2_ttf.dll, and others
```

**Check 3 - Logs:**
- Look for error messages in terminal
- Press `C` to open console window (if app starts)

### "Failed to initialize SDL"

**Cause:** Graphics driver issue or missing DLLs

**Solution:**
```powershell
# 1. Update graphics drivers
# 2. Verify SDL2.dll exists in build\Release\
# 3. Try running from command line instead of double-clicking
cd build\Release
.\RTSPCameraViewer.exe
```

### Window appears but is black/frozen

**Cause:** Renderer not initializing properly

**Solution:**
1. Update graphics drivers
2. Try different rendering backend:
   ```powershell
   # Force software renderer (slow but compatible)
   $env:SDL_RENDER_DRIVER="software"
   .\build\RTSPCameraViewer.exe
   ```
3. Check console for errors (press `C`)

### "Cannot find font" error

**Cause:** Windows font file not found

**Solution:**
The app looks for `C:\Windows\Fonts\segoeui.ttf`

```powershell
# Check if font exists
Test-Path "C:\Windows\Fonts\segoeui.ttf"

# If false, the font is missing (rare)
# Install Segoe UI font or change font in code
```

### Settings not saving

**Cause:** Permission issue or invalid JSON

**Solution:**
```powershell
# Check if config folder exists
Test-Path "config"

# Create if missing
New-Item -ItemType Directory -Path "config" -Force

# Check file permissions
Get-Acl "config\settings.json"

# Manually fix JSON if corrupted
notepad config\settings.json
```

---

## Performance Issues

### High CPU usage

**Cause 1 - Debug build:**
Debug builds are 10-50x slower!

**Solution:**
```powershell
# Always use Release build for testing performance
cmake --build build --config Release
.\build\Release\RTSPCameraViewer.exe
```

**Cause 2 - Too many cameras:**
Each camera adds CPU load

**Solution:**
- Reduce the number of active cameras
- Start with 1-2 cameras, add more gradually

**Cause 3 - Software rendering:**
SDL might be using software renderer

**Solution:**
```powershell
# Check SDL renderer info in logs
# Update graphics drivers
```

### Low FPS

**Cause:** VSync enabled or renderer overloaded

**Solution:**
1. Ensure Release build
2. Reduce camera count
3. Lower camera resolution in settings

### Memory leak

**Cause:** Resource not being freed

**Solution:**
Run Visual Studio Memory Profiler:
1. Debug → Performance Profiler
2. Select "Memory Usage"
3. Start profiling
4. Use application
5. Stop and analyze

---

## Development Issues

### IntelliSense not working (VSCode)

**Solution:**
1. Ctrl+Shift+P → "C/C++: Edit Configurations (JSON)"
2. Verify paths:
   ```json
   {
     "configurations": [{
       "name": "Win32",
       "includePath": [
         "${workspaceFolder}/**",
         "C:/vcpkg/installed/x64-windows/include"
       ],
       "compilerPath": "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.XX/bin/Hostx64/x64/cl.exe"
     }]
   }
   ```
3. Reload window (Ctrl+Shift+P → "Reload Window")

### IntelliSense not working (Visual Studio)

**Solution:**
1. Close Visual Studio
2. Delete `.vs` folder in project root
3. Reopen solution
4. Project → Rescan Solution

### Cannot debug - "Debugger cannot connect"

**Solution:**
1. Ensure Debug build:
   ```powershell
   cmake --build build --config Debug
   ```
2. Check debug configuration in IDE
3. Run as administrator if needed

### Git merge conflicts in build files

**Solution:**
```bash
# Build files should be gitignored
# Never commit files in build/ folder

# To resolve:
git checkout --theirs build/  # or --ours
# Then rebuild
.\build.ps1
```

### Changes not taking effect

**Cause:** Using old build

**Solution:**
```powershell
# Clean rebuild
cmake --build build --config Release --clean-first

# Or full clean
Remove-Item build -Recurse -Force
.\build.ps1
```

---

## Getting More Help

If your issue isn't covered here:

1. **Check logs:**
   - Run app and press `C` to view console
   - Look for error messages

2. **Search issues:**
   - [Project Issues](https://github.com/UPRobotics/RRL-2026-CAMERAS/issues)
   - [vcpkg Issues](https://github.com/microsoft/vcpkg/issues)
   - [FFmpeg Issues](https://trac.ffmpeg.org/)

3. **Ask for help:**
   - Open a GitHub issue with:
     - System info (Windows version, CPU, GPU)
     - Build output
     - Error messages
     - Steps to reproduce

---

## Quick Fixes Summary

| Problem | Quick Fix |
|---------|-----------|
| vcpkg not found | `setx VCPKG_ROOT "C:\vcpkg"` + restart terminal |
| FFmpeg not found | `cd C:\vcpkg && .\vcpkg install ffmpeg:x64-windows` |
| SDL2.dll missing | `copy C:\vcpkg\installed\x64-windows\bin\SDL2.dll build\Release\` |
| Build hangs | Be patient, FFmpeg takes 20-30 minutes first time |
| High CPU | Use Release build, not Debug |
| Clean build needed | `Remove-Item build -Recurse -Force && .\build.ps1` |

---

Most issues are solved by:
1. Using Release build (not Debug)
2. Ensuring vcpkg is properly configured
3. Checking logs in the console window
4. Clean rebuild when in doubt
