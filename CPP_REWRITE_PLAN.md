# C++ RTSP Multi-Camera Viewer - Rewrite Plan

## 📊 Current Python Application Analysis

### Architecture Overview
Your Python application is a sophisticated **multi-threaded RTSP stream viewer** with the following key characteristics:

**Core Features:**
- **Multi-camera support**: Scalable from 1 to 8+ cameras with dynamic grid layout
- **Ultra-low latency**: Aggressive frame dropping, minimal buffering (1-frame queue)
- **Dual-stream switching**: Runtime toggle between high-res (1080p) and low-res (480p)
- **Hardware acceleration**: Auto-detection and support for VAAPI, NVENC, QSV, CUDA
- **Persistent settings**: Camera rotation, quality preferences saved to JSON
- **Robust error handling**: Auto-reconnection, health monitoring, progressive backoff
- **Dual view modes**: Grid view (all cameras) and fullscreen (single camera)
- **Advanced UI**: OpenCV-based with overlays, statistics, and keyboard controls

**Technical Stack (Python):**
- **FFmpeg** (via ffmpeg-python): RTSP stream decoding and processing
- **OpenCV**: Display rendering and frame manipulation
- **Threading**: One thread per camera for concurrent stream processing
- **Queue**: Thread-safe frame buffering with aggressive dropping
- **ConfigParser**: INI-based configuration
- **JSON**: Persistent state storage

**Performance Optimizations:**
- TCP transport for reliable RTSP connections
- Minimal probe size (32KB) and analysis duration (1s)
- Zero-buffering flags (`fflags=nobuffer`, `max_delay=0`)
- Hardware-accelerated scaling (VAAPI/CUDA)
- Pre-allocated frame buffers
- Aggressive frame queue management

---

## 🎯 C++ Rewrite Advantages

### Why C++ Will Be Faster

1. **No Python GIL**: True parallel processing across all camera threads
2. **Native FFmpeg API**: Direct access without Python wrapper overhead
3. **Zero-copy operations**: Direct memory mapping and frame buffer reuse
4. **Better memory management**: Stack allocation, custom allocators, object pooling
5. **Compiler optimizations**: SIMD, vectorization, aggressive inlining
6. **Lower syscall overhead**: Direct system calls without interpreter layer
7. **Cache-friendly data structures**: Contiguous memory layouts
8. **GPU integration**: Direct CUDA/Vulkan/DirectX integration

**Expected Performance Gains:**
- **Latency reduction**: 30-50% lower end-to-end latency
- **CPU usage**: 40-60% less CPU per camera stream
- **Memory footprint**: 50-70% reduction in RAM usage
- **Frame processing**: 2-3x faster frame decode and display pipeline

---

## 🔧 Recommended Technology Stack

### 1. Core Libraries

#### **FFmpeg (libavformat, libavcodec, libavutil, libswscale)**
- **Purpose**: RTSP stream capture and H.264/H.265 decoding
- **Why**: Industry-standard, hardware acceleration support, mature C API
- **Features needed**:
  - `avformat_open_input()` for RTSP streams
  - `avcodec_find_decoder()` for hardware-accelerated decoders
  - `sws_scale()` for efficient frame resizing
  - Hardware decoding contexts (VAAPI, NVDEC, QSV, D3D11VA on Windows)

#### **OpenGL/GLFW or SDL2**
- **Purpose**: High-performance video rendering
- **Recommendations**:
  - **SDL2**: Simpler API, cross-platform, good for immediate-mode rendering
  - **OpenGL + GLFW**: More control, texture streaming, GPU-accelerated composition
  - **Windows-specific alternative**: Direct3D 11/12 for native Windows performance
- **Features needed**:
  - Multiple window/texture support for grid layout
  - VSync control for latency management
  - Keyboard input handling
  - Fullscreen toggle

#### **Threading Library: C++20 std::jthread + std::atomic**
- **Purpose**: Lock-free camera thread management
- **Why**: Modern C++ standard, better than pthread, RAII semantics
- **Alternatives**: Intel TBB for advanced task scheduling

#### **Lock-Free Queue: Boost.Lockfree or moodycamel::ConcurrentQueue**
- **Purpose**: Ultra-fast inter-thread frame passing
- **Why**: Zero-contention, wait-free operations, ~10x faster than std::mutex
- **Features**: SPSC (single-producer-single-consumer) queues per camera

