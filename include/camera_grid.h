#pragma once

#include <SDL2/SDL.h>
#include <vector>

namespace camera_viewer {

enum class ViewMode;

/**
 * @brief Camera grid component managing camera layout and rendering
 */
class CameraGrid {
public:
    CameraGrid();
    ~CameraGrid() = default;

    /**
     * @brief Set the number of active cameras
     */
    void setActiveCameraCount(int count);

    /**
     * @brief Set the current view mode
     */
    void setViewMode(ViewMode mode);

    /**
     * @brief Get the currently selected camera index (for fullscreen mode)
     */
    int getSelectedCameraIndex() const { return m_selectedCameraIndex; }

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
     * @brief Render a placeholder camera slot
     */
    void renderCameraSlot(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex);

    /**
     * @brief Render camera info overlay
     */
    void renderCameraOverlay(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex);

    int m_activeCameraCount;
    ViewMode m_viewMode;
    int m_selectedCameraIndex; // For fullscreen mode

    static constexpr int CAMERA_GAP = 5;
    static constexpr int CAMERA_BORDER = 2;
};

} // namespace camera_viewer
