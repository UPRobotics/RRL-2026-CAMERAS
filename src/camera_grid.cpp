#include "camera_grid.h"
#include "camera_manager.h"
#include "main_window.h"
#include "ui_helpers.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace camera_viewer {

CameraGrid::CameraGrid()
    : m_viewMode(ViewMode::GRID_NXN)
    , m_selectedCameraIndex(0)
    , m_selectedRealCameraIndex(-1)
{
}

void CameraGrid::setCameraManager(CameraManager* manager) {
    m_cameraManager = manager;
}

void CameraGrid::setAvailableCameraIndices(const std::vector<int>& indices) {
    m_availableCameraIndices = indices;
    
    // Reset selection if needed
    if (m_availableCameraIndices.empty()) {
        m_selectedCameraIndex = 0;
        m_selectedRealCameraIndex = -1;
    } else {
        // Ensure selection is valid
        if (m_selectedCameraIndex >= static_cast<int>(m_availableCameraIndices.size())) {
            m_selectedCameraIndex = 0;
        }
        m_selectedRealCameraIndex = m_availableCameraIndices[m_selectedCameraIndex];
    }
}

void CameraGrid::setViewMode(ViewMode mode) {
    m_viewMode = mode;
}

void CameraGrid::setSelectedCameraIndex(int index) {
    // This is the index within the available cameras list (0, 1, 2...) 
    if (index >= 0 && index < static_cast<int>(m_availableCameraIndices.size())) {
        m_selectedCameraIndex = index;
        m_selectedRealCameraIndex = m_availableCameraIndices[index];
    }
}

void CameraGrid::render(SDL_Renderer* renderer, int x, int y, int width, int height) {
    int activeCameraCount = static_cast<int>(m_availableCameraIndices.size());
    
    if (activeCameraCount == 0) {
        // Draw "No cameras active" message
        UIHelpers::drawText(renderer, width / 2 - 60, height / 2, 
                           "No cameras active", Colors::CAMERA_TEXT, 14);
        return;
    }

    if (m_viewMode == ViewMode::FULLSCREEN) {
        // Render single camera fullscreen - use the REAL camera index
        SDL_Rect cameraRect = {x + CAMERA_GAP, y + CAMERA_GAP, 
                              width - CAMERA_GAP * 2, height - CAMERA_GAP * 2};
        renderCameraSlot(renderer, cameraRect, m_selectedRealCameraIndex);
    } else {
        // Calculate grid layout
        int rows, cols;
        if (m_viewMode == ViewMode::GRID_2X2) {
            rows = 2;
            cols = 2;
        } else {
            calculateGridLayout(rows, cols);
        }

        // Calculate camera slot dimensions
        int cameraWidth = (width - CAMERA_GAP * (cols + 1)) / cols;
        int cameraHeight = (height - CAMERA_GAP * (rows + 1)) / rows;

        // Render each available camera slot
        int slotIndex = 0;
        for (int row = 0; row < rows && slotIndex < activeCameraCount; ++row) {
            for (int col = 0; col < cols && slotIndex < activeCameraCount; ++col) {
                int cameraX = x + CAMERA_GAP + col * (cameraWidth + CAMERA_GAP);
                int cameraY = y + CAMERA_GAP + row * (cameraHeight + CAMERA_GAP);

                SDL_Rect cameraRect = {cameraX, cameraY, cameraWidth, cameraHeight};
                // Use the REAL camera index from available indices
                int realCameraIndex = m_availableCameraIndices[slotIndex];
                renderCameraSlot(renderer, cameraRect, realCameraIndex);
                slotIndex++;
            }
        }
    }
}

void CameraGrid::calculateGridLayout(int& rows, int& cols) {
    // Calculate optimal grid size for N cameras
    int count = static_cast<int>(m_availableCameraIndices.size());
    if (count <= 1) {
        rows = 1;
        cols = 1;
    } else if (count <= 4) {
        rows = 2;
        cols = 2;
    } else if (count <= 9) {
        rows = 3;
        cols = 3;
    } else if (count <= 16) {
        rows = 4;
        cols = 4;
    } else {
        // For larger numbers, calculate square-ish layout
        cols = static_cast<int>(std::ceil(std::sqrt(count)));
        rows = static_cast<int>(std::ceil(static_cast<float>(count) / cols));
    }
}

