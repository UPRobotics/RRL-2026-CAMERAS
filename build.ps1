# Build script for RRL-2026-CAMERAS C++ RTSP Viewer
# This script sets up the Visual Studio environment and builds the project

param(
    [switch]$Clean,
    [switch]$Configure,
    [switch]$Rebuild,
    [string]$BuildType = "Release"
)

$ErrorActionPreference = "Stop"

# Colors for output
function Write-ColorOutput($ForegroundColor, $Message) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    Write-Output $Message
    $host.UI.RawUI.ForegroundColor = $fc
}

Write-ColorOutput Green "=== RRL-2026-CAMERAS Build Script ==="
Write-Output ""

# Set paths
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"
$VcpkgRoot = "C:\vcpkg"

# Check if Visual Studio DevShell module exists
$VSDevShellPath = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

if (-not (Test-Path $VSDevShellPath)) {
    Write-ColorOutput Red "Error: Visual Studio 2026 not found!"
    Write-Output "Please install Visual Studio 2026 with C++ development tools."
    exit 1
}

# Import Visual Studio Developer Shell
Import-Module $VSDevShellPath
Enter-VsDevShell -VsInstallPath 'C:\Program Files\Microsoft Visual Studio\18\Community' -Arch amd64 -SkipAutomaticLocation

# Set environment variables
$env:VCPKG_ROOT = $VcpkgRoot

Write-ColorOutput Cyan "Configuration:"
Write-Output "  Project Root: $ProjectRoot"
Write-Output "  Build Directory: $BuildDir"
Write-Output "  Build Type: $BuildType"
Write-Output "  vcpkg Root: $VcpkgRoot"
Write-Output ""

# Clean build directory if requested
if ($Clean -or $Rebuild) {
    Write-ColorOutput Yellow "Cleaning build directory..."
    if (Test-Path $BuildDir) {
        Remove-Item -Path "$BuildDir\*" -Recurse -Force -ErrorAction SilentlyContinue
        Write-ColorOutput Green "✓ Build directory cleaned"
    }
    Write-Output ""
}

# Configure CMake if requested or if build directory doesn't exist
if ($Configure -or $Rebuild -or -not (Test-Path (Join-Path $BuildDir "CMakeCache.txt"))) {
    Write-ColorOutput Yellow "Configuring CMake..."
    
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
    }
    
    $cmakeArgs = @(
        "-G", "NMake Makefiles",
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_TOOLCHAIN_FILE=$VcpkgRoot/scripts/buildsystems/vcpkg.cmake",
        "-S", $ProjectRoot,
        "-B", $BuildDir
    )
    
    & cmake @cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput Red "✗ CMake configuration failed!"
        exit $LASTEXITCODE
    }
    
    Write-ColorOutput Green "✓ CMake configured successfully"
    Write-Output ""
}

# Build the project
Write-ColorOutput Yellow "Building project..."
Push-Location $BuildDir

& nmake

$buildResult = $LASTEXITCODE
Pop-Location

if ($buildResult -eq 0) {
    Write-Output ""
    Write-ColorOutput Green "✓ Build successful!"
    Write-Output ""
    Write-ColorOutput Cyan "Executable: $BuildDir\RTSPCameraViewer.exe"
    Write-Output ""
    Write-Output "To run the application:"
    Write-ColorOutput White "  .\build\RTSPCameraViewer.exe"
    Write-Output ""
} else {
    Write-ColorOutput Red "✗ Build failed!"
    exit $buildResult
}
