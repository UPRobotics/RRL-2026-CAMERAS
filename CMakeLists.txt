cmake_minimum_required(VERSION 3.25)
project(RTSPCameraViewer 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Ultra-low latency RTSP multi-camera viewer"
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

# Compiler optimizations for Release
if(MSVC)
    # MSVC compiler flags
    add_compile_options(
        /W4                 # Warning level 4
        /permissive-        # Standards conformance
        $<$<CONFIG:Release>:/O2>        # Maximum optimization
        $<$<CONFIG:Release>:/GL>        # Whole program optimization
        $<$<CONFIG:Release>:/arch:AVX2> # AVX2 instructions
    )
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>      # Link-time code generation
        $<$<CONFIG:Release>:/OPT:REF>   # Eliminate unused functions
        $<$<CONFIG:Release>:/OPT:ICF>   # Identical COMDAT folding
    )
else()
    # GCC/Clang flags
    add_compile_options(
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-march=native>
        $<$<CONFIG:Release>:-flto>
    )
    add_link_options(
        $<$<CONFIG:Release>:-flto>
    )
endif()

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# FFmpeg libraries
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
    libswresample
)

# SDL2 - try CONFIG first, then MODULE
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 MODULE REQUIRED)
endif()

# SDL2_ttf for text rendering
find_package(SDL2_ttf CONFIG REQUIRED)

# spdlog for logging
find_package(spdlog CONFIG REQUIRED)

# nlohmann_json for configuration
find_package(nlohmann_json CONFIG REQUIRED)

# Optional: Concurrentqueue for lock-free queues
find_package(concurrentqueue CONFIG QUIET)

# Source files
set(SOURCES
    src/main.cpp
    src/main_window.cpp
    src/console_window.cpp
    src/stats_panel.cpp
    src/camera_grid.cpp
    src/ui_helpers.cpp
)

set(HEADERS
    include/types.h
    include/main_window.h
    include/console_window.h
    include/stats_panel.h
    include/camera_grid.h
    include/ui_helpers.h
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    PkgConfig::LIBAV
    SDL2::SDL2
    SDL2::SDL2main
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

# Optionally link concurrentqueue if found
if(concurrentqueue_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE concurrentqueue::concurrentqueue)
    message(STATUS "ConcurrentQueue: Found")
endif()

# Windows-specific settings
if(WIN32)
    # Set subsystem to Windows for GUI app (no console)
    # Comment this line if you want console output during development
    # set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
    
    # Copy SDL2 DLL to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.ini
    DESTINATION bin
)

# Print configuration summary
message(STATUS "")
message(STATUS "=== ${PROJECT_NAME} Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "FFmpeg found: ${LIBAV_FOUND}")
message(STATUS "SDL2 found: ${SDL2_FOUND}")
message(STATUS "spdlog found: ${spdlog_FOUND}")
message(STATUS "nlohmann_json found: ${nlohmann_json_FOUND}")
message(STATUS "=====================================")
message(STATUS "")
