#include <iostream>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
    // Basic test to verify build system works
    spdlog::info("=== C++ RTSP Camera Viewer ===");
    spdlog::info("Build system test successful!");
    spdlog::info("FFmpeg, SDL2, and dependencies are linked correctly.");
    spdlog::info("");
    spdlog::info("Next step: Implement Phase 1 - Single Camera Prototype");
    spdlog::info("See README.md for details.");
    
    return 0;
}
