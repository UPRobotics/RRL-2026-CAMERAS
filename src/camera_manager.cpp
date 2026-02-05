#include "camera_manager.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include <array>
#include <cstring>

// For Linux ping
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <netdb.h>

namespace camera_viewer {

CameraManager::CameraManager(SDL_Renderer* renderer)
    : m_renderer(renderer)
{
}

CameraManager::~CameraManager() {
    stopAll();
    
    if (m_discoveryThread.joinable()) {
        m_discoveryRunning = false;
        m_discoveryThread.join();
    }
}

void CameraManager::setCameraConfigs(const std::vector<CameraConfig>& configs) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    m_configs = configs;
    
    // Clear existing streams
    {
        std::lock_guard<std::mutex> streamLock(m_streamsMutex);
        m_streams.clear();
    }
    
    spdlog::info("Configured {} cameras", configs.size());
}

void CameraManager::setStreamingSettings(const StreamingSettings& settings) {
    m_settings = settings;
    m_currentQuality = settings.default_quality;
}

int CameraManager::discoverCameras(DiscoveryCallback callback) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    if (m_configs.empty()) {
        spdlog::warn("No cameras configured");
        return 0;
    }
    
    spdlog::info("Discovering cameras...");
    
    int availableCount = 0;
    int totalCount = static_cast<int>(m_configs.size());
    
    for (size_t i = 0; i < m_configs.size(); ++i) {
        auto& config = m_configs[i];
        
        if (!config.enabled) {
            spdlog::debug("Camera {} ({}) is disabled", i + 1, config.name);
            config.available = false;
            continue;
        }
        
        spdlog::debug("Pinging camera {} at {}", i + 1, config.ip);
        
        bool reachable = pingHost(config.ip, m_settings.ping_timeout_ms);
        config.available = reachable;
        
        if (reachable) {
            availableCount++;
            spdlog::info("Camera {} ({}) at {} - AVAILABLE", i + 1, config.name, config.ip);
        } else {
            spdlog::warn("Camera {} ({}) at {} - UNREACHABLE", i + 1, config.name, config.ip);
        }
        
        if (callback) {
            callback(availableCount, totalCount);
        }
    }
    
    spdlog::info("Discovery complete: {}/{} cameras available", availableCount, totalCount);
    return availableCount;
}

void CameraManager::startAll(StreamQuality quality) {
    std::lock_guard<std::mutex> configLock(m_configMutex);
    std::lock_guard<std::mutex> streamLock(m_streamsMutex);
    
    m_currentQuality = quality;
    
    // Clear existing streams
    m_streams.clear();
    m_streams.resize(m_configs.size());
    
    int startedCount = 0;
    for (size_t i = 0; i < m_configs.size(); ++i) {
        if (m_configs[i].available && m_configs[i].enabled) {
            m_streams[i] = std::make_unique<CameraStream>(
                static_cast<int>(i), m_configs[i], m_renderer);
            
            if (m_streams[i]->start(quality)) {
                startedCount++;
            }
        }
    }
    
    spdlog::info("Started {} camera streams", startedCount);
}

void CameraManager::stopAll() {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    
    for (auto& stream : m_streams) {
        if (stream) {
            stream->stop();
        }
    }
    
    m_streams.clear();
    spdlog::info("Stopped all camera streams");
}

void CameraManager::restartAll() {
    spdlog::info("Restarting all cameras");
    stopAll();
    discoverCameras();
    startAll(m_currentQuality);
}

bool CameraManager::startCamera(int index, StreamQuality quality) {
    std::lock_guard<std::mutex> configLock(m_configMutex);
    std::lock_guard<std::mutex> streamLock(m_streamsMutex);
    
    if (index < 0 || index >= static_cast<int>(m_configs.size())) {
        spdlog::error("Invalid camera index: {}", index);
        return false;
    }
    
    if (!m_configs[index].available) {
        spdlog::warn("Camera {} is not available", index + 1);
        return false;
    }
    
    // Ensure streams vector is large enough
    if (m_streams.size() <= static_cast<size_t>(index)) {
        m_streams.resize(m_configs.size());
    }
    
    // Stop existing stream if any
    if (m_streams[index]) {
        m_streams[index]->stop();
    }
    
    m_streams[index] = std::make_unique<CameraStream>(index, m_configs[index], m_renderer);
    return m_streams[index]->start(quality);
}

