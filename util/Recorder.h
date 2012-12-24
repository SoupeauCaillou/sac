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
		
		pthread_t th1;
		pthread_mutex_t mutex_buf;
        pthread_cond_t cond;
};
#endif
