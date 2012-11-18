#include "Recorder.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <string>
#include "../base/Profiler.h"

#define VPX_CODEC_DISABLE_COMPAT 1
#define interface (vpx_codec_vp8_cx())
#define fourcc    0x30385056

static void* videoEncoder_Callback(void* obj){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	static_cast<Recorder*>(obj)->thread_video_encode();
	
	pthread_exit(0);
}

static void mem_put_le16(char *mem, unsigned int val) {
    mem[0] = val;
    mem[1] = val>>8;
}

static void mem_put_le32(char *mem, unsigned int val) {
    mem[0] = val;
    mem[1] = val>>8;
    mem[2] = val>>16;
    mem[3] = val>>24;
}

static void write_ivf_file_header(FILE *outfile,
                                  const vpx_codec_enc_cfg_t *cfg,
                                  int frame_cnt) {
    char header[32];
 
    if(cfg->g_pass != VPX_RC_ONE_PASS && cfg->g_pass != VPX_RC_LAST_PASS)
        return;
    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header+4,  0);                   /* version */
    mem_put_le16(header+6,  32);                  /* headersize */
    mem_put_le32(header+8,  fourcc);              /* headersize */
    mem_put_le16(header+12, cfg->g_w);            /* width */
    mem_put_le16(header+14, cfg->g_h);            /* height */
    mem_put_le32(header+16, cfg->g_timebase.den); /* rate */
    mem_put_le32(header+20, cfg->g_timebase.num); /* scale */
    mem_put_le32(header+24, frame_cnt);           /* length */
    mem_put_le32(header+28, 0);                   /* unused */
 
    if(fwrite(header, 1, 32, outfile)){}
}

static void write_ivf_frame_header(FILE *outfile,
                                   const vpx_codec_cx_pkt_t *pkt)
{
    char             header[12];
    vpx_codec_pts_t  pts;
 
    if(pkt->kind != VPX_CODEC_CX_FRAME_PKT){
		std::cout << "Error";
        return;
    }
 
    pts = pkt->data.frame.pts;
    mem_put_le32(header, pkt->data.frame.sz);
    mem_put_le32(header+4, pts&0xFFFFFFFF);
    mem_put_le32(header+8, pts >> 32);
 
    if(fwrite(header, 1, 12, outfile)){}
}

static void RGB_To_YV12( unsigned char *pRGBData, int nFrameWidth, int nFrameHeight, int nFrameChannel, void *pFullYPlane, void *pDownsampledUPlane, void *pDownsampledVPlane )
{
    // Convert RGB -> YV12. We do this in-place to avoid allocating any more memory.
    unsigned char *pYPlaneOut = (unsigned char*)pFullYPlane;
    int nYPlaneOut = 0;

    for ( int y=nFrameHeight-1; y > -1 ; --y)
    {
		for (int x=0; x < nFrameWidth*nFrameChannel; x+=nFrameChannel)
		{
			unsigned char B = pRGBData[x+0+y*nFrameWidth*nFrameChannel];
			unsigned char G = pRGBData[x+1+y*nFrameWidth*nFrameChannel];
			unsigned char R = pRGBData[x+2+y*nFrameWidth*nFrameChannel];

			float Y = (float)( R*66 + G*129 + B*25 + 128 ) / 256 + 16;
			float U = (float)( R*-38 + G*-74 + B*112 + 128 ) / 256 + 128;
			float V = (float)( R*112 + G*-94 + B*-18 + 128 ) / 256 + 128;

			// NOTE: We're converting pRGBData to YUV in-place here as well as writing out YUV to pFullYPlane/pDownsampledUPlane/pDownsampledVPlane.
			pRGBData[x+0+y*nFrameWidth*nFrameChannel] = (unsigned char)Y;
			pRGBData[x+1+y*nFrameWidth*nFrameChannel] = (unsigned char)U;
			pRGBData[x+2+y*nFrameWidth*nFrameChannel] = (unsigned char)V;

			// Write out the Y plane directly here rather than in another loop.
			pYPlaneOut[nYPlaneOut++] = pRGBData[x+0+y*nFrameWidth*nFrameChannel];
		}
	}
    // Downsample to U and V.
    int halfHeight = nFrameHeight >> 1;
    int halfWidth = nFrameWidth >> 1;

    unsigned char *pVPlaneOut = (unsigned char*)pDownsampledVPlane;
    unsigned char *pUPlaneOut = (unsigned char*)pDownsampledUPlane;

    for ( int yPixel=0; yPixel < halfHeight; yPixel++ )
    {
        int iBaseSrc = ( (yPixel*2) * nFrameWidth * nFrameChannel );

        for ( int xPixel=0; xPixel < halfWidth; xPixel++ )
        {
            pVPlaneOut[(halfHeight-1 - yPixel) * halfWidth + xPixel] = pRGBData[iBaseSrc + 2];
            pUPlaneOut[(halfHeight-1 - yPixel) * halfWidth + xPixel] = pRGBData[iBaseSrc + 1];

            iBaseSrc += nFrameChannel * 2;
        }
    }
}

