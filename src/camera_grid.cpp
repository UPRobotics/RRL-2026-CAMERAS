#include "camera_grid.h"
#include "main_window.h"
#include "ui_helpers.h"
#include <cmath>
#include <cstdio>

namespace camera_viewer {

CameraGrid::CameraGrid()
    : m_activeCameraCount(0)
    , m_viewMode(ViewMode::GRID_NXN)
    , m_selectedCameraIndex(0)
{
}

void CameraGrid::setActiveCameraCount(int count) {
    m_activeCameraCount = count;
    // Ensure selected camera is within bounds
    if (m_selectedCameraIndex >= count) {
        m_selectedCameraIndex = count > 0 ? count - 1 : 0;
    }
}

void CameraGrid::setViewMode(ViewMode mode) {
    m_viewMode = mode;
}

void CameraGrid::setSelectedCameraIndex(int index) {
    if (index >= 0 && index < m_activeCameraCount) {
        m_selectedCameraIndex = index;
    }
}

void CameraGrid::render(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (m_activeCameraCount == 0) {
        // Draw "No cameras active" message
        UIHelpers::drawText(renderer, width / 2 - 60, height / 2, 
                           "No cameras active", Colors::CAMERA_TEXT, 14);
        return;
    }

    if (m_viewMode == ViewMode::FULLSCREEN) {
        // Render single camera fullscreen
        SDL_Rect cameraRect = {x + CAMERA_GAP, y + CAMERA_GAP, 
                              width - CAMERA_GAP * 2, height - CAMERA_GAP * 2};
        renderCameraSlot(renderer, cameraRect, m_selectedCameraIndex);
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

        // Render each camera slot
        int cameraIndex = 0;
        for (int row = 0; row < rows && cameraIndex < m_activeCameraCount; ++row) {
            for (int col = 0; col < cols && cameraIndex < m_activeCameraCount; ++col) {
                int cameraX = x + CAMERA_GAP + col * (cameraWidth + CAMERA_GAP);
                int cameraY = y + CAMERA_GAP + row * (cameraHeight + CAMERA_GAP);

                SDL_Rect cameraRect = {cameraX, cameraY, cameraWidth, cameraHeight};
                renderCameraSlot(renderer, cameraRect, cameraIndex);
                cameraIndex++;
            }
        }
    }
}

void CameraGrid::calculateGridLayout(int& rows, int& cols) {
    // Calculate optimal grid size for N cameras
    if (m_activeCameraCount <= 1) {
        rows = 1;
        cols = 1;
    } else if (m_activeCameraCount <= 4) {
        rows = 2;
        cols = 2;
    } else if (m_activeCameraCount <= 9) {
        rows = 3;
        cols = 3;
    } else if (m_activeCameraCount <= 16) {
        rows = 4;
        cols = 4;
    } else {
        // For larger numbers, calculate square-ish layout
        cols = static_cast<int>(std::ceil(std::sqrt(m_activeCameraCount)));
        rows = static_cast<int>(std::ceil(static_cast<float>(m_activeCameraCount) / cols));
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

    // Draw placeholder grid pattern
    SDL_SetRenderDrawColor(renderer, 60, 60, 65, 255);
    int gridSize = 40;
    for (int i = gridSize; i < rect.w; i += gridSize) {
        SDL_RenderDrawLine(renderer, rect.x + i, rect.y, rect.x + i, rect.y + rect.h);
    }
    for (int i = gridSize; i < rect.h; i += gridSize) {
        SDL_RenderDrawLine(renderer, rect.x, rect.y + i, rect.x + rect.w, rect.y + i);
    }

    // Draw camera info overlay
    renderCameraOverlay(renderer, rect, cameraIndex);
}

void CameraGrid::renderCameraOverlay(SDL_Renderer* renderer, SDL_Rect rect, int cameraIndex) {
    // Draw semi-transparent overlay at top
    SDL_Rect overlayRect = {rect.x, rect.y, rect.w, 30};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &overlayRect);

    // Draw camera label
    char label[64];
    snprintf(label, sizeof(label), "Camera %d [No Feed]", cameraIndex + 1);
    UIHelpers::drawText(renderer, rect.x + 10, rect.y + 8, label, Colors::CAMERA_TEXT, 12);

    // Draw status indicator
    SDL_Rect statusRect = {rect.x + rect.w - 25, rect.y + 8, 15, 15};
    SDL_SetRenderDrawColor(renderer, 
        Colors::CAMERA_INACTIVE.r, Colors::CAMERA_INACTIVE.g, 
        Colors::CAMERA_INACTIVE.b, Colors::CAMERA_INACTIVE.a);
    SDL_RenderFillRect(renderer, &statusRect);

    // If this camera is selected in fullscreen mode, draw selection indicator
    if (m_viewMode == ViewMode::FULLSCREEN || cameraIndex == m_selectedCameraIndex) {
        // Draw selected border highlight
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

} // namespace camera_viewer
