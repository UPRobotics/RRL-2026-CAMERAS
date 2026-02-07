#include "camera_stream.h"
#include <spdlog/spdlog.h>
#include <chrono>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext.h>
}

namespace camera_viewer {

CameraStream::CameraStream(int cameraIndex, const CameraConfig& config, SDL_Renderer* renderer, DecodeMode decodeMode)
    : m_cameraIndex(cameraIndex)
    , m_config(config)
    , m_currentQuality(StreamQuality::High)
    , m_decodeMode(decodeMode)
    , m_renderer(renderer)
{
    m_stats.state = CameraState::Disconnected;
    m_lastFpsUpdate = std::chrono::steady_clock::now();
}

CameraStream::~CameraStream() {
    stop();
    
    std::lock_guard<std::mutex> lock(m_textureMutex);
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

bool CameraStream::start(StreamQuality quality) {
    if (m_running.load()) {
        spdlog::warn("Camera {} stream already running", m_cameraIndex + 1);
        return true;
    }
    
    m_currentQuality = quality;
    m_stopRequested = false;
    m_running = true;
    m_reconnectAttempts = 0;
    
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.state = CameraState::Connecting;
        m_stats.total_frames = 0;
        m_stats.dropped_frames = 0;
        m_stats.current_fps = 0;
        m_stats.reconnect_count = 0;
        m_stats.last_frame_time = std::chrono::steady_clock::now();  // Initialize to prevent false timeout
    }
    
    spdlog::info("Starting camera {} stream ({})", 
                 m_cameraIndex + 1, 
                 quality == StreamQuality::High ? "1080p" : "480p");
    
    m_thread = std::thread(&CameraStream::decoderThread, this);
    return true;
}

void CameraStream::stop() {
    if (!m_running.load()) {
        return;
    }
    
    spdlog::info("Stopping camera {} stream", m_cameraIndex + 1);
    
    m_stopRequested = true;
    m_running = false;
    
    if (m_thread.joinable()) {
        m_thread.join();
    }
    
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.state = CameraState::Disconnected;
    }
}

CameraStats CameraStream::getStats() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_stats;
}

SDL_Texture* CameraStream::getFrameTexture() {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    return m_texture;
}

void CameraStream::setFrameCallback(FrameCallback callback) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_frameCallback = std::move(callback);
}

void CameraStream::setQuality(StreamQuality quality) {
    if (m_currentQuality == quality) {
        return;
    }
    
    spdlog::info("Camera {} switching to {} quality", 
                 m_cameraIndex + 1,
                 quality == StreamQuality::High ? "high" : "low");
    
    // Stop and restart with new quality
    bool wasRunning = m_running.load();
    if (wasRunning) {
        stop();
        m_currentQuality = quality;
        start(quality);
    } else {
        m_currentQuality = quality;
    }
}

void CameraStream::decoderThread() {
    spdlog::debug("Camera {} decoder thread started", m_cameraIndex + 1);
    
    const std::string& url = (m_currentQuality == StreamQuality::High) 
                             ? m_config.url_highres 
                             : m_config.url_lowres;
    
    while (!m_stopRequested.load()) {
        // Try to connect
        if (!initDecoder(url)) {
            spdlog::error("Camera {} failed to initialize decoder for {}", m_cameraIndex + 1, url);
            attemptReconnect();
            continue;
        }
        
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.state = CameraState::Connected;
        }
        
        spdlog::info("Camera {} connected successfully", m_cameraIndex + 1);
        
        // Main decode loop
        while (!m_stopRequested.load() && decodeFrame()) {
            // Frame decoded successfully
        }
        
        // Connection lost or stopped
        cleanupDecoder();
        
        if (!m_stopRequested.load()) {
            attemptReconnect();
        }
    }
    
    cleanupDecoder();
    spdlog::debug("Camera {} decoder thread ended", m_cameraIndex + 1);
}

static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts) {
    (void)ctx;
    const enum AVPixelFormat* p;
    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == AV_PIX_FMT_CUDA) {
            return *p;
        }
    }
    return pix_fmts[0];
}

