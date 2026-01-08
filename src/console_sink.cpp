#include "console_sink.h"
#include "console_window.h"
#include "ui_helpers.h"
#include <spdlog/spdlog.h>

namespace camera_viewer {

ConsoleSink::ConsoleSink(ConsoleWindow* consoleWindow)
    : m_consoleWindow(consoleWindow)
{
}

void ConsoleSink::sink_it_(const spdlog::details::log_msg& msg) {
    if (!m_consoleWindow) {
        return;
    }

    // Format the message
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    std::string message(formatted.data(), formatted.size());
    
    // Remove trailing newline if present
    if (!message.empty() && message.back() == '\n') {
        message.pop_back();
    }

    // Choose color based on log level
    SDL_Color color;
    switch (msg.level) {
        case spdlog::level::trace:
        case spdlog::level::debug:
            color = Colors::CONSOLE_DEBUG;
            break;
        case spdlog::level::info:
            color = Colors::CONSOLE_INFO;
            break;
        case spdlog::level::warn:
            color = Colors::CONSOLE_WARNING;
            break;
        case spdlog::level::err:
        case spdlog::level::critical:
            color = Colors::CONSOLE_ERROR;
            break;
        default:
            color = Colors::CONSOLE_TEXT;
            break;
    }

    // Add message to console window
    m_consoleWindow->addLog(message, color);
}

void ConsoleSink::flush_() {
    // Nothing to flush for our console window
}

} // namespace camera_viewer
