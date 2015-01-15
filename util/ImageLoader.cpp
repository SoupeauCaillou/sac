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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses-equality"
#include "stb_image.h"
#pragma GCC diagnostic pop

#include "ImageLoader.h"
#include "../api/AssetAPI.h"
#include "base/Log.h"

#include <stdlib.h>

#if SAC_LINUX || SAC_ANDROID
#include <arpa/inet.h>
#include <endian.h>
#elif SAC_DARWIN
#include <machine/endian.h>
#endif

#if SAC_DESKTOP
#include "rg_etc1.h"
#endif

struct FileBufferOffset {
	FileBuffer file;
	int offset;
};
ImageDesc ImageLoader::loadPng(const std::string& filepath, const FileBuffer& ) {
    ImageDesc result;
    result.datas = 0;
    result.type = ImageDesc::RAW;

    int w;
    int h;
    int comp;
    unsigned char* image = stbi_load(filepath.c_str(), &w, &h, &comp, STBI_rgb_alpha);
    if(image) {
        result.width = w;
        result.height = h;
        result.datas = (char*)image;
        result.channels = comp;
    } else {
        LOGE("Failed to load PNG: " << __(filepath));
    }
    return result;
}

ImageDesc ImageLoader::loadEtc1(const std::string& LOG_USAGE_ONLY(context), const FileBuffer& file, bool etc1supported) {
    #define BE_16_TO_H htons

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
