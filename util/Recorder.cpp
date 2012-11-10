#include "Recorder.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>


#define VPX_CODEC_DISABLE_COMPAT 1
//#include <vpx/vpx_encoder.h>
//#include <vpx/vp8cx.h>
#define interface (vpx_codec_vp8_cx())
#define fourcc    0x30385056

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
 
    if(pkt->kind != VPX_CODEC_CX_FRAME_PKT)
        return;
 
    pts = pkt->data.frame.pts;
    mem_put_le32(header, pkt->data.frame.sz);
    mem_put_le32(header+4, pts&0xFFFFFFFF);
    mem_put_le32(header+8, pts >> 32);
 
    if(fwrite(header, 1, 12, outfile)){}
}

Recorder::Recorder(int width, int height){
	this->width = width;
	this->height = height;
	this->recording = false;
	this->outfile = NULL;
	
	// init PBOs
	glGenBuffersARB(PBO_COUNT, pboIds);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, width*height*CHANNEL_COUNT, 0, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, width*height*CHANNEL_COUNT, 0, GL_STREAM_READ_ARB);
	
	
}

Recorder::~Recorder(){
	glDeleteBuffersARB(PBO_COUNT, pboIds);
}

void Recorder::start(){
	if (outfile == NULL){
		std::cout << "Recording start" << std::endl;
		
		// init VP8 encoder
		/* Populate encoder configuration */
		vpx_codec_err_t res = vpx_codec_enc_config_default(interface, &cfg, 0);
		if (res) {
			std::cout << "Failed to get config: " << vpx_codec_err_to_string(res) << std::endl;
		}
		/* Update the default configuration with our settings */
		cfg.rc_target_bitrate = width * height * cfg.rc_target_bitrate
								/ cfg.g_w / cfg.g_h;
		cfg.g_w = width;
		cfg.g_h = height;
		
		if(vpx_codec_enc_init(&codec, interface, &cfg, 0)){
			std::cout << "Failed to initialize encoder" << std::endl;
		}
		
		if(!vpx_img_alloc(&raw, VPX_IMG_FMT_ARGB, width, height, 1))
		//if(!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1))
			std::cout << "Failed to allocate image" << std::endl;
		
		//new file : time
		std::stringstream a;
		a << "videos/" << time(NULL) << ".avi";
		
		if(!(outfile = fopen(a.str().c_str(), "wb"))){
			std::cout << "Failed to open " << a.str() << " for writing"<< std::endl;
			this->recording = false;
			outfile = NULL;
			return;
		}
		this->recording = true;
		write_ivf_file_header(outfile, &cfg, 0);
		this->frameCounter = 0;
	}
}

void Recorder::stop(){
	if (outfile != NULL){
		std::cout << "Recording stop" << std::endl;
		this->recording = false;
		
		saveImage(NULL);
		
		if(vpx_codec_destroy(&codec)){
			std::cout << "Failed to destroy codec" << std::endl;
		}
				
		if(!fseek(outfile, 0, SEEK_SET))
			write_ivf_file_header(outfile, &cfg, this->frameCounter);
		fclose(outfile);
		outfile = NULL;
	}
}

void Recorder::toggle(){
	if (this->recording)
		stop();
	else
		start();
}

void Recorder::record(){
	static int index = 0;
	if (this->outfile != NULL){
		// "index" is used to read pixels from framebuffer to a PBO
		// "nextIndex" is used to update pixels in the other PBO
		index = (index + 1) % 2;
		int nextIndex = (index + 1) % 2;

		// set the target framebuffer to read
		glReadBuffer(GL_FRONT);

		// read pixels from framebuffer to PBO
		// glReadPixels() should return immediately.
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[index]);
		glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);

		// map the PBO to process its data by CPU
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[nextIndex]);
		GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB,
												GL_READ_ONLY_ARB);
		if(ptr)
		{
			saveImage(ptr);
			glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
		}

		// back to conventional pixel operation
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	}
}

void Recorder::saveImage(GLubyte *ptr){
	vpx_codec_iter_t iter = NULL;
	const vpx_codec_cx_pkt_t *pkt;

	if (ptr != NULL)
	{
		//int j=0;
		//for (int i=0; i < height*width; i+=4, ++j)
		//{
			
			raw.planes[VPX_PLANE_PACKED] = ptr;
			//std::cout << "i = " << i << " / " << height*width << " valeurs pixels" << " 1 =" << (int) *(ptr+i) << " 2 =" << (int)*(ptr+i+1) << " 3 =" << (int)*(ptr+i+2) << " 4 =" << (int)*(ptr+i+3) << std::endl;
			//raw.planes[0][j] = (*(ptr+i) << 24) + (*(ptr+i+1) << 16) + (*(ptr+i+2) << 8) + *(ptr+i+3);
			//raw.planes[0][j] = 0.299 * ( *(ptr+i+2)) + 0.587 * ( *(ptr+i+1)) + 0.114 * ( *(ptr+i));
			//raw.planes[1][j] = (*(ptr+i+1) - raw.planes[0][j]) * 0.565;
			//raw.planes[2][j] = (*(ptr+i+2) - raw.planes[0][j]) * 0.713;
			//raw.planes[3][j] = *(ptr+i);
		//}
	}
	
	if(vpx_codec_encode(&codec, ptr!=NULL? &raw:NULL, this->frameCounter,
						1, flags, VPX_DL_REALTIME))
		std::cout << "Failed to encode frame"<< std::endl;

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
	++this->frameCounter;
}
