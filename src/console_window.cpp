#include "console_window.h"
#include "ui_helpers.h"
#include <spdlog/spdlog.h>

namespace camera_viewer {

ConsoleWindow::ConsoleWindow()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_visible(false)
    , m_width(800)
    , m_height(500)
{
}

ConsoleWindow::~ConsoleWindow() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
}

bool ConsoleWindow::create(int x, int y, int width, int height) {
    m_width = width;
    m_height = height;

    // Create console window
    m_window = SDL_CreateWindow(
        "Console - Logs & Diagnostics",
        x, y, width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        spdlog::error("Console window creation failed: {}", SDL_GetError());
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        spdlog::error("Console renderer creation failed: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    // Add some initial messages
    addLog("Console initialized", Colors::CONSOLE_INFO);
    addLog("Welcome to RTSP Camera Viewer", Colors::CONSOLE_TEXT);
    addLog("Logs, warnings, and errors will appear here", Colors::CONSOLE_TEXT);

    m_visible = true;
    return true;
}

void ConsoleWindow::setVisible(bool visible) {
    m_visible = visible;
    if (m_window) {
        if (visible) {
            SDL_ShowWindow(m_window);
        } else {
            SDL_HideWindow(m_window);
        }
    }
}

void ConsoleWindow::addLog(const std::string& message, SDL_Color color) {
    LogEntry entry;
    entry.message = message;
    entry.color = color;
    m_logMessages.push_back(entry);

    // Limit log size
    if (m_logMessages.size() > MAX_LOG_ENTRIES) {
        m_logMessages.erase(m_logMessages.begin());
    }
}

void ConsoleWindow::render() {
    if (!m_visible || !m_window || !m_renderer) {
        return;
    }

    // Clear background
    SDL_SetRenderDrawColor(m_renderer,
        Colors::CONSOLE_BG.r, Colors::CONSOLE_BG.g, Colors::CONSOLE_BG.b, Colors::CONSOLE_BG.a);
    SDL_RenderClear(m_renderer);

    // Render log messages
    int yPos = 10;
    int lineHeight = 18;
    int startIndex = std::max(0, static_cast<int>(m_logMessages.size()) - (m_height / lineHeight));

    for (size_t i = startIndex; i < m_logMessages.size(); ++i) {
        const auto& log = m_logMessages[i];
        UIHelpers::drawText(m_renderer, 10, yPos, log.message, log.color, 12);
        yPos += lineHeight;
    }

    SDL_RenderPresent(m_renderer);
}

void ConsoleWindow::handleEvent(const SDL_Event& event) {
    if (!m_visible || !m_window) return;

    // Handle console-specific events
    if (event.type == SDL_WINDOWEVENT && 
        event.window.windowID == SDL_GetWindowID(m_window)) {
        
        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
            setVisible(false);
        } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            m_width = event.window.data1;
            m_height = event.window.data2;
        }
    }
}

} // namespace camera_viewer