bool CameraStream::initDecoder(const std::string& url) {
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.state = CameraState::Connecting;
    }
    
    // Allocate format context
    m_formatCtx = avformat_alloc_context();
    if (!m_formatCtx) {
        spdlog::error("Camera {} failed to allocate format context", m_cameraIndex + 1);
        return false;
    }
    
    // Set RTSP options for low latency and compatibility
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);  // Use TCP (required by this camera)
    av_dict_set(&opts, "rtsp_flags", "prefer_tcp", 0); // Prefer TCP transport
    av_dict_set(&opts, "stimeout", "10000000", 0);   // 10 second timeout (in microseconds)
    av_dict_set(&opts, "analyzeduration", "1000000", 0); // 1 second analyze
    av_dict_set(&opts, "probesize", "1000000", 0);   // 1MB probe size
    av_dict_set(&opts, "fflags", "nobuffer+discardcorrupt", 0); // Reduce buffering, discard corrupt
    av_dict_set(&opts, "flags", "low_delay", 0);     // Low delay mode
    av_dict_set(&opts, "max_delay", "500000", 0);    // 500ms max delay
    av_dict_set(&opts, "reorder_queue_size", "0", 0); // No reorder buffer
    av_dict_set(&opts, "buffer_size", "1024000", 0); // 1MB buffer
    
    spdlog::info("Camera {} connecting to: {}", m_cameraIndex + 1, url);
    
    // Open input stream
    int ret = avformat_open_input(&m_formatCtx, url.c_str(), nullptr, &opts);
    av_dict_free(&opts);
    
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        spdlog::error("Camera {} failed to open stream: {}", m_cameraIndex + 1, errBuf);
        avformat_free_context(m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }
    
    // Find stream info
    ret = avformat_find_stream_info(m_formatCtx, nullptr);
    if (ret < 0) {
        spdlog::error("Camera {} failed to find stream info", m_cameraIndex + 1);
        avformat_close_input(&m_formatCtx);
        return false;
    }
    
    // Find video stream
    m_videoStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            break;
        }
    }
    
    if (m_videoStreamIndex == -1) {
        spdlog::error("Camera {} no video stream found", m_cameraIndex + 1);
        avformat_close_input(&m_formatCtx);
        return false;
    }
    
    // Find decoder (CPU or GPU)
    const AVCodec* codec = nullptr;
    const AVCodecID cid = m_formatCtx->streams[m_videoStreamIndex]->codecpar->codec_id;
    if (m_decodeMode == DecodeMode::GPU) {
        const char* name = nullptr;
        switch (cid) {
            case AV_CODEC_ID_H264: name = "h264_cuvid"; break;
            case AV_CODEC_ID_HEVC: name = "hevc_cuvid"; break;
            case AV_CODEC_ID_MPEG2VIDEO: name = "mpeg2_cuvid"; break;
            case AV_CODEC_ID_MPEG4: name = "mpeg4_cuvid"; break;
            case AV_CODEC_ID_VP8: name = "vp8_cuvid"; break;
            case AV_CODEC_ID_VP9: name = "vp9_cuvid"; break;
            case AV_CODEC_ID_AV1: name = "av1_cuvid"; break;
            default: break;
        }
        if (name) {
            codec = avcodec_find_decoder_by_name(name);
            if (!codec) {
                spdlog::warn("Camera {} GPU decoder {} not found, falling back to CPU", m_cameraIndex + 1, name);
            }
        }
    }
    if (!codec) {
        codec = avcodec_find_decoder(cid);
    }
    if (!codec) {
        spdlog::error("Camera {} unsupported codec", m_cameraIndex + 1);
        avformat_close_input(&m_formatCtx);
        return false;
    }
    
    // Allocate codec context
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        spdlog::error("Camera {} failed to allocate codec context", m_cameraIndex + 1);
        avformat_close_input(&m_formatCtx);
        return false;
    }
    
    // Copy codec parameters
    ret = avcodec_parameters_to_context(m_codecCtx, 
        m_formatCtx->streams[m_videoStreamIndex]->codecpar);
    if (ret < 0) {
        spdlog::error("Camera {} failed to copy codec parameters", m_cameraIndex + 1);
        avcodec_free_context(&m_codecCtx);
        avformat_close_input(&m_formatCtx);
        return false;
    }
    
    // Hardware device (CUDA) if requested
    if (m_decodeMode == DecodeMode::GPU) {
        if (av_hwdevice_ctx_create(&m_hwDeviceCtx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0) < 0) {
            spdlog::warn("Camera {} failed to create CUDA hwdevice, falling back to CPU", m_cameraIndex + 1);
            m_decodeMode = DecodeMode::CPU;
        } else {
            m_codecCtx->get_format = get_hw_format;
            m_codecCtx->hw_device_ctx = av_buffer_ref(m_hwDeviceCtx);
        }
    }

    // Set codec options for low latency
    m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    m_codecCtx->flags2 |= AV_CODEC_FLAG2_FAST;
    
    // Open codec
    ret = avcodec_open2(m_codecCtx, codec, nullptr);
    if (ret < 0) {
        spdlog::error("Camera {} failed to open codec", m_cameraIndex + 1);
        avcodec_free_context(&m_codecCtx);
        avformat_close_input(&m_formatCtx);
        if (m_hwDeviceCtx) {
            av_buffer_unref(&m_hwDeviceCtx);
            m_hwDeviceCtx = nullptr;
        }
        return false;
    }
    
    // Allocate frame
    m_frame = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    m_packet = av_packet_alloc();
    
    if (!m_frame || !m_frameRGB || !m_packet) {
        spdlog::error("Camera {} failed to allocate frame/packet", m_cameraIndex + 1);
        cleanupDecoder();
        return false;
    }
    
    // Store frame dimensions in stats
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.frame_width = m_codecCtx->width;
        m_stats.frame_height = m_codecCtx->height;
    }
    
    spdlog::info("Camera {} decoder initialized: {}x{} @ {}", 
                 m_cameraIndex + 1, m_codecCtx->width, m_codecCtx->height,
                 avcodec_get_name(codec->id));
    
    return true;
}

