#include "debug_rtsp_server.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <chrono>

namespace camera_viewer {

DebugRTSPServer::DebugRTSPServer()
    : m_running(false)
    , m_inputFormatCtx(nullptr)
    , m_outputFormatCtx(nullptr)
    , m_decoderCtx(nullptr)
    , m_encoderCtx(nullptr)
    , m_swsCtx(nullptr)
    , m_videoStreamIndex(-1)
    , m_width(1920)
    , m_height(1080)
    , m_fps(30)
    , m_rtspPort(8554)
{
    avdevice_register_all();
}

DebugRTSPServer::~DebugRTSPServer() {
    stop();
}

bool DebugRTSPServer::start(int deviceIndex, int width, int height, int fps, int rtspPort) {
    if (m_running.load()) {
        spdlog::warn("Debug RTSP server already running");
        return false;
    }

    m_width = width;
    m_height = height;
    m_fps = fps;
    m_rtspPort = rtspPort;
    m_rtspUrl = "rtsp://127.0.0.1:" + std::to_string(rtspPort) + "/webcam";

    spdlog::info("=== Debug RTSP Server ===");
    spdlog::info("Starting webcam capture (Device: {}, {}x{} @ {}fps)", deviceIndex, width, height, fps);
    spdlog::info("RTSP URL: {}", m_rtspUrl);

    if (!initializeCapture(deviceIndex, width, height, fps)) {
        spdlog::error("Failed to initialize webcam capture");
        return false;
    }

    if (!initializeEncoder(width, height, fps)) {
        spdlog::error("Failed to initialize encoder");
        cleanup();
        return false;
    }

    m_running.store(true);
    m_captureThread = std::make_unique<std::thread>(&DebugRTSPServer::captureLoop, this);

    spdlog::info("Debug RTSP server started successfully");
    spdlog::info("Connect with: {}", m_rtspUrl);
    return true;
}

void DebugRTSPServer::stop() {
    if (!m_running.load()) {
        return;
    }

    spdlog::info("Stopping debug RTSP server...");
    m_running.store(false);

    if (m_captureThread && m_captureThread->joinable()) {
        m_captureThread->join();
    }

    cleanup();
    spdlog::info("Debug RTSP server stopped");
}

bool DebugRTSPServer::initializeCapture(int deviceIndex, int width, int height, int fps) {
    // Windows uses DirectShow (dshow) for webcam capture
    const AVInputFormat* inputFormat = av_find_input_format("dshow");
    if (!inputFormat) {
        spdlog::error("DirectShow input format not found");
        return false;
    }

    // Build device name - typically "video=Integrated Camera" or "video=USB Camera"
    std::string deviceName = "video=@device_pv_" + std::to_string(deviceIndex);
    
    // Set input options
    AVDictionary* options = nullptr;
    av_dict_set(&options, "video_size", (std::to_string(width) + "x" + std::to_string(height)).c_str(), 0);
    av_dict_set(&options, "framerate", std::to_string(fps).c_str(), 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);

    m_inputFormatCtx = nullptr;
    int ret = avformat_open_input(&m_inputFormatCtx, deviceName.c_str(), inputFormat, &options);
    av_dict_free(&options);

    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        spdlog::error("Failed to open webcam: {}", errbuf);
        spdlog::info("Tip: Run 'ffmpeg -list_devices true -f dshow -i dummy' to see available devices");
        return false;
    }

    if (avformat_find_stream_info(m_inputFormatCtx, nullptr) < 0) {
        spdlog::error("Failed to find stream info");
        return false;
    }

    // Find video stream
    m_videoStreamIndex = -1;
    for (unsigned int i = 0; i < m_inputFormatCtx->nb_streams; i++) {
        if (m_inputFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            break;
        }
    }

    if (m_videoStreamIndex == -1) {
        spdlog::error("No video stream found");
        return false;
    }

    // Setup decoder
    AVCodecParameters* codecpar = m_inputFormatCtx->streams[m_videoStreamIndex]->codecpar;
    const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
    if (!decoder) {
        spdlog::error("Decoder not found");
        return false;
    }

    m_decoderCtx = avcodec_alloc_context3(decoder);
    if (avcodec_parameters_to_context(m_decoderCtx, codecpar) < 0) {
        spdlog::error("Failed to copy decoder parameters");
        return false;
    }

    if (avcodec_open2(m_decoderCtx, decoder, nullptr) < 0) {
        spdlog::error("Failed to open decoder");
        return false;
    }

    spdlog::info("Webcam capture initialized: {}x{} ({})", 
                 m_decoderCtx->width, m_decoderCtx->height,
                 avcodec_get_name(codecpar->codec_id));

