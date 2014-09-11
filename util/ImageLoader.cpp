/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "ImageLoader.h"
#include "../api/AssetAPI.h"
#include "base/Log.h"

#ifdef SAC_EMSCRIPTEN
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rwops.h>
#else
#include <png.h>
#endif
#include <stdlib.h>

#if SAC_LINUX || SAC_ANDROID
#include <endian.h>
#endif

#if SAC_DESKTOP
#include "rg_etc1.h"
#endif

#ifndef SAC_EMSCRIPTEN
static void read_from_buffer(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);
#endif

struct FileBufferOffset {
	FileBuffer file;
	int offset;
};
ImageDesc ImageLoader::loadPng(const std::string& LOG_USAGE_ONLY(context), const FileBuffer& file) {
	ImageDesc result;
	result.datas = 0;
	result.type = ImageDesc::RAW;

#ifndef SAC_EMSCRIPTEN
	uint8_t PNG_header[8];
	memcpy(PNG_header, file.data, 8);
	if (png_sig_cmp(PNG_header, 0, 8) != 0) {
		LOGW(context << " is not a PNG");
		return result;
	}

	png_structp PNG_reader = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (PNG_reader == NULL)
	{
		LOGW("Can't start reading " << context);
		return result;
	}

	png_infop PNG_info = png_create_info_struct(PNG_reader);
	if (PNG_info == NULL)
	{
        LOGW("ERROR: Can't get info for " << context);
		png_destroy_read_struct(&PNG_reader, NULL, NULL);
		return result;
	}

png_infop PNG_end_info = png_create_info_struct(PNG_reader);

	if (setjmp(png_jmpbuf(PNG_reader)))
	{
        LOGW("ERROR: Can't load " << context);
		png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
		return result;
	}

	FileBufferOffset fb;
	fb.file = file;
	fb.offset = 8;
	png_set_read_fn(PNG_reader, &fb, &read_from_buffer);
	// png_init_io(PNG_reader, PNG_file);
	png_set_sig_bytes(PNG_reader, 8);

	png_read_info(PNG_reader, PNG_info);

	result.width = png_get_image_width(PNG_reader, PNG_info);
	result.height = png_get_image_height(PNG_reader, PNG_info);

	png_uint_32 bit_depth, color_type;
	bit_depth = png_get_bit_depth(PNG_reader, PNG_info);
	color_type = png_get_color_type(PNG_reader, PNG_info);

	result.channels = 0;
	if (color_type == PNG_COLOR_TYPE_GRAY)
	{
		if (bit_depth < 8) {
			png_set_expand_gray_1_2_4_to_8(PNG_reader);
		}
		result.channels = 1;
	} else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		result.channels = 2;
	} else if (color_type == PNG_COLOR_TYPE_RGB) {
		result.channels = 3;
	} else if (color_type == PNG_COLOR_TYPE_RGBA) {
		result.channels = 4;
	} else {
		LOGF(context << " INVALID color type: " << color_type);
	}

	// if (color_type & PNG_COLOR_MASK_ALPHA)
    //    png_set_strip_alpha(PNG_reader);

	if (png_get_valid(PNG_reader, PNG_info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(PNG_reader);
	}
	else
	{
		png_set_filler(PNG_reader, 0xff, PNG_FILLER_AFTER);
	}

	if (bit_depth == 16)
	{
		png_set_strip_16(PNG_reader);
	}

	png_read_update_info(PNG_reader, PNG_info);

	int rowbytes = png_get_rowbytes(PNG_reader, PNG_info);
	png_byte* PNG_image_buffer = new png_byte[rowbytes * result.height];

	png_byte** PNG_rows = (png_byte**)malloc(result.height * sizeof(png_byte*));

	int row;
	for (row = 0; row < result.height; ++row) {
		PNG_rows[row] /*result.height - 1 - row]*/ = PNG_image_buffer + (row * rowbytes);
	}

	png_read_image(PNG_reader, PNG_rows);
	free(PNG_rows);

	png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);

	// remove unneeded channels
	int actual = rowbytes / result.width;
	if (actual > result.channels) {
		int newrow = result.channels * result.width;
		png_byte* PNG_image_buffer2 = new png_byte[newrow * result.height];
		for (row = 0; row < result.height; ++row) {
			for (int i=0; i<result.width; i++) {
				memcpy(&PNG_image_buffer2[newrow * row + i * result.channels], &PNG_image_buffer[rowbytes * row + i * actual], result.channels);
			}
		}
		delete[] PNG_image_buffer;
		PNG_image_buffer = PNG_image_buffer2;
	}

	result.datas = (char*)PNG_image_buffer;
#else
    result.datas = 0;
    std::stringstream a;
    a << "assets/" << context << ".png";
    std::string aa = a.str();
    SDL_Surface* s = IMG_Load(aa.c_str());
    if (s == 0) {
        LOGW("Failed to load '" << a.str() << "'");
        return result;
    }
    LOGI("Image : " << context << " format: " << s->w << 'x' << s->h << ' ' << (int)s->format->BitsPerPixel << " bpp");
    result.channels = s->format->BitsPerPixel / 8;

    result.type = ImageDesc::RAW;
    result.width = s->w;
    result.height = s->h;
    result.datas = new char[result.width * result.height * result.channels];
    memcpy(result.datas, s->pixels, result.width * result.height * result.channels);
    SDL_FreeSurface(s);
#endif
    result.mipmap = 0;
	return result;
}