void CameraStream::cleanupDecoder() {
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }

    if (m_hwDeviceCtx) {
        av_buffer_unref(&m_hwDeviceCtx);
        m_hwDeviceCtx = nullptr;
    }
    
    if (m_packet) {
        av_packet_free(&m_packet);
        m_packet = nullptr;
    }
    
    if (m_frameRGB) {
        // Free the buffer we allocated with av_malloc
        if (m_frameRGB->data[0]) {
            av_freep(&m_frameRGB->data[0]);
        }
        av_frame_free(&m_frameRGB);
        m_frameRGB = nullptr;
    }
    
    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
    
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
    
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
    }
    
    m_videoStreamIndex = -1;
    m_formatLogged = false; // Reset for next connection
}

bool CameraStream::decodeFrame() {
    // Read packet
    int ret = av_read_frame(m_formatCtx, m_packet);
    if (ret < 0) {
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
            return false; // End of stream or would block
        }
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        spdlog::warn("Camera {} read frame error: {}", m_cameraIndex + 1, errBuf);
        return false;
    }
    
    // Check if this is a video packet
    if (m_packet->stream_index != m_videoStreamIndex) {
        av_packet_unref(m_packet);
        return true; // Skip non-video packets
    }
    
    // Send packet to decoder
    ret = avcodec_send_packet(m_codecCtx, m_packet);
    av_packet_unref(m_packet);
    
    if (ret < 0) {
        spdlog::warn("Camera {} send packet error", m_cameraIndex + 1);
        return ret == AVERROR(EAGAIN); // Continue if decoder is full
    }
    
    // Receive decoded frame
    ret = avcodec_receive_frame(m_codecCtx, m_frame);
    if (ret < 0) {
        if (ret == AVERROR(EAGAIN)) {
            return true; // Need more packets
        }
        return false; // Error
    }

    AVFrame* usableFrame = m_frame;
    AVFrame* swFrame = nullptr;
    if (m_decodeMode == DecodeMode::GPU && m_frame->format == AV_PIX_FMT_CUDA) {
        swFrame = av_frame_alloc();
        if (swFrame && av_hwframe_transfer_data(swFrame, m_frame, 0) == 0) {
            usableFrame = swFrame; // use downloaded frame
        } else {
            spdlog::warn("Camera {} failed to transfer hw frame to system memory, skipping", m_cameraIndex + 1);
            if (swFrame) av_frame_free(&swFrame);
            av_frame_unref(m_frame);
            return true;
        }
    }
    
    // Update texture with decoded frame
    if (!updateTexture(usableFrame)) {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.dropped_frames++;
    } else {
        // Update stats
        auto now = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.total_frames++;
            m_stats.last_frame_time = now;
            m_frameCountSinceLastUpdate++;
            
            // Calculate FPS every second
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - m_lastFpsUpdate);
            if (elapsed.count() >= 1000) {
                m_stats.current_fps = m_frameCountSinceLastUpdate * 1000.0f / elapsed.count();
                m_frameCountSinceLastUpdate = 0;
                m_lastFpsUpdate = now;
            }
        }
        
        // Call frame callback if set
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            if (m_frameCallback) {
                std::lock_guard<std::mutex> texLock(m_textureMutex);
                m_frameCallback(m_cameraIndex, m_texture, getStats());
            }
        }
    }
    
    if (swFrame) {
        av_frame_free(&swFrame);
    }
    av_frame_unref(m_frame);
    return true;
}

