#include "main_window.h"
#include "console_window.h"
#include "console_sink.h"
#include "stats_panel.h"
#include "camera_grid.h"
#include "camera_manager.h"
#include "ui_helpers.h"
#include "settings_manager.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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
    , m_isFullscreen(false)
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

    // Load settings
    SettingsManager::instance().load();

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

    // Get window position and size from settings
    auto& settings = SettingsManager::instance();
    int windowX = SDL_WINDOWPOS_CENTERED;
    int windowY = SDL_WINDOWPOS_CENTERED;
    
    if (settings.shouldRememberWindowPosition() && settings.getLastWindowX() >= 0) {
        windowX = settings.getLastWindowX();
        windowY = settings.getLastWindowY();
    }
    
    if (settings.shouldRememberWindowSize()) {
        m_windowWidth = settings.getLastWindowWidth();
        m_windowHeight = settings.getLastWindowHeight();
    }

    // Create window
    m_window = SDL_CreateWindow(
        m_title.c_str(),
        windowX,
        windowY,
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
    
    // Initialize camera manager
    m_cameraManager = std::make_unique<CameraManager>(m_renderer);
    m_cameraManager->setCameraConfigs(settings.getCameraConfigs());
    m_cameraManager->setStreamingSettings(settings.getStreamingSettings());
    
    // Connect camera grid to camera manager
    m_cameraGrid->setCameraManager(m_cameraManager.get());
    m_cameraGrid->setViewMode(m_currentViewMode);
    
    // Discover cameras at startup (before main loop)
    spdlog::info("Discovering cameras at startup...");
    int available = m_cameraManager->discoverCameras([](int found, int total) {
        spdlog::debug("Discovery progress: {}/{}", found, total);
    });
    
    // Update camera grid with available indices
    auto availableIndices = m_cameraManager->getAvailableCameraIndices();
    m_cameraGrid->setAvailableCameraIndices(availableIndices);
    m_activeCameraCount = available;
    
    spdlog::info("Found {} available cameras", available);

    // Set up logging to redirect all messages to console window
    auto console_sink = std::make_shared<ConsoleSink>(m_consoleWindow.get());
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    auto logger = std::make_shared<spdlog::logger>("multi_sink", 
        spdlog::sinks_init_list{console_sink, stdout_sink});
    logger->set_level(spdlog::level::trace); // Capture all log levels
    spdlog::set_default_logger(logger);

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
            
            // Sync console visibility state if it was closed
            if (!m_consoleWindow->isVisible() && m_consolVisible) {
                m_consolVisible = false;
                m_toolbarButtons[3].label = "Show Console";
            }
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
    auto& settings = SettingsManager::instance();
    
    // Check keybindings
    if (settings.isKeyForAction(key, "quit") || settings.isKeyForAction(key, "quit_alt")) {
        m_running = false;
    }
    else if (settings.isKeyForAction(key, "toggle_fullscreen")) {
        toggleFullscreen();
    }
    else if (settings.isKeyForAction(key, "toggle_console")) {
        onToggleConsoleClicked();
    }
    else if (settings.isKeyForAction(key, "view_mode_fullscreen")) {
        onViewModeChanged(ViewMode::FULLSCREEN);
    }
    else if (settings.isKeyForAction(key, "view_mode_2x2")) {
        onViewModeChanged(ViewMode::GRID_2X2);
    }
    else if (settings.isKeyForAction(key, "view_mode_grid")) {
        onViewModeChanged(ViewMode::GRID_NXN);
    }
    else if (settings.isKeyForAction(key, "camera_previous")) {
        // Navigate to previous camera in fullscreen mode
        if (m_currentViewMode == ViewMode::FULLSCREEN && m_cameraGrid->getAvailableCameraCount() > 0) {
            int currentIndex = m_cameraGrid->getSelectedCameraIndex();
            int count = m_cameraGrid->getAvailableCameraCount();
            int newIndex = (currentIndex - 1 + count) % count;
            m_cameraGrid->setSelectedCameraIndex(newIndex);
            int realCamNum = m_cameraGrid->getSelectedRealCameraIndex() + 1;
            spdlog::info("Switched to Camera {}", realCamNum);
        }
    }
    else if (settings.isKeyForAction(key, "camera_next")) {
        // Navigate to next camera in fullscreen mode
        if (m_currentViewMode == ViewMode::FULLSCREEN && m_cameraGrid->getAvailableCameraCount() > 0) {
            int currentIndex = m_cameraGrid->getSelectedCameraIndex();
            int count = m_cameraGrid->getAvailableCameraCount();
            int newIndex = (currentIndex + 1) % count;
            m_cameraGrid->setSelectedCameraIndex(newIndex);
            int realCamNum = m_cameraGrid->getSelectedRealCameraIndex() + 1;
            spdlog::info("Switched to Camera {}", realCamNum);
        }
    }
}