ImageDesc ImageLoader::loadEtc1(const std::string& LOG_USAGE_ONLY(context), const FileBuffer& file, bool etc1supported) {
#if SAC_ANDROID
    #define BE_16_TO_H betoh16
#else
    #define BE_16_TO_H be16toh
#endif

    ImageDesc result;
    result.datas = 0;
    result.mipmap = 0;

    unsigned offset = 0;
    if (strncmp("PKM ", (const char*)file.data, 4)) {
        LOGW("ETC " << context << " wrong magic header");
        return result;
    }
    offset += 4;

    // skip version/type
    offset += 4;
    // skip extended width/height
    offset += 2 * 2;
    // read width/height

    result.width = BE_16_TO_H(*(uint16_t*)(&file.data[offset]));
    offset += 2;
    result.height = BE_16_TO_H(*(uint16_t*)(&file.data[offset]));
    offset += 2;


    if (etc1supported) {
        result.datas = new char[file.size - offset];
        memcpy(result.datas, &file.data[offset], file.size - offset);
        result.channels = 3;
        result.type = ImageDesc::ETC1;
    } else {
        #if SAC_DESKTOP
        unsigned int* pixels = new unsigned int[result.width * result.height];
        result.datas = (char*) pixels;
        memset(pixels, 255, result.width * result.height * 4);
        // 64bits -> 4x4 pixels
        LOG_USAGE_ONLY(int blockCount = (result.width * result.height) / (4 * 4);)
        int blockIndex = 0;

        unsigned int decodedBlock[4 * 4];

        for (int i=0; i<result.height/4; i++) {
            for (int j=0; j<result.width/4; j++) {
                bool r = rg_etc1::unpack_etc1_block(&file.data[offset + 8 * blockIndex], decodedBlock);

                for (int k=0; k<4; k++) {
                    memcpy(&pixels[(4 * i + k) * result.width + 4 * j], &decodedBlock[4 * k], 4 * sizeof(int));
                }
                LOGF_IF(!r, "unpack_etc1_block failed. Block: " << blockIndex << '/' << blockCount);
                blockIndex++;
            }
        }

        result.channels = 4;
        result.type = ImageDesc::RAW;
        #else
        LOGF("etc1 emulation only supported on Desktop");
        #endif
    }


    return result;

    #undef BE_16_TO_H
}

#ifndef SAC_EMSCRIPTEN
static void read_from_buffer(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
   if(png_get_io_ptr(png_ptr) == NULL)
      return;   // add custom error handling here
   FileBufferOffset* buffer = (FileBufferOffset*)png_get_io_ptr(png_ptr);
   memcpy(outBytes, &buffer->file.data[buffer->offset], byteCountToRead);
   buffer->offset += byteCountToRead;
}
#endif

ImageDesc ImageLoader::loadPvr(const std::string&, const FileBuffer& file) {
	ImageDesc result;
	result.datas = 0;
	struct PVRTexHeader {
    uint32_t dwHeaderSize;
    uint32_t height;
    uint32_t width;
    uint32_t dwMipMapCount;
    uint32_t dwpfFlags;
    uint32_t dwDataSize;
    uint32_t dwBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwAlphaBitMask;
    uint32_t dwPVR;
    uint32_t dwNumSurfs;
};
	PVRTexHeader* header = (PVRTexHeader*)&file.data[0];

	result.width = header->width;
	result.height = header->height;
    result.mipmap = header->dwMipMapCount;
    result.channels = 3;
    result.type = ImageDesc::PVR;
	int size = file.size - sizeof(PVRTexHeader);
	result.datas = new char[size];
	memcpy(result.datas, &file.data[sizeof(PVRTexHeader)], size);
	return result;
}

ImageDesc ImageLoader::loadDDS(const std::string& /*context*/, const FileBuffer& file) {
    ImageDesc result;
    result.datas = 0;

    // from http://www.mindcontrol.org/~hplus/graphics/dds-info/
    union DDS_header {
      struct {
        unsigned int    dwMagic;
        unsigned int    dwSize;
        unsigned int    dwFlags;
        unsigned int    dwHeight;
        unsigned int    dwWidth;
        unsigned int    dwPitchOrLinearSize;
        unsigned int    dwDepth;
        unsigned int    dwMipMapCount;
        unsigned int    dwReserved1[ 11 ];

    //  DDPIXELFORMAT
        struct {
          unsigned int    dwSize;
          unsigned int    dwFlags;
          unsigned int    dwFourCC;
          unsigned int    dwRGBBitCount;
          unsigned int    dwRBitMask;
          unsigned int    dwGBitMask;
          unsigned int    dwBBitMask;
          unsigned int    dwAlphaBitMask;
      }               sPixelFormat;

    //  DDCAPS2
      struct {
          unsigned int    dwCaps1;
          unsigned int    dwCaps2;
          unsigned int    dwDDSX;
          unsigned int    dwReserved;
      }               sCaps;
      unsigned int    dwReserved2;
      };
      char data[ 128 ];
    };

    const DDS_header* header = (const DDS_header*) &file.data[0];
    result.width = header->dwWidth;
    result.height = header->dwHeight;
    result.type = ImageDesc::S3TC;
    result.channels = 3;
    LOGF_IF(header->sPixelFormat.dwFourCC != 0x31545844 /*D3DFMT_DXT1*/, "We only support DXT1 mode for S3TC compression");
    result.mipmap = header->dwMipMapCount - 1;
    if (result.mipmap < 0) {
        LOGW("Weird mipmap count. Let's force it to 1");
        result.mipmap = 0;
    }
    int size = file.size - sizeof(DDS_header);
    result.datas = new char[size];
    memcpy(result.datas, &file.data[sizeof(DDS_header)], size);
    return result;
}