bool CameraStream::updateTexture(AVFrame* frame) {
    if (!frame || frame->width == 0 || frame->height == 0) {
        spdlog::warn("Camera {} updateTexture: invalid frame (null or zero size)", m_cameraIndex + 1);
        return false;
    }
    
    AVPixelFormat srcFormat = static_cast<AVPixelFormat>(frame->format);
    
    // Log pixel format on first frame for debugging
    if (!m_formatLogged) {
        spdlog::info("Camera {} FRAME DEBUG:", m_cameraIndex + 1);
        spdlog::info("  Format: {} ({})", av_get_pix_fmt_name(srcFormat), frame->format);
        spdlog::info("  Size: {}x{}", frame->width, frame->height);
        spdlog::info("  Linesize[0]: {}, Linesize[1]: {}, Linesize[2]: {}", 
                     frame->linesize[0], frame->linesize[1], frame->linesize[2]);
        spdlog::info("  Data[0]: {:p}, Data[1]: {:p}, Data[2]: {:p}", 
                     (void*)frame->data[0], (void*)frame->data[1], (void*)frame->data[2]);
        spdlog::info("  Key frame: {}", frame->key_frame);
        m_formatLogged = true;
    }
    
    // Use BGRA format which has best SDL support across platforms
    const AVPixelFormat targetFormat = AV_PIX_FMT_BGRA;
    
    // Initialize swscale context to convert to BGRA
    if (!m_swsCtx) {
        spdlog::info("Camera {} creating swscale context: {} -> BGRA", 
                    m_cameraIndex + 1, av_get_pix_fmt_name(srcFormat));
        
        m_swsCtx = sws_getContext(
            frame->width, frame->height, srcFormat,
            frame->width, frame->height, targetFormat,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
        
        if (!m_swsCtx) {
            spdlog::error("Camera {} failed to create swscale context", m_cameraIndex + 1);
            return false;
        }
        
        // Allocate BGRA frame buffer (4 bytes per pixel)
        int bufSize = av_image_get_buffer_size(targetFormat, frame->width, frame->height, 1);
        spdlog::info("Camera {} allocating BGRA buffer: {} bytes", m_cameraIndex + 1, bufSize);
        
        uint8_t* buffer = static_cast<uint8_t*>(av_malloc(bufSize));
        if (!buffer) {
            spdlog::error("Camera {} failed to allocate BGRA buffer", m_cameraIndex + 1);
            sws_freeContext(m_swsCtx);
            m_swsCtx = nullptr;
            return false;
        }
        
        av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize, buffer,
                            targetFormat, frame->width, frame->height, 1);
        m_frameRGB->width = frame->width;
        m_frameRGB->height = frame->height;
        m_frameRGB->format = targetFormat;
        
        spdlog::info("Camera {} BGRA buffer linesize: {}", m_cameraIndex + 1, m_frameRGB->linesize[0]);
    }
    
    // Convert frame to BGRA
    int scaledHeight = sws_scale(m_swsCtx, 
              frame->data, frame->linesize, 0, frame->height,
              m_frameRGB->data, m_frameRGB->linesize);
    
    if (scaledHeight != frame->height) {
        spdlog::warn("Camera {} sws_scale returned {}, expected {}", 
                    m_cameraIndex + 1, scaledHeight, frame->height);
    }
    
    // Debug: check first few pixels of BGRA data (B, G, R, A)
    static int debugCount = 0;
    if (debugCount < 3) {
        uint8_t* data = m_frameRGB->data[0];
        spdlog::info("Camera {} BGRA pixels: [{},{},{},{}] [{},{},{},{}]",
                    m_cameraIndex + 1,
                    data[0], data[1], data[2], data[3],
                    data[4], data[5], data[6], data[7]);
        debugCount++;
    }
    
    // Store frame data for main thread to update texture
    // (SDL textures MUST be updated from the thread that created the renderer)
    {
        std::lock_guard<std::mutex> lock(m_pendingFrameMutex);
        int dataSize = m_frameRGB->linesize[0] * frame->height;
        m_pendingFrameData.resize(dataSize);
        memcpy(m_pendingFrameData.data(), m_frameRGB->data[0], dataSize);
        m_pendingFrameWidth = frame->width;
        m_pendingFrameHeight = frame->height;
        m_pendingFramePitch = m_frameRGB->linesize[0];
        m_hasPendingFrame.store(true);
    }
    
    return true;
}

