/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    Heriswap is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Heriswap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <glm/glm.hpp>
#include "util/ImageLoader.h"
#include "TextureLibrary.h"
class AssetAPI;

#if SAC_ANDROID || SAC_EMSCRIPTEN
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif


class OpenGLTextureCreator {
    public:
        enum Type { COLOR, ALPHA_MASK, COLOR_ALPHA };

        static void detectSupportedTextureFormat();

        static InternalTexture loadFromFile(AssetAPI* assetAPI, const std::string& name, glm::vec2& outSize);

        static GLuint loadFromImageDesc(const ImageDesc& imageDesc, const std::string& name, Type type, glm::vec2& outSize);

        static void updateFromImageDesc(const ImageDesc& imagedesc, GLuint texture, Type type);

        static GLuint create(const glm::vec2& size, int channels, void* imageData = 0);

    private:
        static ImageDesc parseImageContent(const std::string& filename, const FileBuffer& file, bool isPng);
        static GLuint loadSplittedFromFile(AssetAPI* assetAPI, const std::string& name, Type type, glm::vec2& outSize, int& imgChannelCount);
};
