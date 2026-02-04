# Development Guide

## Getting Started

### Development Environment Setup

1. **Clone the repository**
    ```bash
   git clone https://github.com/UPRobotics/RRL-2026-CAMERAS.git
   cd RRL-2026-CAMERAS
   ```

2. **Run initial build**
    ```bash
    ./build.sh
   ```

3. **Open in your preferred IDE**
    - **VSCode**: Open folder and use CMake Tools extension

## Building

### Command Line

```bash
# Release build (optimized, use this for testing)
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Debug build (with symbols, for development)
cd ..
mkdir -p build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### VSCode

1. Open Command Palette (`Ctrl+Shift+P`)
2. "CMake: Select Configure Preset" → Choose preset
3. "CMake: Build" (`F7`)

## Project Structure

```
RRL-2026-CAMERAS/
├── CMakeLists.txt              # Build configuration
├── vcpkg.json                  # Dependencies manifest
├── build.sh                    # Automated build script
│
├── include/                    # Public header files
│   ├── main_window.h          # Main window class
│   ├── camera_grid.h          # Camera layout component
│   ├── console_window.h       # Debug console
│   ├── stats_panel.h          # Statistics panel
│   ├── settings_manager.h     # Settings management
│   ├── console_sink.h         # Custom spdlog sink
│   ├── ui_helpers.h           # UI utilities
│   └── types.h                # Common types and structs
│
├── src/                        # Implementation files
│   ├── main.cpp               # Entry point
│   ├── main_window.cpp
│   ├── camera_grid.cpp
│   ├── console_window.cpp
│   ├── stats_panel.cpp
│   ├── settings_manager.cpp
│   ├── console_sink.cpp
│   └── ui_helpers.cpp
│
├── config/                     # Configuration files
│   └── settings.json          # App settings (generated)
│
└── docs/                       # Documentation
    ├── ARCHITECTURE.md        # System design
    ├── BUILDING.md            # Build guide
    ├── DEVELOPMENT.md         # This file
    └── TROUBLESHOOTING.md     # Common issues
```

## Code Style

### Naming Conventions

```cpp
// Classes: PascalCase
class MainWindow { ... };
class CameraGrid { ... };

// Methods: camelCase
void handleEvents();
void renderToolbar();

// Member variables: m_ prefix + camelCase
int m_windowWidth;
SDL_Window* m_window;
std::vector<ButtonRect> m_toolbarButtons;

// Constants: UPPER_SNAKE_CASE
static constexpr int TOOLBAR_HEIGHT = 50;
static constexpr int BUTTON_WIDTH = 120;

// Namespaces: snake_case
namespace camera_viewer { ... }

// Local variables: camelCase
int cameraCount = 4;
bool isRunning = true;
```

### File Organization

**Header files (`.h`):**
```cpp
#pragma once

#include <system_headers>
#include "local_headers.h"

namespace camera_viewer {

// Forward declarations
class SomeClass;

/**
 * @brief Brief description
 * 
 * Detailed description if needed
 */
class MyClass {
public:
    MyClass();
    ~MyClass();
    
    // Public methods
    
private:
    // Private methods
    
    // Member variables
};

} // namespace camera_viewer
```

**Implementation files (`.cpp`):**
```cpp
#include "my_class.h"
#include <system_headers>
#include "other_headers.h"

namespace camera_viewer {

MyClass::MyClass() {
    // Constructor
}

// Implementation...

} // namespace camera_viewer
```

### Comments

```cpp
// Use single-line comments for brief explanations
int count = 0;  // Current count

/**
 * @brief Use Doxygen-style comments for functions
 * @param renderer The SDL renderer
 * @param rect The rectangle to render
 * @return true if successful
 */
bool renderRect(SDL_Renderer* renderer, SDL_Rect rect);

// TODO: Explain what needs to be done
// FIXME: Describe bug that needs fixing
// NOTE: Important information
```

## Adding Features

### Adding a New UI Component

1. **Create header file** (`include/my_component.h`)
```cpp
#pragma once
#include <SDL2/SDL.h>

namespace camera_viewer {

class MyComponent {
public:
    MyComponent();
    ~MyComponent() = default;
    
    void render(SDL_Renderer* renderer, int x, int y, int width, int height);
    
private:
    // Component state
};

} // namespace camera_viewer
```

2. **Create implementation** (`src/my_component.cpp`)
```cpp
#include "my_component.h"
#include "ui_helpers.h"
#include <spdlog/spdlog.h>

namespace camera_viewer {

MyComponent::MyComponent() {
    spdlog::debug("MyComponent initialized");
}

void MyComponent::render(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Rendering code
}

} // namespace camera_viewer
```

3. **Add to CMakeLists.txt**
```cmake
set(SOURCES
    src/main.cpp
    src/my_component.cpp  # Add here
    # ...
)
```

4. **Integrate into MainWindow**
```cpp
// main_window.h
#include "my_component.h"
private:
    std::unique_ptr<MyComponent> m_myComponent;

// main_window.cpp
bool MainWindow::initialize() {
    // ...
    m_myComponent = std::make_unique<MyComponent>();
    return true;
}

void MainWindow::render() {
    // ...
    m_myComponent->render(m_renderer, x, y, width, height);
}
```

### Adding a Settings Option

1. **Add to `settings.json`**
```json
{
  "my_feature": {
    "enabled": true,
    "value": 42
  }
}
```

2. **Add getters/setters in `SettingsManager`**
```cpp
// settings_manager.h
bool isMyFeatureEnabled() const { return m_myFeatureEnabled; }
void setMyFeatureEnabled(bool enabled) { m_myFeatureEnabled = enabled; }

