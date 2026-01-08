#include "main_window.h"
#include "console_window.h"
#include "stats_panel.h"
#include "camera_grid.h"
#include "ui_helpers.h"
#include <spdlog/spdlog.h>

namespace camera_viewer {

MainWindow::MainWindow(const std::string& title, int width, int height)
    : m_title(title)
    , m_windowWidth(width)
    , m_windowHeight(height)
    , m_running(false)
    , m_isInitialized(false)
    , m_window(nullptr)
    , m_renderer(nullptr)
    , m_currentViewMode(ViewMode::GRID_NXN)
    , m_consolVisible(false)
    , m_activeCameraCount(4)
    , m_mainAreaY(TOOLBAR_HEIGHT)
    , m_mainAreaHeight(height - TOOLBAR_HEIGHT - STATSBAR_HEIGHT)
{
}

MainWindow::~MainWindow() {
    shutdown();
}

bool MainWindow::initialize() {
    if (m_isInitialized) {
        spdlog::warn("MainWindow already initialized");
        return true;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        spdlog::error("SDL initialization failed: {}", SDL_GetError());
        return false;
    }

    // Initialize font manager
    if (!FontManager::instance().initialize()) {
        spdlog::error("Font manager initialization failed");
        SDL_Quit();
        return false;
    }

    // Create window
    m_window = SDL_CreateWindow(
        m_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        spdlog::error("Window creation failed: {}", SDL_GetError());
        FontManager::instance().shutdown();
        SDL_Quit();
        return false;
    }

    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        spdlog::error("Renderer creation failed: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return false;
    }

    // Initialize UI components
    m_consoleWindow = std::make_unique<ConsoleWindow>();
    m_statsPanel = std::make_unique<StatsPanel>();
    m_cameraGrid = std::make_unique<CameraGrid>();
    m_cameraGrid->setActiveCameraCount(m_activeCameraCount);
    m_cameraGrid->setViewMode(m_currentViewMode);

    // Setup toolbar buttons
    int buttonX = BUTTON_MARGIN;
    m_toolbarButtons.clear();

    // Start Cameras button
    ButtonRect startBtn;
    startBtn.rect = {buttonX, (TOOLBAR_HEIGHT - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT};
    startBtn.label = "Start Cameras";
    startBtn.hovered = false;
    startBtn.enabled = true;
    m_toolbarButtons.push_back(startBtn);
    buttonX += BUTTON_WIDTH + BUTTON_MARGIN;

    // Stop Cameras button
    ButtonRect stopBtn;
    stopBtn.rect = {buttonX, (TOOLBAR_HEIGHT - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT};
    stopBtn.label = "Stop Cameras";
    stopBtn.hovered = false;
    stopBtn.enabled = false; // Disabled initially
    m_toolbarButtons.push_back(stopBtn);
    buttonX += BUTTON_WIDTH + BUTTON_MARGIN;

    // Restart Cameras button
    ButtonRect restartBtn;
    restartBtn.rect = {buttonX, (TOOLBAR_HEIGHT - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT};
    restartBtn.label = "Restart Cameras";
    restartBtn.hovered = false;
    restartBtn.enabled = false; // Disabled initially
    m_toolbarButtons.push_back(restartBtn);
    buttonX += BUTTON_WIDTH + BUTTON_MARGIN * 3;

    // Console button
    ButtonRect consoleBtn;
    consoleBtn.rect = {buttonX, (TOOLBAR_HEIGHT - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT};
    consoleBtn.label = "Show Console";
    consoleBtn.hovered = false;
    consoleBtn.enabled = true;
    m_toolbarButtons.push_back(consoleBtn);

    m_isInitialized = true;
    spdlog::info("MainWindow initialized successfully ({}x{})", m_windowWidth, m_windowHeight);
    return true;
}

void MainWindow::run() {
    if (!m_isInitialized) {
        spdlog::error("Cannot run: MainWindow not initialized");
        return;
    }

    m_running = true;
    spdlog::info("Starting main application loop");

    // Simulate some initial stats
    m_statsPanel->updateCpuUsage(25.5f);
    m_statsPanel->updateRamUsage(42.3f);
    m_statsPanel->updateLatency(45.2f);

    while (m_running) {
        handleEvents();
        render();
        SDL_Delay(16); // ~60 FPS
    }

    spdlog::info("Main application loop ended");
}

void MainWindow::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Handle console window events if visible
        if (m_consolVisible && m_consoleWindow) {
            m_consoleWindow->handleEvent(event);
        }

        switch (event.type) {
            case SDL_QUIT:
                m_running = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    handleResize(event.window.data1, event.window.data2);
                } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    // Handle main window close button
                    if (event.window.windowID == SDL_GetWindowID(m_window)) {
                        m_running = false;
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseClick(event.button.x, event.button.y);
                }
                break;

            case SDL_MOUSEMOTION: {
                // Update button hover states
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                for (auto& button : m_toolbarButtons) {
                    button.hovered = UIHelpers::pointInRect(mouseX, mouseY, button.rect);
                }
                break;
            }

            case SDL_KEYDOWN:
                handleKeyPress(event.key.keysym.sym);
                break;
        }
    }
}

void MainWindow::handleMouseClick(int x, int y) {
    // Check toolbar button clicks
    for (size_t i = 0; i < m_toolbarButtons.size(); ++i) {
        if (m_toolbarButtons[i].enabled && UIHelpers::pointInRect(x, y, m_toolbarButtons[i].rect)) {
            switch (i) {
                case 0: onStartCamerasClicked(); break;
                case 1: onStopCamerasClicked(); break;
                case 2: onRestartCamerasClicked(); break;
                case 3: onToggleConsoleClicked(); break;
            }
            break;
        }
    }
}

void MainWindow::handleResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    m_mainAreaHeight = height - TOOLBAR_HEIGHT - STATSBAR_HEIGHT;
    spdlog::debug("Window resized to {}x{}", width, height);
}

void MainWindow::handleKeyPress(SDL_Keycode key) {
    switch (key) {
        case SDLK_ESCAPE:
        case SDLK_q:
            m_running = false;
            break;
        case SDLK_1:
            onViewModeChanged(ViewMode::FULLSCREEN);
            break;
        case SDLK_2:
            onViewModeChanged(ViewMode::GRID_2X2);
            break;
        case SDLK_3:
            onViewModeChanged(ViewMode::GRID_NXN);
            break;
        case SDLK_c:
            onToggleConsoleClicked();
            break;
        case SDLK_LEFT:
            // Navigate to previous camera in fullscreen mode
            if (m_currentViewMode == ViewMode::FULLSCREEN && m_activeCameraCount > 0) {
                int currentIndex = m_cameraGrid->getSelectedCameraIndex();
                int newIndex = (currentIndex - 1 + m_activeCameraCount) % m_activeCameraCount;
                m_cameraGrid->setSelectedCameraIndex(newIndex);
                spdlog::info("Switched to camera {}", newIndex + 1);
            }
            break;
        case SDLK_RIGHT:
            // Navigate to next camera in fullscreen mode
            if (m_currentViewMode == ViewMode::FULLSCREEN && m_activeCameraCount > 0) {
                int currentIndex = m_cameraGrid->getSelectedCameraIndex();
                int newIndex = (currentIndex + 1) % m_activeCameraCount;
                m_cameraGrid->setSelectedCameraIndex(newIndex);
                spdlog::info("Switched to camera {}", newIndex + 1);
            }
            break;
    }
}

void MainWindow::render() {
    // Clear screen
    SDL_SetRenderDrawColor(m_renderer, 
        Colors::BACKGROUND.r, Colors::BACKGROUND.g, Colors::BACKGROUND.b, Colors::BACKGROUND.a);
    SDL_RenderClear(m_renderer);

    // Render UI components
    renderToolbar();
    renderMainArea();
    renderStatsBar();

    // Present
    SDL_RenderPresent(m_renderer);

    // Render console if visible
    if (m_consolVisible && m_consoleWindow) {
        m_consoleWindow->render();
    }
}

void MainWindow::renderToolbar() {
    // Toolbar background
    SDL_Rect toolbarRect = {0, 0, m_windowWidth, TOOLBAR_HEIGHT};
    SDL_SetRenderDrawColor(m_renderer, 
        Colors::TOOLBAR_BG.r, Colors::TOOLBAR_BG.g, Colors::TOOLBAR_BG.b, Colors::TOOLBAR_BG.a);
    SDL_RenderFillRect(m_renderer, &toolbarRect);

    // Toolbar border
    SDL_SetRenderDrawColor(m_renderer, 
        Colors::BORDER.r, Colors::BORDER.g, Colors::BORDER.b, Colors::BORDER.a);
    SDL_RenderDrawLine(m_renderer, 0, TOOLBAR_HEIGHT - 1, m_windowWidth, TOOLBAR_HEIGHT - 1);

    // Render buttons
    for (const auto& button : m_toolbarButtons) {
        UIHelpers::drawButton(m_renderer, button.rect, button.label, 
                             button.hovered, false, button.enabled);
    }
}

void MainWindow::renderMainArea() {
    // Main area background
    SDL_Rect mainAreaRect = {0, m_mainAreaY, m_windowWidth, m_mainAreaHeight};
    SDL_SetRenderDrawColor(m_renderer, 
        Colors::BACKGROUND.r, Colors::BACKGROUND.g, Colors::BACKGROUND.b, Colors::BACKGROUND.a);
    SDL_RenderFillRect(m_renderer, &mainAreaRect);

    // Render camera grid
    if (m_cameraGrid) {
        m_cameraGrid->render(m_renderer, 0, m_mainAreaY, m_windowWidth, m_mainAreaHeight);
    }
}

void MainWindow::renderStatsBar() {
    int statsY = m_windowHeight - STATSBAR_HEIGHT;
    
    // Stats bar background
    SDL_Rect statsRect = {0, statsY, m_windowWidth, STATSBAR_HEIGHT};
    SDL_SetRenderDrawColor(m_renderer, 
        Colors::STATSBAR_BG.r, Colors::STATSBAR_BG.g, Colors::STATSBAR_BG.b, Colors::STATSBAR_BG.a);
    SDL_RenderFillRect(m_renderer, &statsRect);

    // Stats bar border
    SDL_SetRenderDrawColor(m_renderer, 
        Colors::BORDER.r, Colors::BORDER.g, Colors::BORDER.b, Colors::BORDER.a);
    SDL_RenderDrawLine(m_renderer, 0, statsY, m_windowWidth, statsY);

    // Render stats
    if (m_statsPanel) {
        m_statsPanel->render(m_renderer, 0, statsY, m_windowWidth, STATSBAR_HEIGHT);
    }
}

void MainWindow::onStartCamerasClicked() {
    spdlog::info("Start Cameras clicked");
    m_toolbarButtons[0].enabled = false; // Disable Start
    m_toolbarButtons[1].enabled = true;  // Enable Stop
    m_toolbarButtons[2].enabled = true;  // Enable Restart
}

void MainWindow::onStopCamerasClicked() {
    spdlog::info("Stop Cameras clicked");
    m_toolbarButtons[0].enabled = true;  // Enable Start
    m_toolbarButtons[1].enabled = false; // Disable Stop
    m_toolbarButtons[2].enabled = false; // Disable Restart
}

void MainWindow::onRestartCamerasClicked() {
    spdlog::info("Restart Cameras clicked");
}

void MainWindow::onToggleConsoleClicked() {
    m_consolVisible = !m_consolVisible;
    
    if (m_consolVisible) {
        // Create console window if it doesn't exist or show it
        if (!m_consoleWindow->isVisible()) {
            int consoleX = 100;
            int consoleY = 100;
            int consoleWidth = 800;
            int consoleHeight = 500;
            m_consoleWindow->create(consoleX, consoleY, consoleWidth, consoleHeight);
            m_consoleWindow->setVisible(true);
        }
        m_toolbarButtons[3].label = "Hide Console";
        spdlog::info("Console window opened");
    } else {
        m_consoleWindow->setVisible(false);
        m_toolbarButtons[3].label = "Show Console";
        spdlog::info("Console window hidden");
    }
}

void MainWindow::onViewModeChanged(ViewMode mode) {
    m_currentViewMode = mode;
    m_cameraGrid->setViewMode(mode);
    
    const char* modeStr = "Unknown";
    switch (mode) {
        case ViewMode::FULLSCREEN: modeStr = "Fullscreen"; break;
        case ViewMode::GRID_2X2: modeStr = "2x2 Grid"; break;
        case ViewMode::GRID_NXN: modeStr = "NxN Grid"; break;
    }
    spdlog::info("View mode changed to: {}", modeStr);
}

void MainWindow::shutdown() {
    if (!m_isInitialized) return;

    spdlog::info("Shutting down MainWindow");

    m_consoleWindow.reset();
    m_statsPanel.reset();
    m_cameraGrid.reset();

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    FontManager::instance().shutdown();
    SDL_Quit();
    m_isInitialized = false;
}

} // namespace camera_viewer