void MainWindow::render() {
    // Update camera textures from main thread (SDL requirement)
    if (m_cameraManager) {
        m_cameraManager->updateTexturesFromMainThread();
    }
    
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
    
    if (m_activeCameraCount == 0) {
        spdlog::warn("No cameras available - run discovery first");
        return;
    }
    
    // Start all available camera streams
    m_cameraManager->startAll();
    
    m_toolbarButtons[0].enabled = false; // Disable Start
    m_toolbarButtons[1].enabled = true;  // Enable Stop
    m_toolbarButtons[2].enabled = true;  // Enable Restart
    
    spdlog::info("Started {} camera streams", m_activeCameraCount);
}

void MainWindow::onStopCamerasClicked() {
    spdlog::info("Stop Cameras clicked");
    
    // Stop all camera streams
    if (m_cameraManager) {
        m_cameraManager->stopAll();
    }
    
    // Keep available camera info but mark as stopped
    m_toolbarButtons[0].enabled = true;  // Enable Start
    m_toolbarButtons[1].enabled = false; // Disable Stop
    m_toolbarButtons[2].enabled = false; // Disable Restart
}

void MainWindow::onRestartCamerasClicked() {
    spdlog::info("Restart Cameras clicked");
    
    // Stop first
    if (m_cameraManager) {
        m_cameraManager->stopAll();
    }
    
    // Re-discover cameras
    spdlog::info("Re-discovering cameras...");
    int available = m_cameraManager->discoverCameras([](int found, int total) {
        spdlog::debug("Discovery progress: {}/{}", found, total);
    });
    
    // Update camera grid with available indices
    auto availableIndices = m_cameraManager->getAvailableCameraIndices();
    m_cameraGrid->setAvailableCameraIndices(availableIndices);
    m_activeCameraCount = available;
    
    if (available > 0) {
        m_cameraManager->startAll();
        m_toolbarButtons[0].enabled = false;
        m_toolbarButtons[1].enabled = true;
        m_toolbarButtons[2].enabled = true;
    } else {
        m_toolbarButtons[0].enabled = true;
        m_toolbarButtons[1].enabled = false;
        m_toolbarButtons[2].enabled = false;
    }
    
    spdlog::info("Restarted with {} cameras", available);
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

void MainWindow::toggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;
    
    if (m_isFullscreen) {
        // Enter borderless fullscreen
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        spdlog::info("Entered fullscreen mode");
    } else {
        // Exit fullscreen
        SDL_SetWindowFullscreen(m_window, 0);
        spdlog::info("Exited fullscreen mode");
    }
}

void MainWindow::shutdown() {
    if (!m_isInitialized) return;

    spdlog::info("Shutting down MainWindow");
    
    // Stop all camera streams first
    if (m_cameraManager) {
        m_cameraManager->stopAll();
    }
    
    // Save settings before shutdown
    auto& settings = SettingsManager::instance();
    
    if (!m_isFullscreen) {
        // Save window position and size (only when not in fullscreen)
        int x, y;
        SDL_GetWindowPosition(m_window, &x, &y);
        settings.setLastWindowPosition(x, y);
        settings.setLastWindowSize(m_windowWidth, m_windowHeight);
    }
    
    settings.save();

    m_cameraManager.reset();
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