Recorder::Recorder(int width, int height){
	this->width = width;
	this->height = height;
	outfile = NULL;
	recording = false;
    flags = 0;

	pthread_mutex_init(&mutex_buf, NULL);
    pthread_cond_init(&cond, 0);

	initOpenGl_PBO();
	initVP8();
}

Recorder::~Recorder(){
	if (pthread_cancel (th1) != 0) {
		std::cout << "pthread_cancel error for thread" << std::endl;
	}
	vpx_codec_destroy(&codec);
	vpx_img_free (&raw);

	pthread_mutex_destroy(&mutex_buf);
	
	glDeleteBuffers(PBO_COUNT, pboIds);
	delete [] test;
}

bool Recorder::initOpenGl_PBO (){
	// init PBOs
	glGenBuffers(PBO_COUNT, pboIds);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, width*height*CHANNEL_COUNT, 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, width*height*CHANNEL_COUNT, 0, GL_STREAM_READ);

	return true;
} 

bool Recorder::initVP8 (){
	// init VP8 encoder
	/* Populate encoder configuration */
	vpx_codec_err_t res = vpx_codec_enc_config_default(interface, &cfg, 0);
	if (res) {
		std::cout << "Failed to get config: " << vpx_codec_err_to_string(res) << std::endl;
		return false;
	}
	/* Update the default configuration with our settings */
	cfg.rc_target_bitrate = (width*height*3*8*30) /1000;
	cfg.g_w = width;
	cfg.g_h = height;
	cfg.kf_mode = VPX_KF_AUTO;
	cfg.kf_max_dist = 300;
	cfg.g_pass = VPX_RC_ONE_PASS; /* -p 1 */
	cfg.g_timebase.num = 1;
	cfg.g_timebase.den = 30; /* fps = 30 */
	cfg.rc_end_usage = VPX_CBR; /* --end-usage=cbr */
	cfg.g_threads = 2;
	
	if (vpx_codec_enc_init(&codec, interface, &cfg, 0)){
		std::cout << "Failed to initialize encoder" << std::endl;
		return false;
	}
	
	vpx_codec_control(&codec, VP8E_SET_CPUUSED, (cfg.g_threads > 1) ? 10 : 10); 
	vpx_codec_control(&codec, VP8E_SET_STATIC_THRESHOLD, 0);
	vpx_codec_control(&codec, VP8E_SET_ENABLEAUTOALTREF, 1);
	
	if (cfg.g_threads > 1) {
		if (vpx_codec_control(&codec, VP8E_SET_TOKEN_PARTITIONS, (vp8e_token_partitions) cfg.g_threads) != VPX_CODEC_OK) {
			std::cout << "VP8: failed to set multiple token partition" << std::endl;
		} else {
			std::cout << "VP8: multiple token partitions used" << std::endl;
		}
	}

	if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1)){
		std::cout << "Failed to allocate image" << std::endl;
		return false;
	}
	
	return true;
}

bool Recorder::initSound (){
	return true;
}

void Recorder::start(){
	if (outfile == NULL && recording == false){
		std::cout << "Recording start" << std::endl;

		//new file : time
        char tmp[256];
		long H = time(NULL);
        strftime(tmp, sizeof(tmp), "videos/rr_%d%m%Y_%X.webm", localtime(&H));
		
		if(!(outfile = fopen(tmp, "wb"))){
			std::cout << "Failed to open '" << tmp << "' for writing"<< std::endl;
			outfile = NULL;
			return;
		}
		write_ivf_file_header(outfile, &cfg, 0);
		this->frameCounter = 0;
		recording = true;
		
		// on lance le thread pour l'encodage
		if (pthread_create (&th1, NULL, videoEncoder_Callback, (void*) this) < 0) {
			std::cout << "pthread_create error for thread" << std::endl;
		}
	}
}