### 2. Configuration & State Management

#### **INI Parser: inih or toml++**
- **Purpose**: Configuration file parsing
- **Why**: Lightweight, header-only options available
- **Alternative**: YAML with yaml-cpp for more complex configs

#### **JSON: nlohmann/json**
- **Purpose**: Persistent state (rotations, quality settings)
- **Why**: Modern C++, single-header, easy API

### 3. Platform-Specific Optimizations

#### **Windows (Your Platform)**
- **Hardware acceleration**: 
  - D3D11VA for Intel/AMD integrated graphics
  - NVDEC for NVIDIA GPUs via FFmpeg
  - Windows Media Foundation as fallback
- **Display**: Direct3D 11 texture streaming for lowest latency
- **Threading**: Windows thread priority boost for camera threads
- **Memory**: Large page allocation for frame buffers

### 4. Build System

#### **CMake**
- **Why**: Cross-platform, FFmpeg integration, modern C++ support
- **Features**: vcpkg integration for dependency management
- **Example targets**: Release with AVX2/SSE optimizations

---

## 🗺️ Development Roadmap

### **Phase 1: Foundation (Week 1-2)**

#### 1.1 Project Setup
- [ ] CMake build system with vcpkg for dependencies
- [ ] FFmpeg library integration (libav*)
- [ ] SDL2 or GLFW integration
- [ ] Basic logging system (spdlog recommended)
- [ ] Configuration parser (INI support)

#### 1.2 Single Camera Prototype
- [ ] RTSP connection with FFmpeg API
- [ ] H.264 software decoding
- [ ] Frame to RGB conversion (swscale)
- [ ] Single window rendering with SDL2/OpenGL
- [ ] Basic error handling

**Deliverable**: Single camera RTSP viewer with ~200ms latency

---

### **Phase 2: Multi-Threading & Performance (Week 3-4)**

#### 2.1 Camera Thread Architecture
- [ ] Camera class with dedicated thread
- [ ] Lock-free frame queue (moodycamel::ConcurrentQueue)
- [ ] Thread-safe statistics tracking
- [ ] Graceful start/stop with RAII

#### 2.2 Ultra-Low Latency Optimizations
- [ ] Pre-allocated frame buffer pool (object pooling)
- [ ] Aggressive frame dropping (queue flush on new frame)
- [ ] Zero-copy texture upload to GPU
- [ ] Minimal FFmpeg buffering flags
- [ ] TCP keep-alive for RTSP

**Deliverable**: 4-camera viewer with <100ms latency per stream

---

### **Phase 3: Display & UI (Week 5)**

#### 3.1 Grid Layout Manager
- [ ] Dynamic grid calculation (auto-scale for N cameras)
- [ ] OpenGL texture array or SDL2 multi-rendering
- [ ] Fullscreen toggle
- [ ] Camera navigation (arrow keys)

#### 3.2 Keyboard Controls
- [ ] ESC/Q: Quit
- [ ] SPACE: Toggle grid/fullscreen
- [ ] L: Switch stream quality
- [ ] R: Rotate camera
- [ ] S: Show statistics
- [ ] P: Restart cameras

**Deliverable**: Full UI with grid and fullscreen modes

---

### **Phase 4: Hardware Acceleration (Week 6)**

#### 4.1 Windows Hardware Decoding
- [ ] D3D11VA decoder context for Intel/AMD
- [ ] NVDEC (CUDA) decoder context for NVIDIA
- [ ] Automatic fallback to software decoding
- [ ] Hardware decoder capability detection

#### 4.2 GPU-Accelerated Rendering
- [ ] Direct3D 11 texture streaming (Windows)
- [ ] Hardware YUV to RGB conversion
- [ ] GPU-side frame scaling
- [ ] Minimal CPU-GPU data transfer

**Deliverable**: Hardware-accelerated pipeline with <50ms latency

---

### **Phase 5: Robustness & Features (Week 7-8)**

#### 5.1 Error Handling & Recovery
- [ ] Auto-reconnection with exponential backoff
- [ ] Health monitoring per camera
- [ ] Connection state machine
- [ ] Detailed error logging
- [ ] Network timeout handling

#### 5.2 Dual-Stream Support
- [ ] High-res and low-res URL configuration
- [ ] Runtime stream switching (L key)
- [ ] Seamless reconnection
- [ ] Quality indicator overlay

