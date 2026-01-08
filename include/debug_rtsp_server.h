#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace camera_viewer {

/**
 * @brief Debug RTSP server that streams webcam feed
 * 
 * This class captures webcam input and streams it via RTSP for testing purposes.
 * Activated with --debug flag.
 */
class DebugRTSPServer {
public:
    DebugRTSPServer();
    ~DebugRTSPServer();

    /**
     * @brief Start the RTSP server with webcam capture
     * @param deviceIndex Webcam device index (default 0)
     * @param width Video width (default 1920)
     * @param height Video height (default 1080)
     * @param fps Frame rate (default 30)
     * @param rtspPort RTSP server port (default 8554)
     * @return true if started successfully
     */
    bool start(int deviceIndex = 0, int width = 1920, int height = 1080, int fps = 30, int rtspPort = 8554);

    /**
     * @brief Stop the RTSP server
     */
    void stop();

    /**
     * @brief Check if server is running
     */
    bool isRunning() const { return m_running.load(); }

    /**
     * @brief Get the RTSP URL for connecting
     */
    std::string getRTSPUrl() const { return m_rtspUrl; }

private:
    void captureLoop();
    bool initializeCapture(int deviceIndex, int width, int height, int fps);
    bool initializeEncoder(int width, int height, int fps);
    void cleanup();

    std::atomic<bool> m_running;
    std::unique_ptr<std::thread> m_captureThread;
    
    // FFmpeg contexts
    AVFormatContext* m_inputFormatCtx;
    AVFormatContext* m_outputFormatCtx;
    AVCodecContext* m_decoderCtx;
    AVCodecContext* m_encoderCtx;
    SwsContext* m_swsCtx;
    
    int m_videoStreamIndex;
    int m_width;
    int m_height;
    int m_fps;
    int m_rtspPort;
    std::string m_rtspUrl;
};

} // namespace camera_viewer