bool CameraStream::updateTextureFromMainThread() {
    // This function MUST be called from the main/render thread
    if (!m_hasPendingFrame.load()) {
        return false;
    }
    
    std::lock_guard<std::mutex> frameLock(m_pendingFrameMutex);
    std::lock_guard<std::mutex> texLock(m_textureMutex);
    
    if (m_pendingFrameData.empty()) {
        return false;
    }
    
    // Create texture if needed (must be done from main thread)
    if (!m_texture) {
        spdlog::info("Camera {} creating SDL texture {}x{} ARGB8888 (from main thread)", 
                    m_cameraIndex + 1, m_pendingFrameWidth, m_pendingFrameHeight);
        
        m_texture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            m_pendingFrameWidth,
            m_pendingFrameHeight
        );
        
        if (!m_texture) {
            spdlog::error("Camera {} failed to create texture: {}", 
                         m_cameraIndex + 1, SDL_GetError());
            return false;
        }
        
        // Query texture to verify format
        Uint32 actualFormat;
        int access, w, h;
        SDL_QueryTexture(m_texture, &actualFormat, &access, &w, &h);
        spdlog::info("Camera {} texture created: format=0x{:08X}, access={}, size={}x{}", 
                    m_cameraIndex + 1, actualFormat, access, w, h);
    }
    
    // Update texture with pending frame data
    int ret = SDL_UpdateTexture(
        m_texture,
        nullptr,
        m_pendingFrameData.data(),
        m_pendingFramePitch
    );
    
    if (ret < 0) {
        spdlog::warn("Camera {} failed to update texture: {}", 
                    m_cameraIndex + 1, SDL_GetError());
        return false;
    }
    
    m_hasPendingFrame.store(false);
    return true;
}

void CameraStream::attemptReconnect() {
    m_reconnectAttempts++;
    
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.state = CameraState::Reconnecting;
        m_stats.reconnect_count++;
    }
    
    if (m_reconnectAttempts > MAX_RECONNECT_ATTEMPTS) {
        spdlog::error("Camera {} max reconnect attempts reached, giving up", m_cameraIndex + 1);
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.state = CameraState::Error;
        }
        m_stopRequested = true;
        return;
    }
    
    spdlog::warn("Camera {} reconnecting (attempt {}/{})", 
                 m_cameraIndex + 1, m_reconnectAttempts, MAX_RECONNECT_ATTEMPTS);
    
    // Wait before reconnecting
    std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
}

} // namespace camera_viewer
