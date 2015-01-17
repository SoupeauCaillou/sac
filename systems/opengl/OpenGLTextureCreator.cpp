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



#include "OpenGLTextureCreator.h"
#include "api/AssetAPI.h"
#include "base/Log.h"
#include <cstring>
#if SAC_ANDROID
#include <GLES2/gl2ext.h>
#elif SAC_IOS
#include <OpenGLES/ES2/glext.h>
#elif SAC_EMSCRIPTEN
#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#endif

#include "OpenglHelper.h"

#define ALPHA_MASK_TAG "_alpha"

DPI::Enum OpenGLTextureCreator::dpi = DPI::High;

static bool pvrFormatSupported = false;
static bool pkmFormatSupported = false;
static bool s3tcFormatSupported = false;

std::string OpenGLTextureCreator::DPI2Folder(DPI::Enum dpi) {
    switch (dpi) {
        case DPI::Low:
            return "ldpi";
        case DPI::Medium:
            return "mdpi";
        case DPI::High:
            return "hdpi";
        default:
            LOGF("Unhandled DPI setting");
    }
    return "";
}

void OpenGLTextureCreator::detectSupportedTextureFormat() {
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);

    LOGV(1, "extensions: " << extensions );
#if SAC_IOS
    pvrFormatSupported = (strstr((const char*)extensions, "GL_IMG_texture_compression_pvrtc") != 0);
#else
    pvrFormatSupported = false;
#endif
#if SAC_EMSCRIPTEN
    s3tcFormatSupported = (strstr((const char*)extensions, "WEBGL_compressed_texture_s3tc") != 0);
#else
    s3tcFormatSupported = (strstr((const char*)extensions, "GL_EXT_texture_compression_s3tc") != 0);
#endif

#if SAC_ANDROID
    pkmFormatSupported = true;
#else
    pkmFormatSupported = (strstr((const char*)extensions, "GL_OES_compressed_ETC1_RGB8_texture") != 0);
#endif
    LOGV(1, "Supported texture format:");
    LOGV(1, " - PVR : " << pvrFormatSupported );
    LOGV(1, " - PKM : " << pkmFormatSupported );
    LOGV(1, " - S3TC: " << s3tcFormatSupported );
}

#if SAC_DESKTOP
void OpenGLTextureCreator::forceEtc1Usage() {
    LOGW("ETC1 texture usage forced");
    pvrFormatSupported = s3tcFormatSupported = false;
}
#endif

static GLenum channelCountToGLFormat(int channelCount) {
    GLenum format = 0;
    switch (channelCount) {
        case 1:
            format = GL_ALPHA;
            LOGV(2,  channelCount << " -> GL_ALPHA");
            break;
        case 2:
            format = GL_LUMINANCE_ALPHA;
            LOGV(2, channelCount << " -> GL_LUMINANCE_ALPHA");
            break;
        case 3:
            format = GL_RGB;
            LOGV(2, channelCount << " -> GL_RGB");
            break;
        case 4:
            format = GL_RGBA;
            LOGV(2, channelCount << " -> GL_RGBA");
            break;
        default:
            LOGF("Invalid channel count: " << channelCount);
    }
    return format;
}

static GLuint createAndInitTexture(bool enableMipmapping) {
    GLuint result;
    // Create OpenGL texture, and initialize
    GL_OPERATION(glGenTextures(1, &result))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, result))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
    // Enable mipmapping when supported
    if (enableMipmapping) {
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST))
    } else {
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
        GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
    }
    return result;
}

GLuint OpenGLTextureCreator::create(const glm::vec2& size, int channels, void* imageData) {
    GLenum format = channelCountToGLFormat(channels);

    GLuint result = createAndInitTexture(false);

    // Create OpenGL texture, and initialize
    GL_OPERATION(glGenTextures(1, &result))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, result))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))

    // Allocate texture space
    GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, format, GL_UNSIGNED_BYTE, NULL))

    // upload data, if any
    if (imageData)
        GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format, GL_UNSIGNED_BYTE, imageData))

    return result;
}

InternalTexture OpenGLTextureCreator::loadFromFile(AssetAPI* assetAPI, const std::string& name, glm::vec2& outSize) {
    InternalTexture result;
    result.color = result.alpha = 0;
    int imgChannelCount = 0;
    result.color = loadSplittedFromFile(assetAPI, name, COLOR, outSize, imgChannelCount);
    result.alpha = loadSplittedFromFile(assetAPI, name + "_alpha", ALPHA_MASK, outSize, imgChannelCount);

    return result;
}
GLuint OpenGLTextureCreator::loadSplittedFromFile(AssetAPI* assetAPI, const std::string& name, Type type, glm::vec2& outSize, int& imgChannelCount) {
    // Read file content
    FileBuffer file;
    bool png = false;

    // First, try PVR compression, then PKM (ETC1)
    const char* extension = DefaultFileExtension();
    LOGV(1, "Loading " << name << extension);
    file = assetAPI->loadAsset(name + extension);

    if (!file.data) {
        LOGV(1, "Using PNG version - " << name);
        file = assetAPI->loadAsset(name + ".png");
        if (!file.data) {
            LOGE("Image not found '" << name << ".png'");
            return 0;
        }
        png = true;
    }

    // Parse image
    ImageDesc image = parseImageContent(name, file, png);
    delete[] file.data;
    if (!image.datas) {
        LOGE("Could not read image, aborting");
        return 0;
    }
    #if SAC_EMSCRIPTEN
    LOGT("Remove this non-sense");
    if (type == ALPHA_MASK && image.channels==4) {
        for (int i=0; i<image.height; i++) {
            for (int j=0; j<image.width; j++) {
                char* pixel = &image.datas[i * image.width * 4 + j * 4];
                pixel[3] = pixel[0]; // assign red channel to alpha
            }
        }
    }
    #endif
    imgChannelCount = image.channels;

    GLuint result = loadFromImageDesc(image, name, type, outSize);

    delete[] image.datas;

    return result;
}

