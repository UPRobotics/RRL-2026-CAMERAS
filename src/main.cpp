#include <iostream>
#include <algorithm>
#include <spdlog/spdlog.h>
#include "main_window.h"
#include "settings_manager.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    // Check for debug flag
    bool debugMode = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            debugMode = true;
            break;
        }
    }

    // Set log level to trace to capture all messages
    spdlog::set_level(spdlog::level::trace);
    
    spdlog::info("=== C++ RTSP Camera Viewer ===");
    spdlog::info("Starting application...");
    
    // Load settings
    camera_viewer::SettingsManager::instance().load();
    
#ifdef _WIN32
    HANDLE rtspStdoutRead = NULL;
    HANDLE rtspStdoutWrite = NULL;
    PROCESS_INFORMATION rtspServerProcess = {};
    HANDLE rtspOutputThread = NULL;
    HANDLE rtspJobObject = NULL;
    
    // Launch RTSP server if in debug mode
    if (debugMode) {
        spdlog::info("========================================");
        spdlog::info("DEBUG MODE ENABLED");
        spdlog::info("========================================");
        spdlog::info("Launching debug RTSP server (headless)...");
        
        // Create a job object to manage all child processes
        rtspJobObject = CreateJobObjectW(NULL, NULL);
        if (rtspJobObject) {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(rtspJobObject, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
        }
        
        // Create pipe for stdout/stderr redirection
        SECURITY_ATTRIBUTES sa = {};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        
        if (!CreatePipe(&rtspStdoutRead, &rtspStdoutWrite, &sa, 0)) {
            spdlog::error("Failed to create pipe for RTSP server output");
        } else {
            // Ensure read handle is not inherited
            SetHandleInformation(rtspStdoutRead, HANDLE_FLAG_INHERIT, 0);
            
            STARTUPINFOW si = {};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdOutput = rtspStdoutWrite;
            si.hStdError = rtspStdoutWrite;
            si.wShowWindow = SW_HIDE;
            
            std::wstring command = L"powershell.exe -WindowStyle Hidden -ExecutionPolicy Bypass -File debug-webcam-rtsp.ps1";
            
            if (CreateProcessW(
                NULL,
                const_cast<LPWSTR>(command.c_str()),
                NULL,
                NULL,
                TRUE,  // Inherit handles
                CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB,
                NULL,
                NULL,
                &si,
                &rtspServerProcess
            )) {
                CloseHandle(rtspStdoutWrite);  // Close write end in parent
                
                // Assign process to job object
                if (rtspJobObject) {
                    AssignProcessToJobObject(rtspJobObject, rtspServerProcess.hProcess);
                }
                
                // Create thread to read RTSP server output
                rtspOutputThread = CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
                    HANDLE hRead = (HANDLE)param;
                    char buffer[4096];
                    DWORD bytesRead;
                    
                    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        std::string output(buffer);
                        
                        // Filter out verbose messages, only log important info
                        if (output.find("ERROR") != std::string::npos || 
                            output.find("Error") != std::string::npos ||
                            output.find("RTSP stream") != std::string::npos ||
                            output.find("Starting") != std::string::npos) {
                            
                            // Remove newlines and log
                            output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());
                            output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
                            if (!output.empty()) {
                                spdlog::info("[RTSP Server] {}", output);
                            }
                        }
                    }
                    return 0;
                }, rtspStdoutRead, 0, NULL);
                
                spdlog::info("RTSP server launched (headless mode)");
                spdlog::info("RTSP URL: {}", camera_viewer::SettingsManager::instance().getDebugRtspUrl());
                spdlog::info("Virtual cameras: {}", camera_viewer::SettingsManager::instance().getDebugVirtualCameraCount());
                spdlog::info("========================================");
                
                // Give the server a moment to start
                Sleep(3000);
            } else {
                CloseHandle(rtspStdoutRead);
                CloseHandle(rtspStdoutWrite);
                spdlog::error("Failed to launch RTSP server");
                spdlog::warn("You can manually start it with: .\\debug-webcam-rtsp.ps1");
            }
        }
    }
#endif
    
    // Create main window
    camera_viewer::MainWindow mainWindow("RTSP Camera Viewer - Multi-Camera Monitor", 1280, 720);
    
    // Initialize
    if (!mainWindow.initialize()) {
        spdlog::error("Failed to initialize main window");
        return 1;
    }
    
    // Pass debug mode to main window
    if (debugMode) {
        mainWindow.setDebugMode(true);
    }
    
    // Run application
    mainWindow.run();
    
    // Cleanup
    mainWindow.shutdown();
    
#ifdef _WIN32
    // Terminate RTSP server if we launched it
    if (debugMode && rtspServerProcess.hProcess) {
        spdlog::info("Stopping RTSP server...");
        
        // Closing the job object will kill all child processes
        if (rtspJobObject) {
            CloseHandle(rtspJobObject);
            spdlog::info("Job object closed - all child processes terminated");
        }
        
        // Stop the output thread
        if (rtspOutputThread) {
            WaitForSingleObject(rtspOutputThread, 1000);
            CloseHandle(rtspOutputThread);
        }
        
        // Terminate the PowerShell process
        TerminateProcess(rtspServerProcess.hProcess, 0);
        WaitForSingleObject(rtspServerProcess.hProcess, 1000);
        
        CloseHandle(rtspServerProcess.hProcess);
        CloseHandle(rtspServerProcess.hThread);
        
        if (rtspStdoutRead) {
            CloseHandle(rtspStdoutRead);
        }
        
        // Wait a moment for processes to clean up
        Sleep(500);
        
        // Final check - force kill any remaining processes
        system("taskkill /F /IM ffmpeg.exe >nul 2>&1");
        system("taskkill /F /IM mediamtx.exe >nul 2>&1");
        
        spdlog::info("RTSP server stopped");
    }
#endif
    
    spdlog::info("Application terminated successfully");
    return 0;
}