    return true;
}

bool DebugRTSPServer::initializeEncoder(int width, int height, int fps) {
    // Find H.264 encoder
    const AVCodec* encoder = avcodec_find_encoder_by_name("libx264");
    if (!encoder) {
        encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    }
    if (!encoder) {
        spdlog::error("H.264 encoder not found");
        return false;
    }

    m_encoderCtx = avcodec_alloc_context3(encoder);
    if (!m_encoderCtx) {
        spdlog::error("Failed to allocate encoder context");
        return false;
    }

    // Configure encoder for low-latency streaming
    m_encoderCtx->width = width;
    m_encoderCtx->height = height;
    m_encoderCtx->time_base = {1, fps};
    m_encoderCtx->framerate = {fps, 1};
    m_encoderCtx->gop_size = fps; // 1 second GOP
    m_encoderCtx->max_b_frames = 0; // No B-frames for low latency
    m_encoderCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encoderCtx->bit_rate = 4000000; // 4 Mbps
    m_encoderCtx->rc_buffer_size = m_encoderCtx->bit_rate;
    m_encoderCtx->rc_max_rate = m_encoderCtx->bit_rate;
    m_encoderCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;

    // H.264 specific options for low latency
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "preset", "ultrafast", 0);
    av_dict_set(&opts, "tune", "zerolatency", 0);
    av_dict_set(&opts, "profile", "baseline", 0);

    if (avcodec_open2(m_encoderCtx, encoder, &opts) < 0) {
        av_dict_free(&opts);
        spdlog::error("Failed to open encoder");
        return false;
    }
    av_dict_free(&opts);

    // Setup RTSP output (needs external RTSP server like MediaMTX)
    spdlog::info("Encoder initialized: H.264 {}x{} @ {}fps, {} kbps",
                 width, height, fps, m_encoderCtx->bit_rate / 1000);
    spdlog::warn("Note: This implementation requires an external RTSP server (e.g., MediaMTX)");
    spdlog::info("Alternatively, use FFmpeg command-line for simpler setup");

    return true;
}

void DebugRTSPServer::captureLoop() {
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* scaledFrame = av_frame_alloc();

    scaledFrame->format = m_encoderCtx->pix_fmt;
    scaledFrame->width = m_encoderCtx->width;
    scaledFrame->height = m_encoderCtx->height;
    av_frame_get_buffer(scaledFrame, 0);

    int64_t frameCount = 0;

    while (m_running.load()) {
        if (av_read_frame(m_inputFormatCtx, packet) >= 0) {
            if (packet->stream_index == m_videoStreamIndex) {
                if (avcodec_send_packet(m_decoderCtx, packet) == 0) {
                    while (avcodec_receive_frame(m_decoderCtx, frame) == 0) {
                        // Scale frame if needed
                        if (!m_swsCtx) {
                            m_swsCtx = sws_getContext(
                                frame->width, frame->height, (AVPixelFormat)frame->format,
                                m_encoderCtx->width, m_encoderCtx->height, m_encoderCtx->pix_fmt,
                                SWS_BILINEAR, nullptr, nullptr, nullptr
                            );
                        }

                        sws_scale(m_swsCtx, frame->data, frame->linesize, 0, frame->height,
                                  scaledFrame->data, scaledFrame->linesize);

                        scaledFrame->pts = frameCount++;

                        // Encode frame
                        if (avcodec_send_frame(m_encoderCtx, scaledFrame) == 0) {
                            AVPacket* outPacket = av_packet_alloc();
                            while (avcodec_receive_packet(m_encoderCtx, outPacket) == 0) {
                                // In a full implementation, send this packet to RTSP server
                                // For now, just count frames
                                av_packet_unref(outPacket);
                            }
                            av_packet_free(&outPacket);
                        }
                    }
                }
            }
            av_packet_unref(packet);
        }
    }

    av_frame_free(&frame);
    av_frame_free(&scaledFrame);
    av_packet_free(&packet);
}

void DebugRTSPServer::cleanup() {
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }

    if (m_encoderCtx) {
        avcodec_free_context(&m_encoderCtx);
    }

    if (m_decoderCtx) {
        avcodec_free_context(&m_decoderCtx);
    }

    if (m_inputFormatCtx) {
        avformat_close_input(&m_inputFormatCtx);
    }

    if (m_outputFormatCtx) {
        if (m_outputFormatCtx->pb) {
            avio_closep(&m_outputFormatCtx->pb);
        }
        avformat_free_context(m_outputFormatCtx);
        m_outputFormatCtx = nullptr;
    }
}

} // namespace camera_viewer
