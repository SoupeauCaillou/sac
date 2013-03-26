#include "OpenGLTextureCreator.h"
#include "api/AssetAPI.h"
#include "base/Log.h"
#include <cstring>

#include "OpenglHelper.h"

#define ALPHA_MASK_TAG "_alpha"

static bool pvrFormatSupported = false;
static bool pkmFormatSupported = false;

void OpenGLTextureCreator::detectSupportedTextureFormat() {
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    pvrFormatSupported = (strstr((const char*)extensions, "GL_IMG_texture_compression_pvrtc") != 0);
    #ifdef ANDROID
    pkmFormatSupported = true;
    #endif
}

static GLenum channelCountToGLFormat(int channelCount) {
    GLenum format;
    switch (channelCount) {
        case 1:
            format = GL_ALPHA;
            LOGV(2,  channelCount << " -> GL_ALPHA")
            break;
        case 2:
            format = GL_LUMINANCE_ALPHA;
            LOGV(2, channelCount << " -> GL_LUMINANCE_ALPHA")
            break;
        case 3:
            format = GL_RGB;
            LOGV(2, channelCount << " -> GL_RGB")
            break;
        case 4:
            format = GL_RGBA;
            LOGV(2, channelCount << " -> GL_RGBA")
            break;
        default:
            LOGF("Invalid channel count: " << channelCount)
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

GLuint OpenGLTextureCreator::create(const Vector2& size, int channels, void* imageData) {
    GLenum format = channelCountToGLFormat(channels);

    GLuint result = createAndInitTexture(false);

    // Create OpenGL texture, and initialize
    GL_OPERATION(glGenTextures(1, &result))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, result))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))

    // Allocate texture space
    GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, format, size.X, size.Y, 0, format, GL_UNSIGNED_BYTE, NULL))

    // upload data, if any
    if (imageData)
        GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.X, size.Y, format, GL_UNSIGNED_BYTE, imageData))

    return result;
}

InternalTexture OpenGLTextureCreator::loadFromFile(AssetAPI* assetAPI, const std::string& name, Vector2& outSize) {
    InternalTexture result;
    result.color = result.alpha = 0;
    int imgChannelCount = 0;
    result.color = loadSplittedFromFile(assetAPI, name, COLOR, outSize, imgChannelCount);
    if (result.color && imgChannelCount == 4) {
        LOGV(1, name << " texture has 4 channels. Use it for alpha as well")
        result.alpha = loadSplittedFromFile(assetAPI, name, ALPHA_MASK, outSize, imgChannelCount);
    } else {
        result.alpha = loadSplittedFromFile(assetAPI, name + "_alpha", ALPHA_MASK, outSize, imgChannelCount);
    }
    return result;
}
GLuint OpenGLTextureCreator::loadSplittedFromFile(AssetAPI* assetAPI, const std::string& name, Type type, Vector2& outSize, int& imgChannelCount) {
    // Read file content
    FileBuffer file;
    bool png = false;

    // First, try PVR compression, then PKM (ETC1)
    if (type == COLOR) {
        if (pvrFormatSupported) {
            LOGV(2, "Using PVR version")
            file = assetAPI->loadAsset(name + ".pvr");
        } else if (pkmFormatSupported) {
            LOGV(2, "Using PKM version")
            file = assetAPI->loadAsset(name + ".pkm");
        } else {
            LOGV(1, "PVM nor ETC1 supported, falling back on PNG format")
        }
    }

    if (!file.data) {
        LOGV(2, "Using PNG version")
        file = assetAPI->loadAsset(name + ".png");
        if (!file.data) {
            LOGE("Image not found '" << name << ".png'")
            return false;
        }
        png = true;
    }

    // Parse image
    ImageDesc image = parseImageContent(name, file, png);
    delete[] file.data;
    if (!image.datas) {
        LOGE("Could not read image, aborting")
        return false;
    }
    imgChannelCount = image.channels;

    GLuint result = loadFromImageDesc(image, name, type, outSize);

    delete[] image.datas;

    return result;
}

static GLenum typeToFormat(OpenGLTextureCreator::Type type) {
    switch (type) {
        case OpenGLTextureCreator::COLOR:
            return GL_RGB;
        case OpenGLTextureCreator::ALPHA_MASK:
            return GL_ALPHA;
        case OpenGLTextureCreator::COLOR_ALPHA:
            return GL_RGBA;
        default:
            LOGF("Unhandled typeToFormat value: " << type)
     }
     return GL_INVALID_VALUE;
}

void OpenGLTextureCreator::updateFromImageDesc(const ImageDesc& image, GLuint texture, Type type) {
    const bool enableMipMapping =
#ifdef ANDROID
    ((type == COLOR) && image.mipmap > 0);
#elif defined(EMSCRIPTEN)
    false;
#else
    (type == COLOR) || (type == COLOR_ALPHA);
#endif

    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, texture))

    // Determine GL format based on channel count
    GLenum format = channelCountToGLFormat(image.channels);

    if (image.type == ImageDesc::RAW) {
        LOGV(2, "Using PNG texture version " << image.width << 'x' << image.height)
        #ifdef EMSCRIPTEN
        GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, NULL))
        #else
        GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, typeToFormat(type), image.width, image.height, 0, format, GL_UNSIGNED_BYTE, NULL))
        #endif
        GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, image.datas))
    } else {
       #ifdef ANDROID
        char* ptr = image.datas;
        LOGV(2, "Using " << (pvrSupported ? "PVR" : "ETC1") << " texture version (" << image.width << 'x' << image.height << " - " << image.mipmap << " mipmap)")
        for (int level=0; level<=image.mipmap; level++) {
            int width = MathUtil::Max(1, image.width >> level);
            int height = MathUtil::Max(1, image.height >> level);
            unsigned imgSize = 0;
            if (pvrSupported) 
                imgSize =( MathUtil::Max(width, 8) * MathUtil::Max(height, 8) * 4 + 7) / 8;
            else
                imgSize = 8 * ((width + 3) >> 2) * ((height + 3) >> 2);
            LOGV(3, "\t- mipmap " << level << " : " << width << 'x' << height)
            GL_OPERATION(glCompressedTexImage2D(GL_TEXTURE_2D, level, pvrSupported ? GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG : GL_ETC1_RGB8_OES, width, height, 0, imgSize, ptr))
            ptr += imgSize;
        }
        #else
        LOGF("ETC compression not supported")
        return;
        #endif
    }

#if !defined(ANDROID) && !defined(EMSCRIPTEN)
    if (image.mipmap == 0 && enableMipMapping) {
        LOGV(1, "Generating mipmaps")
        glGenerateMipmap(GL_TEXTURE_2D);
    }
#endif
}

GLuint OpenGLTextureCreator::loadFromImageDesc(const ImageDesc& image, const std::string& /*name*/, Type type, Vector2& outSize) {
    const bool enableMipMapping =
#ifdef ANDROID
    ((type == COLOR) && image.mipmap > 0);
#elif defined(EMSCRIPTEN)
    false;
#else
    (type == COLOR) || (type == COLOR_ALPHA);
#endif

    // Create GL texture object
    GLuint result = createAndInitTexture(enableMipMapping);
    
    updateFromImageDesc(image, result, type);

    outSize.X = image.width;
    outSize.Y = image.height;

    return result;
}

ImageDesc OpenGLTextureCreator::parseImageContent(const std::string& basename, const FileBuffer& file, bool isPng) {
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
        LOGW("Failed to load '" << a.str() << "'")
        return whiteTexture;
    }
    LOGV(1, ("Image format: " << s->w << 'x' << s->h << ' ' << s->format->BitsPerPixel))
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