#if 0 && ! SAC_ANDROID
static GLenum typeToFormat(OpenGLTextureCreator::Type type) {
    switch (type) {
        case OpenGLTextureCreator::COLOR:
            return GL_RGB;
        case OpenGLTextureCreator::ALPHA_MASK:
            return GL_ALPHA;
        case OpenGLTextureCreator::COLOR_ALPHA:
            return GL_RGBA;
        default:
            LOGF("Unhandled typeToFormat value: " << type);
     }
     return GL_INVALID_VALUE;
}
#endif

static GLenum imageDescToGLenum(ImageDesc desc) {
    switch (desc.type) {
        case ImageDesc::RAW: {
#if SAC_EMSCRIPTEN
            return GL_RGBA;
#else
            return channelCountToGLFormat(desc.channels);
#endif
        }
#if SAC_MOBILE
#if SAC_ANDROID
        case ImageDesc::ETC1: {
            LOGW_IF(desc.channels != 3, "Incoherent channel count " << desc.channels << " while using ETC1 compression");
            return GL_ETC1_RGB8_OES;
        }
#endif
        case ImageDesc::PVR: {
            LOGW_IF(desc.channels != 3, "Incoherent channel count " << desc.channels << " while using PVR compression");
            return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        }
#endif
#if ! SAC_IOS
        case ImageDesc::S3TC:
            LOGW_IF(desc.channels != 3, "Incoherent channel count " << desc.channels << " while using S3TC compression");
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif
        default:
            LOGF("Invalid desc.type: " << desc.type);
            return 0;
    }
}

void OpenGLTextureCreator::updateFromImageDesc(const ImageDesc& image, GLuint texture, Type) {
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, texture))

    // Determine GL format based on channel count
    GLenum format = imageDescToGLenum(image);

    if (image.type == ImageDesc::RAW) {
        LOGV(1, "Using PNG texture version " << image.width << 'x' << image.height);
        GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, NULL))
        GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, image.datas))
    } else {
        char* ptr = image.datas;
        LOGV(1, "Using texture type '" << image.type << "' (" << image.width << 'x' << image.height << " - " << image.mipmap << " mipmap)");
        for (int level=0; level<=image.mipmap; level++) {
            int width = std::max(1, image.width >> level);
            int height = std::max(1, image.height >> level);
#if SAC_IOS
            width = height = glm::max(width, height);
#endif
            unsigned imgSize = 0;
            if (pvrFormatSupported)
                imgSize =( std::max(width, 8) * std::max(height, 8) * 4 + 7) / 8;
            else
                imgSize = 8 * ((width + 3) >> 2) * ((height + 3) >> 2);
            LOGV(3, "\t- mipmap " << level << " : " << width << 'x' << height);
            GL_OPERATION(glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, imgSize, ptr))
            ptr += imgSize;
        }
    }

#if SAC_MOBILE || SAC_EMSCRIPTEN
#else
    const bool enableMipMapping = false;
    if (image.mipmap == 0 && enableMipMapping) {
        LOGV(1, "Generating mipmaps");
        GL_OPERATION(glGenerateMipmap(GL_TEXTURE_2D))
    }
#endif
}

GLuint OpenGLTextureCreator::loadFromImageDesc(const ImageDesc& image, const std::string& /*name*/, Type type, glm::vec2& outSize) {
#if 0
    const bool enableMipMapping =
    #if SAC_ANDROID
        ((type == COLOR) && image.mipmap > 0);
    #elif SAC_EMSCRIPTEN
        false;
    #else
        (type == COLOR) || (type == COLOR_ALPHA);
    #endif

    // Create GL texture object
    GLuint result = createAndInitTexture(enableMipMapping);
#else
    GLuint result = createAndInitTexture(false);
#endif
    updateFromImageDesc(image, result, type);

    outSize.x = image.width;
    outSize.y = image.height;

    return result;
}

ImageDesc OpenGLTextureCreator::parseImageContent(const std::string& basename, const FileBuffer& file, bool isPng) {
    // load image (ugh)
    return isPng ?
        ImageLoader::loadPng(basename, file) :
            pvrFormatSupported ? ImageLoader::loadPvr(basename, file):
            s3tcFormatSupported ? ImageLoader::loadDDS(basename, file):
                ImageLoader::loadEtc1(basename, file, pkmFormatSupported);
}

const char* OpenGLTextureCreator::DefaultFileExtension() {
    if (pvrFormatSupported) {
        return ".pvr";
    } else if (s3tcFormatSupported) {
        return ".dds";
    } else {
        return ".pkm";
    }
}

