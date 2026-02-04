# Architecture Guide

## System Overview

The RTSP Camera Viewer is built with a modular, component-based architecture designed for maintainability, performance, and extensibility.

## High-Level Architecture

```
┌──────────────────────────────────────────────────────┐
│                    Application                        │
│                      (main.cpp)                       │
│  - Process management                                 │
│  - Lifecycle control                                  │
└──────────────────────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────┐
│                   MainWindow                          │
│  - SDL2 window & renderer                             │
│  - Event handling (mouse, keyboard)                   │
│  - UI layout and component management                 │
│  - View mode control                                  │
└──────────────────────────────────────────────────────┘
           │
    ┌──────┴──────┬──────────────┬─────────────┐
    ▼             ▼              ▼             ▼
┌────────┐  ┌──────────┐  ┌──────────┐  ┌────────────┐
│Toolbar │  │  Camera  │  │  Stats   │  │  Console   │
│        │  │   Grid   │  │  Panel   │  │  Window    │
└────────┘  └──────────┘  └──────────┘  └────────────┘
                   │
       ┌───────────┼───────────┐
       ▼           ▼           ▼
   Camera 1    Camera 2    Camera N
```

## Core Components

### 1. MainWindow (`main_window.h/cpp`)

**Responsibilities:**
- Main application window management
- SDL2 renderer lifecycle
- Event handling (mouse, keyboard, resize)
- UI component orchestration
- View mode management

**Key Methods:**
- `initialize()` - Sets up SDL2, creates UI components
- `run()` - Main event loop
- `render()` - Renders all UI components
- `handleEvents()` - Processes SDL2 events
- `shutdown()` - Cleanup

**Event Flow:**
```
SDL Event → handleEvents() → 
  ├─ Mouse Click → handleMouseClick()
  ├─ Key Press → handleKeyPress()
  └─ Window Resize → handleResize()
```

### 2. CameraGrid (`camera_grid.h/cpp`)

**Responsibilities:**
- Camera layout calculation
- Camera slot rendering
- Grid/fullscreen view modes
- Camera selection and navigation

**View Modes:**
1. **Fullscreen** - Single camera fills entire area
2. **Grid 2x2** - Fixed 2x2 layout (4 cameras)
3. **Grid NxN** - Auto-calculated based on camera count

**Layout Algorithm:**
```cpp
// For N cameras, calculate optimal grid
cols = ceil(sqrt(N))
rows = ceil(N / cols)
```

### 3. ConsoleWindow (`console_window.h/cpp`)

**Responsibilities:**
- Debug log display
- Scrollable text window
- Log filtering and formatting
- Custom spdlog sink integration

**Features:**
- Circular buffer for log storage
- Automatic scrolling
- Color-coded log levels
- Resizable window

### 4. StatsPanel (`stats_panel.h/cpp`)

**Responsibilities:**
- Real-time statistics display
- FPS counters
- Connection status
- System metrics

**Displayed Info:**
- Active camera count
- FPS per camera
- Network status
- CPU/Memory usage (future)

### 5. SettingsManager (`settings_manager.h/cpp`)

**Responsibilities:**
- JSON settings persistence
- Keybinding management
- Window position/size memory

**Settings Categories:**
- `display` - Window and fullscreen settings
- `keybindings` - Keyboard shortcuts
- `console` - Console window preferences

## Threading Model

Currently single-threaded with plans for:

```
Main Thread (UI)
  └─ SDL2 Event Loop
      └─ Render Loop (60 FPS)

[Future] Camera Threads
  ├─ Camera 1 Thread (RTSP decode)
  ├─ Camera 2 Thread (RTSP decode)
  └─ Camera N Thread (RTSP decode)
```

**Design Decisions:**
- UI runs on main thread for simplicity
- Each camera will have dedicated thread for RTSP
- Lock-free queues for frame passing
- Single-frame latency priority

## Memory Management

- **Smart Pointers**: `std::unique_ptr` for owned resources
- **RAII**: SDL2 resources managed with RAII wrappers
- **No Manual Memory**: All allocations use smart pointers or containers

**Resource Lifecycle:**
```
MainWindow::initialize()
  └─ Create components (unique_ptr)
      └─ Initialize SDL resources
          
MainWindow::shutdown()
  └─ Reset unique_ptrs (auto-cleanup)
      └─ SDL resources freed via RAII
```

## Error Handling

**Strategy:**
1. **Validation** - Early parameter checking
2. **Logging** - spdlog for all errors
3. **Graceful Degradation** - Continue with reduced functionality
4. **User Feedback** - Display errors in console

