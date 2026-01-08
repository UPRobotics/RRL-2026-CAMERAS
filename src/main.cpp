#include <iostream>
#include <spdlog/spdlog.h>
#include "main_window.h"

int main(int argc, char* argv[]) {
    // Set log level to trace to capture all messages
    spdlog::set_level(spdlog::level::trace);
    
    spdlog::info("=== C++ RTSP Camera Viewer ===");
    spdlog::info("Starting application...");
    
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