private:
    bool m_myFeatureEnabled = true;

// settings_manager.cpp
bool SettingsManager::load() {
    // ...
    if (j.contains("my_feature")) {
        m_myFeatureEnabled = j["my_feature"].value("enabled", true);
    }
}

bool SettingsManager::save() {
    // ...
    j["my_feature"] = {
        {"enabled", m_myFeatureEnabled}
    };
}
```

## Debugging

### Debug Build

```powershell
# Build debug version
cmake --build build --config Debug

# Run with debugger attached
# Visual Studio: F5
# VSCode: F5 with C++ debugger configured
```

### Logging

```cpp
#include <spdlog/spdlog.h>

// Different log levels
spdlog::trace("Very detailed info");
spdlog::debug("Debug information");
spdlog::info("General information");
spdlog::warn("Warning message");
spdlog::error("Error occurred: {}", error);
```

**Log levels:**
- `trace` - Most verbose
- `debug` - Development info
- `info` - General messages (default)
- `warn` - Warnings
- `error` - Errors

### Common Debug Scenarios

**Check SDL errors:**
```cpp
if (!surface) {
    spdlog::error("SDL Error: {}", SDL_GetError());
}
```

**Verify initialization:**
```cpp
spdlog::info("Component initialized: {}", m_isInitialized ? "yes" : "no");
```

**Track execution:**
```cpp
spdlog::debug("Entering function: handleEvents");
// ... code ...
spdlog::debug("Exiting function: handleEvents");
```

## Testing

### Manual Testing

1. **Build the application**
    ```bash
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)
    ```

2. **Run the application**
    ```bash
    ./RTSPCameraViewer
    ```

3. **Open console** (press `C` in app)

4. **Test features:**
   - Click buttons
   - Press keyboard shortcuts
   - Resize window
   - Check logs in console

## Performance Profiling

### Visual Studio Profiler

1. Debug → Performance Profiler
2. Select "CPU Usage"
3. Start profiling
4. Perform actions in app
5. Stop profiling
6. Analyze hot paths

### Manual Timing

```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();

// Code to measure

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
spdlog::info("Operation took: {}ms", duration.count());
```

## Common Tasks

### Adding a Keyboard Shortcut

1. **Add to `settings.json`**
```json
"keybindings": {
    "my_action": "M"
}
```

2. **Handle in `MainWindow::handleKeyPress`**
```cpp
void MainWindow::handleKeyPress(SDL_Keycode key) {
    auto& settings = SettingsManager::instance();
    
    if (settings.isKeyForAction(key, "my_action")) {
        onMyAction();
    }
}
```

### Adding a Toolbar Button

```cpp
// In MainWindow::initialize()
m_toolbarButtons.push_back({
    {x, y, BUTTON_WIDTH, BUTTON_HEIGHT},
    "My Button",
    false,  // not hovered
    true    // enabled
});

// In MainWindow::handleMouseClick()
if (m_toolbarButtons[index].enabled && /* click detected */) {
    onMyButtonClicked();
}
```

## Git Workflow

### Branching

```bash
# Create feature branch
git checkout -b feature/my-feature

# Make changes, commit often
git add .
git commit -m "Add my feature"

# Push to remote
git push origin feature/my-feature

# Create pull request on GitHub
```

### Commit Messages

Follow conventional commits:

```
type: brief description

Longer description if needed
- Bullet points for multiple changes
- Each change on new line

Fixes #123
```

**Types:**
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation
- `style:` - Code style (formatting)
- `refactor:` - Code refactoring
- `perf:` - Performance improvement
- `test:` - Tests
- `chore:` - Maintenance

**Examples:**
```
feat: add camera rotation support

fix: correct window resizing bug

docs: update architecture guide

refactor: extract UI helpers to separate file
```

## Troubleshooting Development Issues

### IntelliSense Not Working

**VSCode:**
1. Ctrl+Shift+P → "C/C++: Edit Configurations (JSON)"
2. Verify `compilerPath` and `includePath`
3. Reload window

**Visual Studio:**
1. Delete `.vs` folder
2. Reopen solution
3. Project → Rescan Solution

### Build Errors After Pulling

```powershell
# Clean and rebuild
cmake --build build --config Release --clean-first

# If that fails, delete build folder
Remove-Item build -Recurse -Force
.\build.ps1
```

### Link Errors

```powershell
# Verify vcpkg packages are installed
cd C:\vcpkg
.\vcpkg list

# Reinstall if needed
.\vcpkg remove <package>
.\vcpkg install <package>:x64-windows
```

## Resources

### Documentation
- [SDL2 Wiki](https://wiki.libsdl.org/)
- [FFmpeg API Docs](https://ffmpeg.org/doxygen/trunk/)
- [spdlog GitHub](https://github.com/gabime/spdlog)
- [nlohmann/json Docs](https://json.nlohmann.me/)

### Learning
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/)
- [Modern C++ Best Practices](https://github.com/cpp-best-practices/cppbestpractices)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)

### Tools
- [vcpkg](https://github.com/microsoft/vcpkg)
- [CMake](https://cmake.org/)
- [Visual Studio](https://visualstudio.microsoft.com/)

---

Happy coding! If you have questions, check the docs or open an issue.
