#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <functional>
#include "types.h"

namespace camera_viewer {

enum class ViewMode;
class CameraManager;

/**
 * @brief Camera grid component managing camera layout and rendering
 * 
 * Renders camera video feeds in a grid layout, scaling each camera
 * to fit its grid cell while maintaining aspect ratio.
 */
class CameraGrid {
public:
    CameraGrid();
    ~CameraGrid() = default;

    /**
     * @brief Set the camera manager for retrieving textures
     */
    void setCameraManager(CameraManager* manager);

    /**
     * @brief Set the available camera indices (actual camera numbers)
     */
    void setAvailableCameraIndices(const std::vector<int>& indices);

    /**
     * @brief Set the current view mode
     */
    void setViewMode(ViewMode mode);

    /**
     * @brief Get the currently selected camera index (for fullscreen mode)
     * @return Index within available cameras list (0, 1, 2...)
     */
    int getSelectedCameraIndex() const { return m_selectedCameraIndex; }
    
    /**
     * @brief Get the actual camera number of selected camera
     * @return Real camera index (e.g., 4 for Camera 5)
     */
    int getSelectedRealCameraIndex() const { return m_selectedRealCameraIndex; }
    
    /**
     * @brief Get number of available cameras
     */
    int getAvailableCameraCount() const { return static_cast<int>(m_availableCameraIndices.size()); }

    /**
     * @brief Set the selected camera index (for fullscreen mode)
     */
    void setSelectedCameraIndex(int index);

    /**
     * @brief Render the camera grid
     */
    void render(SDL_Renderer* renderer, int x, int y, int width, int height);

private:
    /**
     * @brief Calculate grid dimensions based on camera count
     */
    void calculateGridLayout(int& rows, int& cols);

    /**
     * @brief Render a camera slot (with video or placeholder)
     */
    void renderCameraSlot(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex);

    /**
     * @brief Render video texture scaled to fit the rect while maintaining aspect ratio
     */
    void renderVideoTexture(SDL_Renderer* renderer, SDL_Rect rect, SDL_Texture* texture, int cameraIndex);

    /**
     * @brief Render placeholder for disconnected camera
     */
    void renderPlaceholder(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex);

    /**
     * @brief Render camera info overlay
     */
    void renderCameraOverlay(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex, const CameraStats& stats);

    /**
     * @brief Calculate destination rect that fits source in target while maintaining aspect ratio
     */
    SDL_Rect calculateFitRect(SDL_Rect target, int srcWidth, int srcHeight);

    CameraManager* m_cameraManager = nullptr;
    std::vector<int> m_availableCameraIndices; // Actual camera indices that are available
    ViewMode m_viewMode;
    int m_selectedCameraIndex; // For fullscreen mode (index into m_availableCameraIndices)
    int m_selectedRealCameraIndex; // Actual camera number

    static constexpr int CAMERA_GAP = 5;
    static constexpr int CAMERA_BORDER = 2;
};

} // namespace camera_viewer