**Example:**
```cpp
if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::error("SDL Init failed: {}", SDL_GetError());
    return false;  // Fail initialization
}
```

## Rendering Pipeline

```
MainWindow::render()
  ├─ Clear Screen
  ├─ renderToolbar()
  │   └─ Draw buttons with hover states
  ├─ renderMainArea()
  │   └─ CameraGrid::render()
  │       └─ For each camera slot:
  │           ├─ Draw background
  │           ├─ Draw placeholder grid
  │           └─ Draw camera info overlay
  └─ renderStatsBar()
      └─ StatsPanel::render()
          └─ Draw stats text
```

**Frame Rate:**
- Target: 60 FPS UI
- Actual: Limited by SDL_Delay in main loop
- Camera streams: Independent of UI (30 FPS typical)

## Configuration System

**File:** `config/settings.json`

**Loading:**
```
SettingsManager::instance().load()
  └─ Parse JSON
      ├─ Validate keys
      ├─ Load with defaults for missing keys
      └─ Store in member variables
```

**Saving:**
```
SettingsManager::instance().save()
  └─ Serialize to JSON
      └─ Write to file
```

**Access:**
```cpp
// Get setting
auto width = SettingsManager::instance().getLastWindowWidth();

// Set setting
SettingsManager::instance().setLastWindowSize(1280, 720);
SettingsManager::instance().save();
```

## Future: RTSP Streaming

**Planned Architecture:**
```
CameraStream (per camera)
  └─ Decoder Thread
      ├─ FFmpeg RTSP connection
      ├─ Hardware-accelerated decode
      ├─ Frame queue (size=1)
      └─ Frame ready callback
      
CameraGrid
  └─ Render latest frame from each CameraStream
```

**Frame Flow:**
```
RTSP Stream → FFmpeg Decode → Frame Queue → Texture Upload → Render
```

## UI Layout

**Fixed Layout:**
```
┌───────────────────────────────────────┐
│ Toolbar (50px)                        │ 
│ [Start] [Stop] [Restart] [Console]   │
├───────────────────────────────────────┤
│                                        │
│                                        │
│         Main Camera Area               │
│         (dynamic height)               │
│                                        │
│                                        │
├───────────────────────────────────────┤
│ Stats Bar (60px)                      │
│ FPS: 30 | Cameras: 4/8 | Status: OK  │
└───────────────────────────────────────┘
```

**Console Window (separate):**
- Floating window
- Position/size remembered
- Can be toggled with 'C' key

## Extension Points

Areas designed for future extension:

1. **Camera Providers** - Add new stream sources
2. **Codecs** - Support H.265, AV1, etc.
3. **Hardware Decoders** - Add more HW accel backends
4. **View Modes** - Custom layouts, PIP, etc.
5. **UI Themes** - Color schemes and styles
6. **Recording** - Save streams to disk
7. **OSD Overlays** - Custom overlays on camera feeds

## Dependencies

**Direct Dependencies:**
- SDL2 2.30+ - Windowing and rendering
- SDL2_ttf - Text rendering
- FFmpeg (libav*) - RTSP and video decoding
- spdlog - Logging
- nlohmann/json - JSON parsing

**Build Dependencies:**
- CMake 3.25+ - Build system
- vcpkg - Package management
- Visual Studio 2022 - C++ compiler

## Performance Considerations

**Current:**
- UI runs at 60 FPS
- Minimal CPU usage (~5%) without streams
- ~100MB memory footprint

**With Streaming (planned):**
- Target: <50% CPU with 4 cameras (HW decode)
- Target: <500MB RAM with 8 cameras
- Target: <100ms latency per stream

**Optimization Strategies:**
1. Hardware-accelerated decoding
2. Single-frame buffering
3. Lock-free queues
4. Efficient texture updates
5. Conditional rendering (only visible cameras)

## Code Style

**Conventions:**
- Classes: PascalCase (`MainWindow`)
- Methods: camelCase (`handleEvents`)
- Members: m_prefix (`m_window`)
- Constants: UPPER_CASE (`TOOLBAR_HEIGHT`)
- Namespaces: snake_case (`camera_viewer`)

**Patterns:**
- Singleton: SettingsManager, FontManager
- RAII: All resource management
- Factory: Future for camera creation
- Observer: Future for event system

## Testing Strategy

**Current:**
- Manual testing with real RTSP cameras
- Visual verification of UI

**Future:**
- Unit tests for components
- Integration tests for RTSP
- Performance benchmarks
- Stress tests with many cameras

---

This architecture provides a solid foundation for the multi-camera RTSP viewer with clear separation of concerns and room for future enhancements.
