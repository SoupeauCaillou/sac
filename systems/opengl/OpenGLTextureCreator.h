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

#pragma once

#include <glm/glm.hpp>
#include "util/ImageLoader.h"
#include "TextureLibrary.h"
class AssetAPI;

#if SAC_ANDROID || SAC_EMSCRIPTEN
#include <GLES2/gl2.h>
#elif SAC_IOS
#include <OpenGLES/ES2/gl.h>
#else
#include <GL/glew.h>
#endif

namespace DPI {
    enum Enum { Low, Medium, High };
}
class OpenGLTextureCreator {
    public:
    static DPI::Enum dpi;
    static std::string DPI2Folder(DPI::Enum dpi);
    static const char* DefaultFileExtension();

    public:
    enum Type { COLOR, ALPHA_MASK, COLOR_ALPHA };

    static void detectSupportedTextureFormat();

#if SAC_DESKTOP
    static void forceEtc1Usage();
#endif

    static InternalTexture loadFromFile(AssetAPI* assetAPI,
                                        const std::string& name,
                                        glm::vec2& outSize);

    static GLuint loadFromImageDesc(const ImageDesc& imageDesc,
                                    const std::string& name,
                                    Type type,
                                    glm::vec2& outSize);

    static void
    updateFromImageDesc(const ImageDesc& imagedesc, GLuint texture, Type type);

    static GLuint
    create(const glm::vec2& size, int channels, void* imageData = 0);

    private:
    static ImageDesc parseImageContent(const std::string& filename,
                                       const FileBuffer& file,
                                       bool isPng);
    static GLuint loadSplittedFromFile(AssetAPI* assetAPI,
                                       const std::string& name,
                                       Type type,
                                       glm::vec2& outSize,
                                       int& imgChannelCount);
};
