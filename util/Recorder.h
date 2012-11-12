#pragma once

#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

class Recorder {
	public:
		Recorder(int width, int height);
		~Recorder();
		
		void start();
		void stop();
		void record();
		void toggle();
		
		static const int PBO_COUNT = 2;
		static const int CHANNEL_COUNT = 4;
	private:
		void saveImage(GLubyte *ptr);
		void initVP8();
		
		int width, height;
		bool recording;
		
		GLuint pboIds[PBO_COUNT];
		
		vpx_codec_ctx_t codec;
		vpx_codec_enc_cfg_t cfg;
		vpx_image_t raw;
		
		int frameCounter;
		int flags;
		
		FILE *outfile;
};
