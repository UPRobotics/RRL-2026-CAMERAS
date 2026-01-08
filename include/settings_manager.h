#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <nlohmann/json.hpp>

namespace camera_viewer {

/**
 * @brief Manages application settings and keybindings
 */
class SettingsManager {
public:
    static SettingsManager& instance();
    
    /**
     * @brief Load settings from file
     */
    bool load(const std::string& filepath = "config/settings.json");
    
    /**
     * @brief Save settings to file
     */
    bool save(const std::string& filepath = "config/settings.json");
    
    /**
     * @brief Get SDL keycode for an action
     */
    SDL_Keycode getKeyBinding(const std::string& action) const;
    
    /**
     * @brief Check if a key matches an action
     */
    bool isKeyForAction(SDL_Keycode key, const std::string& action) const;
    
    /**
     * @brief Set key binding for an action
     */
    void setKeyBinding(const std::string& action, SDL_Keycode key);
    
    /**
     * @brief Display settings
     */
    bool shouldRememberWindowPosition() const { return m_rememberWindowPosition; }
    bool shouldRememberWindowSize() const { return m_rememberWindowSize; }
    bool shouldFullscreenOnStartup() const { return m_fullscreenOnStartup; }
    
    int getLastWindowX() const { return m_lastWindowX; }
    int getLastWindowY() const { return m_lastWindowY; }
    int getLastWindowWidth() const { return m_lastWindowWidth; }
    int getLastWindowHeight() const { return m_lastWindowHeight; }
    
    void setLastWindowPosition(int x, int y) { m_lastWindowX = x; m_lastWindowY = y; }
    void setLastWindowSize(int width, int height) { m_lastWindowWidth = width; m_lastWindowHeight = height; }
    
    /**
     * @brief Console settings
     */
    bool shouldAutoOpenConsole() const { return m_autoOpenConsole; }
    bool shouldRememberConsolePosition() const { return m_rememberConsolePosition; }
    
    int getLastConsoleX() const { return m_lastConsoleX; }
    int getLastConsoleY() const { return m_lastConsoleY; }
    int getLastConsoleWidth() const { return m_lastConsoleWidth; }
    int getLastConsoleHeight() const { return m_lastConsoleHeight; }
    
    void setLastConsolePosition(int x, int y) { m_lastConsoleX = x; m_lastConsoleY = y; }
    void setLastConsoleSize(int width, int height) { m_lastConsoleWidth = width; m_lastConsoleHeight = height; }
    
    /**
     * @brief Debug settings
     */
    std::string getDebugRtspUrl() const { return m_debugRtspUrl; }
    int getDebugVirtualCameraCount() const { return m_debugVirtualCameraCount; }
    void setDebugRtspUrl(const std::string& url) { m_debugRtspUrl = url; }
    void setDebugVirtualCameraCount(int count) { m_debugVirtualCameraCount = count; }
    
private:
    SettingsManager() = default;
    ~SettingsManager() = default;
    
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    
    SDL_Keycode stringToKeycode(const std::string& keyName) const;
    std::string keycodeToString(SDL_Keycode key) const;
    
    std::map<std::string, SDL_Keycode> m_keybindings;
    
    // Display settings
    bool m_rememberWindowPosition = true;
    bool m_rememberWindowSize = true;
    bool m_fullscreenOnStartup = false;
    int m_lastWindowX = -1;
    int m_lastWindowY = -1;
    int m_lastWindowWidth = 1280;
    int m_lastWindowHeight = 720;
    
    // Console settings
    bool m_autoOpenConsole = false;
    bool m_rememberConsolePosition = true;
    int m_lastConsoleX = 100;
    int m_lastConsoleY = 100;
    int m_lastConsoleWidth = 800;
    int m_lastConsoleHeight = 500;
    
    // Debug settings
    std::string m_debugRtspUrl = "rtsp://127.0.0.1:8554/webcam";
    int m_debugVirtualCameraCount = 1;
};

} // namespace camera_viewer