#### 5.3 Persistent State
- [ ] JSON state file (camera rotations)
- [ ] Quality preference saving
- [ ] Auto-load on startup

**Deliverable**: Production-ready application matching Python feature set

---

### **Phase 6: Advanced Optimizations (Week 9+)**

#### 6.1 Memory Optimization
- [ ] Custom allocator for frame buffers
- [ ] Memory pool for network buffers
- [ ] Stack allocation for small objects
- [ ] Profiling and leak detection

#### 6.2 Performance Tuning
- [ ] SIMD optimizations (AVX2) for frame processing
- [ ] Profile-guided optimization (PGO)
- [ ] Cache-line alignment for hot data
- [ ] Benchmark suite with latency measurements

#### 6.3 Polish
- [ ] FPS overlay (optional)
- [ ] Camera labels and status
- [ ] Configuration hot-reload
- [ ] Windows installer (optional)

**Deliverable**: Optimized, production-grade application

---

## 📐 Architecture Design

### Class Structure

```cpp
// Core camera class
class RTSPCamera {
public:
    RTSPCamera(std::string id, std::string url, Size targetSize);
    ~RTSPCamera();
    
    void start();
    void stop();
    void restart();
    bool getFrame(Frame& frame);  // Non-blocking
    bool isHealthy() const;
    CameraStats getStats() const;
    
private:
    void streamLoop();  // Runs in dedicated thread
    bool connectFFmpeg();
    bool processFrame(AVFrame* frame);
    
    std::string id_;
    std::string rtspUrl_;
    std::jthread thread_;
    std::atomic<bool> running_;
    
    // Lock-free frame queue
    moodycamel::ReaderWriterQueue<Frame> frameQueue_;
    
    // FFmpeg context
    AVFormatContext* formatCtx_;
    AVCodecContext* codecCtx_;
    SwsContext* swsCtx_;
    
    // Statistics
    std::atomic<uint64_t> totalFrames_;
    std::atomic<uint64_t> droppedFrames_;
    std::atomic<float> fps_;
};

// Display manager
class DisplayManager {
public:
    DisplayManager(const std::vector<RTSPCamera*>& cameras);
    void run();  // Main event loop
    
private:
    void renderGrid();
    void renderFullscreen(int cameraIndex);
    void handleKeyboard(int key);
    
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    std::vector<SDL_Texture*> textures_;
    ViewMode viewMode_;
    int currentCamera_;
};

// Configuration
class Config {
public:
    static Config& instance();
    void load(const std::string& path);
    
    std::vector<CameraConfig> getCameras() const;
    DisplayConfig getDisplayConfig() const;
    PerformanceConfig getPerformanceConfig() const;
    
private:
    // INI parsing
};

// Main application
class CameraApplication {
public:
    void run();
    
private:
    void initializeCameras();
    void mainLoop();
    void cleanup();
    
    std::vector<std::unique_ptr<RTSPCamera>> cameras_;
    std::unique_ptr<DisplayManager> display_;
    Config config_;
};
```

### Data Flow

```
[RTSP Stream] 
    ↓
[FFmpeg Decode Thread] → AVFrame
    ↓
[HW Accel Decode (optional)] → GPU Frame
    ↓
[Scale & Convert] → RGB Frame
    ↓
[Lock-Free Queue] (size=1, drop old)
    ↓
[Main Thread] → Get Latest Frame
    ↓
[GPU Texture Upload] → Zero-copy
    ↓
[OpenGL/D3D Render] → Display
```

---

## 🎯 Key Performance Targets

| Metric | Python (Current) | C++ (Target) | Improvement |
|--------|------------------|--------------|-------------|
| **Per-camera latency** | 100-200ms | <50ms | 50-75% |
| **CPU usage (4 cams)** | 40-60% | 15-25% | 60% |
| **Memory footprint** | 500-800MB | 150-300MB | 60% |
| **Startup time** | 3-5s | <1s | 70% |
| **Max cameras** | 8-10 | 16-32 | 200% |
| **Frame processing** | 30-60 FPS | 120+ FPS | 200% |

---

## 🛠️ Development Tools & Environment

