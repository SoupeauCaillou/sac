#include "TextureLibrary.h"
#include "api/AssetAPI.h"
#include <glog/logging.h>

#include "OpenglHelper.h"

#define ALPHA_MASK_TAG "_alpha"

bool TextureLibrary::doLoad(const std::string& name, Texture& texture, const TextureRef& ) {
    // Determine if file is a color or alpha, based on name convention
    enum { COLOR, ALPHA_MASK } type;
    if (name.find(ALPHA_MASK_TAG) != std::string::npos) {
        type = ALPHA_MASK;
    } else {
        type = COLOR;
    }

    // Read file content
    FileBuffer file;
    bool png = false;

    // First, try PVR compression, then PKM (ETC1)
    if (type == COLOR) {
        if (pvrFormatSupported) {
            VLOG(2) << "Using PVR version";
            file = assetAPI->loadAsset(name + ".pvr");
        } else if (pkmFormatSupported) {
            VLOG(2) << "Using PKM version";
            file = assetAPI->loadAsset(name + ".pkm");
        } else {
            VLOG(1) << "PVM nor ETC1 supported, falling back on PNG format";
        }
    }

    if (!file.data) {
        VLOG(2) << "Using PNG version";
        file = assetAPI->loadAsset(name + ".png");
        if (!file.data) {
            LOG(ERROR) << "Image not found '" << name << ".png'";
            return false;
        }
        png = true;
    }

    // Parse image
    ImageDesc image = parseImageContent(name, file, png);

    delete[] file.data;
    if (!image.datas) {
        LOG(ERROR) << "Could not read image, aborting";
        return false;
    }

    // Create OpenGL texture, and initialize
    GL_OPERATION(glGenTextures(1, &texture.id))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, texture.id))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))

    // Enable mipmapping when supported
#ifdef ANDROID
    if ((type == COLOR) && image.mipmap > 0) {
#elif defined(EMSCRIPTEN)
    if (false) {
#else
    if (type == COLOR) {
#endif
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST))
    } else {
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
    }

    // Determine GL format based on channel count
    GLenum format;
    switch (image.channels) {
        case 1:
            format = GL_ALPHA;
            break;
        case 2:
            format = GL_LUMINANCE_ALPHA;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }

    if (png) {
        VLOG(2) << "Using PNG texture version " << image.width << 'x' << image.height;
        #ifdef EMSCRIPTEN
        GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, NULL))
        #else
        GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, (type == COLOR) ? GL_RGB:GL_ALPHA, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, NULL))
        #endif
        GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, image.datas))
    } else {
       #ifdef ANDROID
        char* ptr = image.datas;
        VLOG(2) << "Using " << (pvrSupported ? "PVR" : "ETC1") << " texture version (" << image.width << 'x' << image.height << " - " << image.mipmap << " mipmap)";
        for (int level=0; level<=image.mipmap; level++) {
            int width = MathUtil::Max(1, image.width >> level);
            int height = MathUtil::Max(1, image.height >> level);
            unsigned imgSize = 0;
            if (pvrSupported) 
                imgSize =( MathUtil::Max(width, 8) * MathUtil::Max(height, 8) * 4 + 7) / 8;
            else
                imgSize = 8 * ((width + 3) >> 2) * ((height + 3) >> 2);
            VLOG(3) << "\t- mipmap " << level << " : " << width << 'x' << height;
            GL_OPERATION(glCompressedTexImage2D(GL_TEXTURE_2D, level, pvrSupported ? GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG : GL_ETC1_RGB8_OES, width, height, 0, imgSize, ptr))
            ptr += imgSize;
        }
        #else
        LOG(FATAL) << "ETC compression not supported";
        return false;
        #endif
    }

#if !defined(ANDROID) && !defined(EMSCRIPTEN)
    if (image.mipmap == 0) {
        VLOG(1) << "Generating mipmaps";
        glGenerateMipmap(GL_TEXTURE_2D);
    }
#endif
    delete[] image.datas;
    texture.size.X = image.width;
    texture.size.Y = image.height;

    return true;
}


void TextureLibrary::doUnload(const std::string& name, const Texture& in) {

}

void TextureLibrary::reload(const std::string& name, Texture& out) {

}

ImageDesc TextureLibrary::parseImageContent(const std::string& basename, const FileBuffer& file, bool isPng) const {
#ifndef EMSCRIPTEN
    // load image
    return isPng ? ImageLoader::loadPng(basename, file) : pvrFormatSupported ? ImageLoader::loadPvr(basename, file) : ImageLoader::loadEct1(basename, file);
#else
    TODO
    ImageDesc image;
    std::stringstream a;
    a << "assets/" << basename << ".png";
    std::string aa = a.str();
    SDL_Surface* s = IMG_Load(aa.c_str());
    if (s == 0) {
        LOG(WARNING) << "Failed to load '" << a.str() << "'";
        return whiteTexture;
    }
    VLOG(1) << ("Image format: " << s->w << 'x' << s->h << ' ' << s->format->BitsPerPixel);
    image.channels = s->format->BitsPerPixel / 8;

    image.width = s->w;
    image.height = s->h;
    image.datas = new char[image.width * image.height * image.channels];
    memcpy(image.datas, s->pixels, image.width * image.height * image.channels);
    if (!colorOrAlpha && image.channels==4) {
        for (int i=0; i<image.height; i++) {
            for (int j=0; j<image.width; j++) {
                char* pixel = &image.datas[i * image.width * 4 + j * 4];
                pixel[3] = pixel[0]; // assign red channel to alpha
            }
        }
    }
    SDL_FreeSurface(s);
    png = true;
#endif  
}