void Recorder::stop(){
	if (outfile != NULL && recording == true){
		std::cout << "Recording stop" << std::endl;
		pthread_mutex_lock (&mutex_buf);
        pthread_cond_signal(&cond);
		buf.push(NULL);
        pthread_mutex_unlock (&mutex_buf);
		recording = false;
	}
}

void Recorder::toggle(){
	if (outfile)
		stop();
	else
		start();
}

void Recorder::record(){
	static int d = 0;
	static int index = 0;

	if (recording && outfile){
        d = (d + 1) % 2;
		if (d == 0){
            PROFILE("Recorder", "read-request", BeginEvent);
			// "index" is used to read pixels from framebuffer to a PBO
			// "nextIndex" is used to update pixels in the other PBO
			index = (index + 1) % PBO_COUNT;
			int nextIndex = (index + 1) % PBO_COUNT;

			// set the target framebuffer to read
			glReadBuffer(GL_FRONT);

			// read pixels from framebuffer to PBO
			// glReadPixels() should return immediately.
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[index]);
			glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
            PROFILE("Recorder", "read-request", EndEvent);
            PROFILE("Recorder", "read-back", BeginEvent);
			// map the PBO to process its data by CPU
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
			GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER,
													GL_READ_ONLY);
			if(ptr)
			{
				test = new GLubyte[width*height * CHANNEL_COUNT];
				memcpy (test, ptr, width*height*CHANNEL_COUNT );

				pthread_mutex_lock (&mutex_buf);
				buf.push(test);
                pthread_cond_signal(&cond);
				pthread_mutex_unlock (&mutex_buf);
				
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}

			// back to conventional pixel operation
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            PROFILE("Recorder", "read-back", EndEvent);
		}
	}
}

void Recorder::thread_video_encode(){
	while (1)
	{
        if(outfile != NULL && !recording) {
             if(!fseek(outfile, 0, SEEK_SET))
                 write_ivf_file_header(outfile, &cfg, this->frameCounter-1);

             fclose(outfile);

             outfile = NULL;

             std::cout << "Recording is available" << std::endl;
             break;
        }

        pthread_mutex_lock (&mutex_buf);
		while (buf.empty())
            pthread_cond_wait(&cond, &mutex_buf);
	    GLubyte *ptr = buf.front();
		buf.pop();
		pthread_mutex_unlock (&mutex_buf);
		addFrame(ptr);
        if (ptr)
		    delete [] ptr;
	}
}

void Recorder::addFrame(GLubyte *ptr){
	vpx_codec_iter_t iter = NULL;
	const vpx_codec_cx_pkt_t *pkt;
	
	if (ptr)
		RGB_To_YV12(ptr, width, height, CHANNEL_COUNT, raw.planes[0], raw.planes[1], raw.planes[2]);
	
    PROFILE("Recorder", "encode-image", BeginEvent);
	if(vpx_codec_encode(&codec, ptr ? &raw : NULL, this->frameCounter,
						1, flags, VPX_DL_REALTIME)){
		const char *detail = vpx_codec_error_detail(&codec);
		std::cout << "Failed to encode frame";
		if (detail)
			std::cout << "     " << detail;
		std::cout << std::endl;
	}
    PROFILE("Recorder", "encode-image", EndEvent);
    PROFILE("Recorder", "write-disk", BeginEvent);
	while( (pkt = vpx_codec_get_cx_data(&codec, &iter)) ) {
		switch(pkt->kind) {
		case VPX_CODEC_CX_FRAME_PKT:
			write_ivf_frame_header(outfile, pkt);
			if(fwrite(pkt->data.frame.buf, 1, pkt->data.frame.sz,
					  outfile)){}
			break;                                                    
		default:
			break;
		}
	}
    PROFILE("Recorder", "write-disk", EndEvent);
	++this->frameCounter;
}