### Required Tools
- **Compiler**: MSVC 2022 (Visual Studio) or Clang 15+ with C++20 support
- **Build system**: CMake 3.25+
- **Package manager**: vcpkg for dependency management
- **Debugger**: Visual Studio Debugger or lldb
- **Profiler**: Intel VTune, Visual Studio Profiler, or Tracy Profiler
- **Version control**: Git with LFS for large binaries

### Recommended IDEs
- **Visual Studio 2022**: Best Windows integration, excellent debugger
- **CLion**: Cross-platform, CMake integration
- **VSCode**: Lightweight, with C++ extensions

### Dependencies (vcpkg)
```bash
vcpkg install ffmpeg[core,avcodec,avformat,swscale,avutil]:x64-windows
vcpkg install sdl2:x64-windows
vcpkg install nlohmann-json:x64-windows
vcpkg install spdlog:x64-windows
vcpkg install concurrentqueue:x64-windows
```

---

## 📝 Implementation Notes

### Critical Optimizations

1. **Frame Queue Management**
   ```cpp
   // Aggressive frame dropping
   while (!frameQueue_.empty()) {
       Frame oldFrame;
       frameQueue_.try_dequeue(oldFrame);  // Drop old frames
       ++droppedFrames_;
   }
   frameQueue_.enqueue(std::move(newFrame));  // Add latest
   ```

2. **Hardware Decoder Setup**
   ```cpp
   // NVDEC example
   AVBufferRef* hwDeviceCtx = nullptr;
   av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
   codecCtx->hw_device_ctx = av_buffer_ref(hwDeviceCtx);
   ```

3. **Zero-Copy Texture Upload**
   ```cpp
   // OpenGL PBO (Pixel Buffer Object) for async upload
   glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
   glBufferData(GL_PIXEL_UNPACK_BUFFER, frameSize, nullptr, GL_STREAM_DRAW);
   void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
   memcpy(ptr, frameData, frameSize);
   glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);
   ```

### FFmpeg Configuration for Ultra-Low Latency
```cpp
AVDictionary* options = nullptr;
av_dict_set(&options, "rtsp_transport", "tcp", 0);
av_dict_set(&options, "fflags", "nobuffer", 0);
av_dict_set(&options, "max_delay", "0", 0);
av_dict_set(&options, "probesize", "32768", 0);      // 32KB
av_dict_set(&options, "analyzeduration", "1000000", 0);  // 1s
av_dict_set(&options, "buffer_size", "32768", 0);
```

---

## 🎓 Learning Resources

### FFmpeg C API
- **FFmpeg Documentation**: https://ffmpeg.org/doxygen/trunk/
- **Hardware Acceleration**: https://trac.ffmpeg.org/wiki/HWAccelIntro
- **Example Code**: FFmpeg source tree `doc/examples/`

### Modern C++ Concurrency
- **C++20 std::jthread**: https://en.cppreference.com/w/cpp/thread/jthread
- **Lock-free Programming**: https://preshing.com/20120612/an-introduction-to-lock-free-programming/

### OpenGL/SDL2 Rendering
- **SDL2 Tutorial**: https://lazyfoo.net/tutorials/SDL/
- **OpenGL Texture Streaming**: https://www.khronos.org/opengl/wiki/Pixel_Buffer_Object

---

## ✅ Success Criteria

The C++ rewrite will be considered successful when:

- ✅ All Python application features are replicated
- ✅ Latency is <50ms per camera (measured from network to display)
- ✅ CPU usage is <25% for 4 cameras (on modern CPU)
- ✅ Supports 8+ cameras simultaneously without frame drops
- ✅ Hardware acceleration works on Intel/AMD/NVIDIA
- ✅ Stable operation for 24+ hours without memory leaks
- ✅ Code is maintainable with modern C++ practices

---

## 🚀 Getting Started

### Immediate Next Steps

1. **Set up development environment**
   - Install Visual Studio 2022 with C++ workload
   - Install CMake and vcpkg
   - Clone a starter template

2. **Proof of concept (Day 1-2)**
   - Single RTSP stream with FFmpeg
   - Decode to frame buffer
   - Display in SDL2 window
   - Measure baseline latency

3. **Iterate rapidly**
   - Add second camera with threading
   - Implement frame queue
   - Profile and optimize
   - Compare with Python version

**Would you like me to generate starter code for any of these phases?**

---

*This document will be updated as development progresses. Track changes in git history.*
