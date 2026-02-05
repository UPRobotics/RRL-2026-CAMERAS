#include <iostream>
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>
#include "main_window.h"
#include "settings_manager.h"

extern "C" {
#include <libavutil/log.h>
}

// Global debug flag
bool g_debugMode = false;

// FFmpeg log callback for debug mode - show ALL messages
// Using fprintf directly to avoid potential spdlog threading issues
static void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list vl) {
    (void)ptr; // Unused parameter
    
    if (!g_debugMode) return;
    
    // Filter based on level - show warnings and above, plus info
    if (level > AV_LOG_VERBOSE) return;
    
    char buf[2048];
    vsnprintf(buf, sizeof(buf), fmt, vl);
    
    // Remove trailing newline
    size_t len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[--len] = '\0';
    }
    
    // Skip empty messages
    if (len == 0) return;
    
    // Use fprintf for thread safety (spdlog can crash from FFmpeg threads)
    const char* levelStr = "INFO";
    switch (level) {
        case AV_LOG_PANIC:
        case AV_LOG_FATAL:
            levelStr = "FATAL";
            break;
        case AV_LOG_ERROR:
            levelStr = "ERROR";
            break;
        case AV_LOG_WARNING:
            levelStr = "WARN";
            break;
        case AV_LOG_INFO:
            levelStr = "INFO";
            break;
        case AV_LOG_VERBOSE:
            levelStr = "VERBOSE";
            break;
        default:
            levelStr = "DEBUG";
            break;
    }
    
    fprintf(stderr, "[FFmpeg-%s] %s\n", levelStr, buf);
    fflush(stderr);
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            g_debugMode = true;
        }
    }
    
    // Set log level based on debug mode
    if (g_debugMode) {
        spdlog::set_level(spdlog::level::trace);
        // Set FFmpeg to VERBOSE level to see all connection and decoding details
        av_log_set_level(AV_LOG_VERBOSE);
        av_log_set_callback(ffmpegLogCallback);
        spdlog::info("Debug mode enabled - FFmpeg VERBOSE output will be shown");
    } else {
        spdlog::set_level(spdlog::level::info);
        av_log_set_level(AV_LOG_QUIET);
    }
    
    spdlog::info("=== C++ RTSP Camera Viewer ===");
    spdlog::info("Starting application...");
    
    // Load settings
    camera_viewer::SettingsManager::instance().load();
    
    // Create main window
    camera_viewer::MainWindow mainWindow("RTSP Camera Viewer - Multi-Camera Monitor", 1280, 720);
    
    // Initialize
    if (!mainWindow.initialize()) {
        spdlog::error("Failed to initialize main window");
        return 1;
    }
    
    // Run application
    mainWindow.run();
    
    // Cleanup
    mainWindow.shutdown();
    
    spdlog::info("Application terminated successfully");
    return 0;
}
