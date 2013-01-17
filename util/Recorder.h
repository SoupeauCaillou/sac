/*!
 * \file Recorder.h
 * \brief A basic tool to record video with OpenGL
 * \author Jordane Pelloux-Prayer
 */
#pragma once

#ifndef EMSCRIPTEN
#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>

#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

#include <pthread.h>

/*!
 * \class Recorder
 */
class Recorder {
        public:
        /*! \def PBO_COUNT
         *  \brief Number of used PBO (OpenGL)
         */
        static const int PBO_COUNT = 2;
        /*! \def CHANNEL_COUNT
         *  \brief Number of channel in captured image
         */
        static const int CHANNEL_COUNT = 4;

        /*! \brief Recorder class constructor
         *  \param width : width of the box to capture
         *  \param height : height of the box to capture
         */
        Recorder(int width, int height);

        /*! \brief Destructor
         */
        ~Recorder();

        /*!
         * \brief Start the capture of video
         */
        void start();

        /*!
         * \brief Stop the capture of video
         */
        void stop();

        /*!
         * \brief Switch the state of recorder
         */
        void toggle();

        /*!
         * \brief Record images
         */
        void record();

        /*!
         * \brief function of thread to encode image in an VPX format
         */
        void thread_video_encode();

    private:
        /*!
         * \brief Add an image to encode
         */
        void addFrame(GLubyte *ptr);

        /*!
         * \brief Initialisation of VP8 encoder
         */
        bool initVP8();
        /*!
         * \brief Initialisaiton of PBO for OpenGl
         */
        bool initOpenGl_PBO();
        /*!
         * \brief Not used
         */
        bool initSound();

        int width, height; /*! Size of the box to capture */
        bool recording; /*! State of the recorder */

        GLuint pboIds[PBO_COUNT]; /*! Id of PBOs */

        std::queue<GLubyte*> buf; /*! Queue to stock pictures before their encoding */

        vpx_codec_ctx_t codec; /*! VP8 codec */
        vpx_codec_enc_cfg_t cfg; /*! VP8 codec config */
        vpx_image_t raw; /*! image for vp8 (in YUV) */

        int frameCounter; /*! Number of frame in a video */
        int flags;

        FILE *outfile; /*! File where will be save the video */

        pthread_t th1;  /*! Thread to encode with VP8 encoder */
        pthread_mutex_t mutex_buf; /*! mutex to excluse use of picture queue 'buf' */
        pthread_cond_t cond; /*! Condition for 'th1' to be active */
};
#endif
