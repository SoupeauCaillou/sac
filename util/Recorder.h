#pragma once

#if SAC_LINUX && ! SAC_EMSCRIPTEN
#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

class Recorder {
    public:
        static const int PBO_COUNT = 2;
        static const int CHANNEL_COUNT = 4;

        Recorder(int width, int height);
        ~Recorder();

        void start();
        void stop();
        void toggle();

        void record();

        void thread_video_encode();

    private:
        void addFrame(GLubyte *ptr);

        bool initVP8();
        bool initOpenGl_PBO();
        bool initSound();

        int width, height;
        bool recording;

        GLuint pboIds[PBO_COUNT];

        std::queue<GLubyte*> buf;

        vpx_codec_ctx_t codec;
        vpx_codec_enc_cfg_t cfg;
        vpx_image_t raw;

        int frameCounter;
        int flags;

        FILE *outfile;

        std::thread th1;
        std::mutex mutex_buf;
        std::condition_variable cond;
};
#endif
