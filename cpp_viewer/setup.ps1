# PowerShell setup script for C++ RTSP Camera Viewer
# Automates vcpkg and CMake setup on Windows

param(
    [switch]$SkipVcpkg = $false,
    [switch]$SkipBuild = $false
)

$ErrorActionPreference = "Stop"

Write-Host "=== C++ RTSP Camera Viewer Setup ===" -ForegroundColor Cyan
Write-Host ""

# Check for required tools
function Test-Command {
    param($Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    } catch {
        return $false
    }
}

# Check CMake
if (-not (Test-Command "cmake")) {
    Write-Host "[ERROR] CMake not found. Please install CMake 3.25 or later." -ForegroundColor Red
    Write-Host "Download from: https://cmake.org/download/" -ForegroundColor Yellow
    exit 1
}
Write-Host "[OK] CMake found: $(cmake --version | Select-Object -First 1)" -ForegroundColor Green

# Check Git
if (-not (Test-Command "git")) {
    Write-Host "[ERROR] Git not found. Please install Git." -ForegroundColor Red
    Write-Host "Download from: https://git-scm.com/download/win" -ForegroundColor Yellow
    exit 1
}
Write-Host "[OK] Git found" -ForegroundColor Green

# Check Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "[OK] Visual Studio found at: $vsPath" -ForegroundColor Green
    }
} else {
    Write-Host "[WARNING] Visual Studio not detected. Make sure you have VS 2019 or later with C++ workload." -ForegroundColor Yellow
}

Write-Host ""

# Setup vcpkg
if (-not $SkipVcpkg) {
    Write-Host "=== Setting up vcpkg ===" -ForegroundColor Cyan
    
    $vcpkgRoot = $env:VCPKG_ROOT
    
    if (-not $vcpkgRoot) {
        Write-Host "VCPKG_ROOT not set. Where would you like to install vcpkg?" -ForegroundColor Yellow
        Write-Host "1. C:\vcpkg (recommended)" -ForegroundColor White
        Write-Host "2. Custom path" -ForegroundColor White
        Write-Host "3. Skip vcpkg setup (I already have it configured)" -ForegroundColor White
        
        $choice = Read-Host "Enter choice (1-3)"
        
        switch ($choice) {
            "1" {
                $vcpkgRoot = "C:\vcpkg"
            }
            "2" {
                $vcpkgRoot = Read-Host "Enter vcpkg installation path"
            }
            "3" {
                Write-Host "Skipping vcpkg setup." -ForegroundColor Yellow
                $vcpkgRoot = $null
            }
            default {
                Write-Host "[ERROR] Invalid choice." -ForegroundColor Red
                exit 1
            }
        }
    }
    
    if ($vcpkgRoot) {
        # Install or update vcpkg
        if (-not (Test-Path $vcpkgRoot)) {
            Write-Host "Installing vcpkg to: $vcpkgRoot" -ForegroundColor Yellow
            git clone https://github.com/microsoft/vcpkg.git $vcpkgRoot
            & "$vcpkgRoot\bootstrap-vcpkg.bat"
        } else {
            Write-Host "vcpkg already exists at: $vcpkgRoot" -ForegroundColor Green
            Write-Host "Updating vcpkg..." -ForegroundColor Yellow
            Push-Location $vcpkgRoot
            git pull
            Pop-Location
        }
        
        # Set environment variable
        [System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", $vcpkgRoot, [System.EnvironmentVariableTarget]::User)
        $env:VCPKG_ROOT = $vcpkgRoot
        
        Write-Host "[OK] vcpkg configured at: $vcpkgRoot" -ForegroundColor Green
        
        # Install dependencies
        Write-Host ""
        Write-Host "Installing dependencies via vcpkg (this may take 10-30 minutes)..." -ForegroundColor Yellow
        Write-Host "Dependencies: FFmpeg, SDL2, spdlog, nlohmann-json, concurrentqueue" -ForegroundColor White
        
        $vcpkgExe = Join-Path $vcpkgRoot "vcpkg.exe"
        
        & $vcpkgExe install --triplet=x64-windows `
            "ffmpeg[core,avcodec,avformat,avutil,swscale,swresample]:x64-windows" `
            "sdl2:x64-windows" `
            "spdlog:x64-windows" `
            "nlohmann-json:x64-windows" `
            "concurrentqueue:x64-windows"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[OK] All dependencies installed successfully" -ForegroundColor Green
        } else {
            Write-Host "[ERROR] vcpkg installation failed" -ForegroundColor Red
            exit 1
        }
        
        # Integrate with Visual Studio
        Write-Host "Integrating vcpkg with Visual Studio..." -ForegroundColor Yellow
        & $vcpkgExe integrate install
        
        Write-Host "[OK] vcpkg integrated with Visual Studio" -ForegroundColor Green
    }
}

Write-Host ""

# Create build directory and configure
if (-not $SkipBuild) {
    Write-Host "=== Configuring CMake ===" -ForegroundColor Cyan
    
    $buildDir = "build"
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }
    
    Push-Location $buildDir
    
    Write-Host "Running CMake configuration..." -ForegroundColor Yellow
    cmake .. -G "Visual Studio 17 2022" -A x64
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[OK] CMake configuration successful" -ForegroundColor Green
        Write-Host ""
        Write-Host "Build directory created at: $PWD" -ForegroundColor White
        Write-Host ""
        Write-Host "To build the project:" -ForegroundColor Yellow
        Write-Host "  cmake --build . --config Release" -ForegroundColor White
        Write-Host ""
        Write-Host "Or open the solution in Visual Studio:" -ForegroundColor Yellow
        Write-Host "  .\RTSPCameraViewer.sln" -ForegroundColor White
    } else {
        Write-Host "[ERROR] CMake configuration failed" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    Pop-Location
}

Write-Host ""
Write-Host "=== Setup Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Review config.ini and adjust camera URLs" -ForegroundColor White
Write-Host "2. Build the project: cd build && cmake --build . --config Release" -ForegroundColor White
Write-Host "3. Run the executable: .\build\Release\RTSPCameraViewer.exe" -ForegroundColor White
Write-Host ""