void CameraManager::stopCamera(int index) {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    
    if (index >= 0 && index < static_cast<int>(m_streams.size()) && m_streams[index]) {
        m_streams[index]->stop();
        m_streams[index].reset();
    }
}

int CameraManager::getAvailableCameraCount() const {
    std::lock_guard<std::mutex> lock(m_configMutex);
    return static_cast<int>(std::count_if(m_configs.begin(), m_configs.end(),
        [](const CameraConfig& c) { return c.available && c.enabled; }));
}

std::vector<int> CameraManager::getAvailableCameraIndices() const {
    std::lock_guard<std::mutex> lock(m_configMutex);
    std::vector<int> indices;
    for (size_t i = 0; i < m_configs.size(); ++i) {
        if (m_configs[i].available && m_configs[i].enabled) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}

int CameraManager::getRunningCameraCount() const {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    return static_cast<int>(std::count_if(m_streams.begin(), m_streams.end(),
        [](const std::unique_ptr<CameraStream>& s) { return s && s->isRunning(); }));
}

const CameraConfig& CameraManager::getCameraConfig(int index) const {
    std::lock_guard<std::mutex> lock(m_configMutex);
    static CameraConfig empty;
    if (index < 0 || index >= static_cast<int>(m_configs.size())) {
        return empty;
    }
    return m_configs[index];
}

CameraStats CameraManager::getCameraStats(int index) const {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    if (index >= 0 && index < static_cast<int>(m_streams.size()) && m_streams[index]) {
        return m_streams[index]->getStats();
    }
    return CameraStats{};
}

SDL_Texture* CameraManager::getCameraTexture(int index) {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    if (index >= 0 && index < static_cast<int>(m_streams.size()) && m_streams[index]) {
        return m_streams[index]->getFrameTexture();
    }
    return nullptr;
}

void CameraManager::updateTexturesFromMainThread() {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    for (auto& stream : m_streams) {
        if (stream && stream->isRunning()) {
            stream->updateTextureFromMainThread();
        }
    }
}

bool CameraManager::isCameraAvailable(int index) const {
    std::lock_guard<std::mutex> lock(m_configMutex);
    if (index < 0 || index >= static_cast<int>(m_configs.size())) {
        return false;
    }
    return m_configs[index].available && m_configs[index].enabled;
}

void CameraManager::setStateChangeCallback(StateChangeCallback callback) {
    m_stateChangeCallback = std::move(callback);
}

void CameraManager::setAllQuality(StreamQuality quality) {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    m_currentQuality = quality;
    
    for (auto& stream : m_streams) {
        if (stream) {
            stream->setQuality(quality);
        }
    }
}

float CameraManager::getAverageFps() const {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    
    float totalFps = 0.0f;
    int count = 0;
    
    for (const auto& stream : m_streams) {
        if (stream && stream->isRunning()) {
            auto stats = stream->getStats();
            if (stats.state == CameraState::Connected) {
                totalFps += stats.current_fps;
                count++;
            }
        }
    }
    
    return count > 0 ? totalFps / count : 0.0f;
}

float CameraManager::getAverageLatency() const {
    std::lock_guard<std::mutex> lock(m_streamsMutex);
    
    float totalLatency = 0.0f;
    int count = 0;
    
    for (const auto& stream : m_streams) {
        if (stream && stream->isRunning()) {
            auto stats = stream->getStats();
            if (stats.state == CameraState::Connected) {
                totalLatency += stats.latency_ms;
                count++;
            }
        }
    }
    
    return count > 0 ? totalLatency / count : 0.0f;
}

bool CameraManager::pingHost(const std::string& ip, int timeoutMs) {
    // Use system ping command for simplicity (ICMP requires root on Linux)
    // Format: ping -c 1 -W <timeout_seconds> <ip>
    
    int timeoutSec = (timeoutMs + 999) / 1000;  // Round up to seconds
    if (timeoutSec < 1) timeoutSec = 1;
    
    std::string cmd = "ping -c 1 -W " + std::to_string(timeoutSec) + " " + ip + " > /dev/null 2>&1";
    
    int result = system(cmd.c_str());
    return (result == 0);
}

void CameraManager::discoveryThread(DiscoveryCallback callback) {
    m_discoveryRunning = true;
    
    int available = discoverCameras([this, &callback](int avail, int total) {
        if (callback && m_discoveryRunning) {
            callback(avail, total);
        }
    });
    
    m_discoveryRunning = false;
    
    if (callback) {
        callback(available, static_cast<int>(m_configs.size()));
    }
}

} // namespace camera_viewer