void CameraGrid::renderCameraSlot(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex) {
    // Draw border
    SDL_SetRenderDrawColor(renderer, 
        Colors::CAMERA_BORDER.r, Colors::CAMERA_BORDER.g, 
        Colors::CAMERA_BORDER.b, Colors::CAMERA_BORDER.a);
    for (int i = 0; i < CAMERA_BORDER; ++i) {
        SDL_Rect borderRect = {rect.x - i, rect.y - i, rect.w + i * 2, rect.h + i * 2};
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    // Draw background
    SDL_SetRenderDrawColor(renderer, 
        Colors::CAMERA_BG.r, Colors::CAMERA_BG.g, 
        Colors::CAMERA_BG.b, Colors::CAMERA_BG.a);
    SDL_RenderFillRect(renderer, &rect);

    // Get camera stats
    CameraStats stats;
    SDL_Texture* texture = nullptr;
    
    if (m_cameraManager) {
        texture = m_cameraManager->getCameraTexture(cameraIndex);
        stats = m_cameraManager->getCameraStats(cameraIndex);
    }

    if (texture && stats.state == CameraState::Connected) {
        // Render actual video
        renderVideoTexture(renderer, rect, texture, cameraIndex);
    } else {
        // Render placeholder
        renderPlaceholder(renderer, rect, cameraIndex);
    }

    // Render overlay with camera info
    renderCameraOverlay(renderer, rect, cameraIndex, stats);

    // If this camera is selected, draw selection highlight
    // Compare with the REAL camera index
    if (cameraIndex == m_selectedRealCameraIndex) {
        SDL_SetRenderDrawColor(renderer, 
            Colors::BUTTON_HOVER.r, Colors::BUTTON_HOVER.g, 
            Colors::BUTTON_HOVER.b, 255);
        for (int i = 0; i < 3; ++i) {
            SDL_Rect highlightRect = {
                rect.x - CAMERA_BORDER - i, 
                rect.y - CAMERA_BORDER - i, 
                rect.w + (CAMERA_BORDER + i) * 2, 
                rect.h + (CAMERA_BORDER + i) * 2
            };
            SDL_RenderDrawRect(renderer, &highlightRect);
        }
    }
}

SDL_Rect CameraGrid::calculateFitRect(SDL_Rect target, int srcWidth, int srcHeight) {
    if (srcWidth <= 0 || srcHeight <= 0) {
        return target;
    }

    float targetAspect = static_cast<float>(target.w) / target.h;
    float srcAspect = static_cast<float>(srcWidth) / srcHeight;

    SDL_Rect result;
    
    if (srcAspect > targetAspect) {
        // Source is wider - fit to width
        result.w = target.w;
        result.h = static_cast<int>(target.w / srcAspect);
        result.x = target.x;
        result.y = target.y + (target.h - result.h) / 2;
    } else {
        // Source is taller - fit to height
        result.h = target.h;
        result.w = static_cast<int>(target.h * srcAspect);
        result.x = target.x + (target.w - result.w) / 2;
        result.y = target.y;
    }

    return result;
}

void CameraGrid::renderVideoTexture(SDL_Renderer* renderer, SDL_Rect rect, SDL_Texture* texture, int cameraIndex) {
    (void)cameraIndex; // May be used later for overlays
    
    // Query texture size
    int texW, texH;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    
    // Calculate destination rect that maintains aspect ratio
    SDL_Rect destRect = calculateFitRect(rect, texW, texH);
    
    // Fill letterbox/pillarbox areas with black
    if (destRect.x > rect.x || destRect.y > rect.y) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // Render the video texture
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
}

void CameraGrid::renderPlaceholder(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex) {
    (void)cameraIndex;
    
    // Draw placeholder grid pattern
    SDL_SetRenderDrawColor(renderer, 60, 60, 65, 255);
    int gridSize = 40;
    for (int i = gridSize; i < rect.w; i += gridSize) {
        SDL_RenderDrawLine(renderer, rect.x + i, rect.y, rect.x + i, rect.y + rect.h);
    }
    for (int i = gridSize; i < rect.h; i += gridSize) {
        SDL_RenderDrawLine(renderer, rect.x, rect.y + i, rect.x + rect.w, rect.y + i);
    }
}

void CameraGrid::renderCameraOverlay(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex, const CameraStats& stats) {
    // Draw semi-transparent overlay at top
    SDL_Rect overlayRect = {rect.x, rect.y, rect.w, 30};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &overlayRect);

    // Get camera name from manager if available
    std::string cameraName;
    if (m_cameraManager) {
        const auto& config = m_cameraManager->getCameraConfig(cameraIndex);
        cameraName = config.name;
    }
    if (cameraName.empty()) {
        cameraName = "Camera " + std::to_string(cameraIndex + 1);
    }

    // Draw camera label with status
    char label[128];
    const char* stateStr = "";
    switch (stats.state) {
        case CameraState::Disconnected: stateStr = "Disconnected"; break;
        case CameraState::Connecting: stateStr = "Connecting..."; break;
        case CameraState::Connected: stateStr = ""; break; // Don't show when connected
        case CameraState::Reconnecting: stateStr = "Reconnecting..."; break;
        case CameraState::Error: stateStr = "Error"; break;
    }
    
    if (stats.state == CameraState::Connected) {
        snprintf(label, sizeof(label), "%s [%.1f FPS]", 
                 cameraName.c_str(), stats.current_fps);
    } else {
        snprintf(label, sizeof(label), "%s [%s]", 
                 cameraName.c_str(), stateStr);
    }
    UIHelpers::drawText(renderer, rect.x + 10, rect.y + 8, label, Colors::CAMERA_TEXT, 12);

    // Draw status indicator
    SDL_Rect statusRect = {rect.x + rect.w - 25, rect.y + 8, 15, 15};
    
    SDL_Color statusColor;
    switch (stats.state) {
        case CameraState::Connected:
            statusColor = Colors::CAMERA_ACTIVE;
            break;
        case CameraState::Connecting:
        case CameraState::Reconnecting:
            statusColor = {255, 165, 0, 255}; // Orange
            break;
        case CameraState::Disconnected:
        case CameraState::Error:
        default:
            statusColor = Colors::CAMERA_INACTIVE;
            break;
    }
    
    SDL_SetRenderDrawColor(renderer, statusColor.r, statusColor.g, statusColor.b, statusColor.a);
    SDL_RenderFillRect(renderer, &statusRect);

    // Show resolution info at bottom if connected
    if (stats.state == CameraState::Connected && stats.frame_width > 0) {
        SDL_Rect bottomOverlay = {rect.x, rect.y + rect.h - 25, rect.w, 25};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &bottomOverlay);
        
        char resLabel[64];
        snprintf(resLabel, sizeof(resLabel), "%dx%d | Frames: %lu", 
                 stats.frame_width, stats.frame_height, stats.total_frames);
        UIHelpers::drawText(renderer, rect.x + 10, rect.y + rect.h - 18, 
                           resLabel, Colors::CAMERA_TEXT, 10);
    }
}

} // namespace camera_viewer
