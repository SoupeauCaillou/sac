#include "Recorder.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>


#define VPX_CODEC_DISABLE_COMPAT 1
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

static void RGB_To_YV12( unsigned char *pRGBData, int nFrameWidth, int nFrameHeight, void *pFullYPlane, void *pDownsampledUPlane, void *pDownsampledVPlane )
{
    int nRGBBytes = nFrameWidth * nFrameHeight * 3;

    // Convert RGB -> YV12. We do this in-place to avoid allocating any more memory.
    unsigned char *pYPlaneOut = (unsigned char*)pFullYPlane;
    int nYPlaneOut = 0;

    for ( int y=nFrameHeight-1; y > -1 ; --y)
    {
		for (int x=0; x < nFrameWidth*3; x+=3)
		{
			unsigned char B = pRGBData[x+2+y*nFrameWidth*3];
			unsigned char G = pRGBData[x+1+y*nFrameWidth*3];
			unsigned char R = pRGBData[x+0+y*nFrameWidth*3];

			float Y = (float)( R*66 + G*129 + B*25 + 128 ) / 256 + 16;
			float U = (float)( R*-38 + G*-74 + B*112 + 128 ) / 256 + 128;
			float V = (float)( R*112 + G*-94 + B*-18 + 128 ) / 256 + 128;

			// NOTE: We're converting pRGBData to YUV in-place here as well as writing out YUV to pFullYPlane/pDownsampledUPlane/pDownsampledVPlane.
			pRGBData[x+0+y*nFrameWidth*3] = (unsigned char)Y;
			pRGBData[x+1+y*nFrameWidth*3] = (unsigned char)U;
			pRGBData[x+2+y*nFrameWidth*3] = (unsigned char)V;

			// Write out the Y plane directly here rather than in another loop.
			pYPlaneOut[nYPlaneOut++] = pRGBData[x+0+y*nFrameWidth*3];
		}
	}
    // Downsample to U and V.
    int halfHeight = nFrameHeight >> 1;
    int halfWidth = nFrameWidth >> 1;

    unsigned char *pVPlaneOut = (unsigned char*)pDownsampledVPlane;
    unsigned char *pUPlaneOut = (unsigned char*)pDownsampledUPlane;

    for ( int yPixel=0; yPixel < halfHeight; yPixel++ )
    {
        int iBaseSrc = ( (yPixel*2) * nFrameWidth * 3 );

        for ( int xPixel=0; xPixel < halfWidth; xPixel++ )
        {
            pVPlaneOut[(halfHeight-1 - yPixel) * halfWidth + xPixel] = pRGBData[iBaseSrc + 2];
            pUPlaneOut[(halfHeight-1 - yPixel) * halfWidth + xPixel] = pRGBData[iBaseSrc + 1];

            iBaseSrc += 6;
        }
    }
}

Recorder::Recorder(int width, int height){
	this->width = width;
	this->height = height;
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
		
		if(!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1))
			std::cout << "Failed to allocate image" << std::endl;
		
		//new file : time
		std::stringstream a;
		long H = time(NULL);
		a << "videos/" << ctime(&H) << ".webm";
		
		if(!(outfile = fopen(a.str().c_str(), "wb"))){
			std::cout << "Failed to open " << a.str() << " for writing"<< std::endl;
			outfile = NULL;
			return;
		}
		
		write_ivf_file_header(outfile, &cfg, 0);
		this->frameCounter = 0;
	}
}

void Recorder::stop(){
	if (outfile != NULL){
		std::cout << "Recording stop" << std::endl;

		saveImage(NULL);

		if(!fseek(outfile, 0, SEEK_SET))
			write_ivf_file_header(outfile, &cfg, this->frameCounter-1);
		
		fclose(outfile);
		outfile = NULL;
		
		if(vpx_codec_destroy(&codec)){
			std::cout << "Failed to destroy codec" << std::endl;
		}
	}
}

void Recorder::toggle(){
	if (outfile)
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
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);

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
	
	if (ptr)
		RGB_To_YV12(ptr, width, height, raw.planes[0], raw.planes[1], raw.planes[2]);
	
	if(vpx_codec_encode(&codec, ptr ? &raw : NULL, this->frameCounter,
						1, flags, VPX_DL_REALTIME)){
		const char *detail = vpx_codec_error_detail(&codec);
		std::cout << "Failed to encode frame";
		if (detail)
			std::cout << "     " << detail;
		std::cout << std::endl;
	}
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
