# Complete Setup Guide

## Step-by-Step Instructions for Windows

### Step 1: Install Prerequisites

#### 1.1 Visual Studio 2022
1. Download Visual Studio 2022 Community (free):
   - https://visualstudio.microsoft.com/downloads/
2. During installation, select **"Desktop development with C++"** workload
3. Recommended components:
   - MSVC v143 compiler
   - Windows 10/11 SDK
   - CMake tools for Windows
   - C++ profiling tools (optional)

#### 1.2 CMake
1. Download CMake Windows installer:
   - https://cmake.org/download/
   - Get the "Windows x64 Installer"
2. During installation:
   - ✅ Check "Add CMake to system PATH"
3. Verify installation:
   ```powershell
   cmake --version
   # Should show 3.25 or later
   ```

#### 1.3 Git
1. Download Git for Windows:
   - https://git-scm.com/download/win
2. Use default installation options
3. Verify:
   ```powershell
   git --version
   ```

### Step 2: Run Automated Setup

1. **Open PowerShell** (Right-click Start → Windows PowerShell)

2. **Navigate to cpp_viewer directory**:
   ```powershell
   cd "C:\Users\Chumbi\OneDrive - up.edu.mx\Documents\GitHub\RRL-2026-CAMERAS\cpp_viewer"
   ```

3. **Enable script execution** (if needed):
   ```powershell
   Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned
   # Type 'Y' and press Enter
   ```

4. **Run setup script**:
   ```powershell
   .\setup.ps1
   ```

5. **Follow prompts**:
   - Choose vcpkg installation location (recommended: C:\vcpkg)
   - Wait for dependencies to download and compile (~15-30 minutes)
   - Script will configure CMake automatically

### Step 3: Verify Setup

After setup completes, verify everything is working:

```powershell
# Navigate to build directory
cd build

# Build the test application
cmake --build . --config Release

# Run the test
.\Release\RTSPCameraViewer.exe
```

You should see:
```
[info] === C++ RTSP Camera Viewer ===
[info] Build system test successful!
[info] FFmpeg, SDL2, and dependencies are linked correctly.
```

### Step 4: IDE Setup (Optional)

#### Visual Studio 2022

1. Open Visual Studio 2022
2. File → Open → Folder
3. Select the `cpp_viewer` folder
4. Visual Studio will automatically detect CMakeLists.txt
5. Select "x64-Release" configuration in the toolbar
6. Build → Build All (Ctrl+Shift+B)

#### VSCode

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)
2. Open `cpp_viewer` folder
3. Press Ctrl+Shift+P → "CMake: Configure"
4. Select "Visual Studio 2022 Release - amd64"
5. Press F7 to build

## Troubleshooting

### Issue: "vcpkg.exe not found"

**Solution 1:** Set VCPKG_ROOT environment variable
```powershell
setx VCPKG_ROOT "C:\vcpkg"
# Close and reopen PowerShell
```

**Solution 2:** Manually specify in CMake
```powershell
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

### Issue: "FFmpeg packages failed to build"

**Solution:** Try rebuilding specific package
```powershell
cd C:\vcpkg
.\vcpkg remove ffmpeg:x64-windows
.\vcpkg install ffmpeg[core,avcodec,avformat,avutil,swscale]:x64-windows
```

### Issue: "LINK: fatal error LNK1104: cannot open SDL2.dll"

**Solution:** Copy DLL manually
```powershell
copy "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" "build\Release\"
```

### Issue: Setup script hangs at "Installing dependencies"

**Cause:** FFmpeg compilation takes 15-30 minutes. This is normal.

**Check progress:**
```powershell
# In another PowerShell window
cd C:\vcpkg
Get-Process | Where-Object {$_.ProcessName -like "*cl*" -or $_.ProcessName -like "*link*"}
```

If processes are running, vcpkg is still compiling. Be patient.

### Issue: "Cannot find pkg-config"

**Solution:** Install pkg-config via vcpkg
```powershell
cd C:\vcpkg
.\vcpkg install pkgconf:x64-windows
```

Add to PATH:
```powershell
$vcpkgPath = "C:\vcpkg\installed\x64-windows\tools\pkgconf"
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";$vcpkgPath", [EnvironmentVariableTarget]::User)
```

## Manual Dependency Installation

If automated setup fails, install dependencies manually:

```powershell
cd C:\vcpkg

# Core dependencies
.\vcpkg install ffmpeg[core]:x64-windows
.\vcpkg install ffmpeg[avcodec]:x64-windows
.\vcpkg install ffmpeg[avformat]:x64-windows
.\vcpkg install ffmpeg[avutil]:x64-windows
.\vcpkg install ffmpeg[swscale]:x64-windows
.\vcpkg install ffmpeg[swresample]:x64-windows
.\vcpkg install sdl2:x64-windows
.\vcpkg install spdlog:x64-windows
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install concurrentqueue:x64-windows

# Verify installation
.\vcpkg list
```

## Testing Individual Components

### Test FFmpeg

Create `test_ffmpeg.cpp`:
```cpp
extern "C" {
#include <libavformat/avformat.h>
}
#include <iostream>

int main() {
    std::cout << "FFmpeg version: " << av_version_info() << std::endl;
    return 0;
}
```

Compile:
```powershell
cl /I "C:\vcpkg\installed\x64-windows\include" test_ffmpeg.cpp ^
   /link "C:\vcpkg\installed\x64-windows\lib\avformat.lib"
```

### Test SDL2

Create `test_sdl.cpp`:
```cpp
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "SDL2 initialized successfully!" << std::endl;
    SDL_Quit();
    return 0;
}
```

## Performance Verification

After successful setup, benchmark the build:

```powershell
cd build

# Clean build
cmake --build . --config Release --clean-first

# Time the compilation
Measure-Command { cmake --build . --config Release }
```

Typical build times:
- **Initial build**: 30-60 seconds
- **Incremental build**: 5-15 seconds

## Next Steps

Once setup is complete:

1. ✅ Verify build works: `.\Release\RTSPCameraViewer.exe`
2. 📝 Edit `config.ini` with your camera URLs
3. 🚀 Start implementing Phase 1 (see CPP_REWRITE_PLAN.md)

## Getting Help

If you encounter issues:

1. Check this guide first
2. Review error messages carefully
3. Search vcpkg issues: https://github.com/microsoft/vcpkg/issues
4. Check FFmpeg documentation: https://ffmpeg.org/documentation.html

## Estimated Time

- **Automated setup**: 30-45 minutes (mostly waiting for compilation)
- **Manual setup**: 1-2 hours (if troubleshooting needed)
- **First successful build**: Priceless! 🎉
