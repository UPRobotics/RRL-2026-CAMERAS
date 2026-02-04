cmake_minimum_required(VERSION 3.25)
project(RTSPCameraViewer 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Ultra-low latency RTSP multi-camera viewer for Ubuntu 22.04"
)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# GCC compiler flags for Linux
add_compile_options(
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    $<$<CONFIG:Release>:-flto>
)
add_link_options(
    $<$<CONFIG:Release>:-flto>
)

# Find required packages using pkg-config
find_package(PkgConfig REQUIRED)

# FFmpeg libraries
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
    libswresample
)

# SDL2
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# SDL2_ttf for text rendering
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

# spdlog for logging (header-only or system library)
find_package(spdlog REQUIRED)

# nlohmann_json for configuration (header-only)
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS /usr/include /usr/local/include
    REQUIRED
)

# Concurrentqueue header-only library
find_path(CONCURRENTQUEUE_INCLUDE_DIR concurrentqueue/concurrentqueue.h
    PATHS /usr/include /usr/local/include
)

# Source files
set(SOURCES
    src/main.cpp
    src/main_window.cpp
    src/console_window.cpp
    src/console_sink.cpp
    src/settings_manager.cpp
    src/stats_panel.cpp
    src/camera_grid.cpp
    src/ui_helpers.cpp
)

set(HEADERS
    include/types.h
    include/main_window.h
    include/console_window.h
    include/console_sink.h
    include/settings_manager.h
    include/stats_panel.h
    include/camera_grid.h
    include/ui_helpers.h
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NLOHMANN_JSON_INCLUDE_DIR}
)

# Add concurrentqueue include if found
if(CONCURRENTQUEUE_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CONCURRENTQUEUE_INCLUDE_DIR})
    message(STATUS "ConcurrentQueue: Found at ${CONCURRENTQUEUE_INCLUDE_DIR}")
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    PkgConfig::LIBAV
    PkgConfig::SDL2
    PkgConfig::SDL2_TTF
    spdlog::spdlog
    pthread
)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.ini
    ${CMAKE_CURRENT_SOURCE_DIR}/config/settings.json
    DESTINATION bin/config
)

# Print configuration summary
message(STATUS "")
message(STATUS "=== ${PROJECT_NAME} Configuration (Ubuntu 22.04) ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "FFmpeg: ${LIBAV_VERSION}")
message(STATUS "SDL2: ${SDL2_VERSION}")
message(STATUS "SDL2_ttf: ${SDL2_TTF_VERSION}")
message(STATUS "spdlog: Found")
message(STATUS "nlohmann_json: Found")
message(STATUS "=====================================================")
message(STATUS "")
