#pragma once

#include "types.h"
#include "camera_stream.h"
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>

namespace camera_viewer {

/**
 * @brief Manages all camera streams including discovery and lifecycle
 * 
 * Handles:
 * - Pinging camera IPs to check availability
 * - Starting/stopping individual camera streams
 * - Coordinating quality switching
 */
class CameraManager {
public:
    using DiscoveryCallback = std::function<void(int availableCount, int totalCount)>;
    using StateChangeCallback = std::function<void(int cameraIndex, CameraState newState)>;
    
    explicit CameraManager(SDL_Renderer* renderer);
    ~CameraManager();
    
    // Disable copy
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    
    /**
     * @brief Set camera configurations (from settings)
     */
    void setCameraConfigs(const std::vector<CameraConfig>& configs);
    
    /**
     * @brief Set streaming settings
     */
    void setStreamingSettings(const StreamingSettings& settings);
    
    /**
     * @brief Discover available cameras by pinging their IPs
     * @param callback Called with progress (available, total)
     * @return Number of cameras available
     */
    int discoverCameras(DiscoveryCallback callback = nullptr);
    
    /**
     * @brief Start all available camera streams
     * @param quality Stream quality (High = 1080p, Low = 480p)
     */
    void startAll(StreamQuality quality = StreamQuality::High);
    
    /**
     * @brief Stop all camera streams
     */
    void stopAll();
    
    /**
     * @brief Restart all camera streams
     */
    void restartAll();
    
    /**
     * @brief Start a specific camera stream
     */
    bool startCamera(int index, StreamQuality quality = StreamQuality::High);
    
    /**
     * @brief Stop a specific camera stream
     */
    void stopCamera(int index);
    
    /**
     * @brief Get number of configured cameras
     */
    int getCameraCount() const { return static_cast<int>(m_configs.size()); }
    
    /**
     * @brief Get number of available cameras (after discovery)
     */
    int getAvailableCameraCount() const;
    
    /**
     * @brief Get list of available camera indices
     */
    std::vector<int> getAvailableCameraIndices() const;
    
    /**
     * @brief Get number of currently running cameras
     */
    int getRunningCameraCount() const;
    
    /**
     * @brief Get camera configuration
     */
    const CameraConfig& getCameraConfig(int index) const;
    
    /**
     * @brief Get camera statistics
     */
    CameraStats getCameraStats(int index) const;
    
    /**
     * @brief Get camera frame texture for rendering
     * @param index Camera index
     * @return SDL_Texture* or nullptr if not available
     */
    SDL_Texture* getCameraTexture(int index);
    
    /**
     * @brief Update all camera textures from the main thread
     * MUST be called from the main/render thread before rendering
     */
    void updateTexturesFromMainThread();
    
    /**
     * @brief Check if camera is available (can be connected)
     */
    bool isCameraAvailable(int index) const;
    
    /**
     * @brief Set callback for state changes
     */
    void setStateChangeCallback(StateChangeCallback callback);
    
    /**
     * @brief Set quality for all cameras
     */
    void setAllQuality(StreamQuality quality);
    
    /**
     * @brief Get average FPS across all cameras
     */
    float getAverageFps() const;
    
    /**
     * @brief Get average latency across all cameras
     */
    float getAverageLatency() const;
    
    /**
     * @brief Check for camera frame timeouts and auto-restart if needed
     * Should be called periodically from the main loop
     */
    void checkAutoRecovery();
    
private:
    /**
     * @brief Ping a single IP address
     * @param ip IP address to ping
     * @param timeoutMs Timeout in milliseconds
     * @return true if reachable
     */
    bool pingHost(const std::string& ip, int timeoutMs);
    
    /**
     * @brief Discovery thread function
     */
    void discoveryThread(DiscoveryCallback callback);
    
    // Renderer (not owned)
    SDL_Renderer* m_renderer;
    
    // Camera configurations
    std::vector<CameraConfig> m_configs;
    mutable std::mutex m_configMutex;
    
    // Camera streams
    std::vector<std::unique_ptr<CameraStream>> m_streams;
    mutable std::mutex m_streamsMutex;
    
    // Settings
    StreamingSettings m_settings;
    StreamQuality m_currentQuality = StreamQuality::High;
    
    // Callbacks
    StateChangeCallback m_stateChangeCallback;
    
    // Discovery
    std::atomic<bool> m_discoveryRunning{false};
    std::thread m_discoveryThread;
    
    // Auto-recovery tracking
    std::chrono::steady_clock::time_point m_lastRecoveryCheck;
    
    // Recovery queue - cameras to restart (done in background)
    std::vector<size_t> m_recoveryQueue;
    std::mutex m_recoveryMutex;
    std::thread m_recoveryThread;
    std::atomic<bool> m_recoveryRunning{false};
    
    /**
     * @brief Background thread function for camera recovery
     */
    void recoveryThreadFunc();
};

} // namespace camera_viewer
