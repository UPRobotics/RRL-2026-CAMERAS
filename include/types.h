#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace rtsp_viewer {

// Basic types
using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

// Frame data structure
struct Frame {
    uint8_t* data = nullptr;
    int width = 0;
    int height = 0;
    int linesize = 0;
    TimePoint timestamp;
    
    // Disable copy, enable move
    Frame() = default;
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    Frame(Frame&&) noexcept = default;
    Frame& operator=(Frame&&) noexcept = default;
};

// Camera statistics
struct CameraStats {
    uint64_t total_frames = 0;
    uint64_t dropped_frames = 0;
    uint32_t current_fps = 0;
    uint32_t connection_attempts = 0;
    uint32_t successful_connections = 0;
    TimePoint last_frame_time;
    bool is_connected = false;
};

// Display view modes
enum class ViewMode {
    Grid,
    Fullscreen
};

// Stream quality
enum class StreamQuality {
    HighRes,
    LowRes
};

// Hardware acceleration type
enum class HWAccelType {
    None,
    D3D11VA,
    NVDEC,
    DXVA2,
    Auto
};

// Camera configuration
struct CameraConfig {
    std::string id;
    std::string url_highres;
    std::string url_lowres;
    int target_width = 640;
    int target_height = 360;
    StreamQuality initial_quality = StreamQuality::HighRes;
};

} // namespace rtsp_viewer
