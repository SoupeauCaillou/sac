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

#include "base/NamedAssetLibrary.h"
#include <glm/glm.hpp>
#include "OpenglHelper.h"
#include "util/ImageLoader.h"

struct InternalTexture {
    GLuint color;
    GLuint alpha;

    bool operator==(const InternalTexture& t) const {
        return color == t.color && alpha == t.alpha;
    }
    bool operator!=(const InternalTexture& t) const {
        return color != t.color || alpha != t.alpha;
    }
    bool operator<(const InternalTexture& t) const {
        return color < t.color;
    }

    static InternalTexture Invalid;
};

struct TextureInfo {
    // GL texture(s)
    InternalTexture glref;
    // is image rotated in atlas
    bool rotateUV;
    // which atlas
    short atlasIndex;
    // uv coords in atlas
    glm::vec2 uv[2];
    // texture original size
    glm::vec2 originalSize;
    // texture redux offset/size
    glm::vec2 reduxStart, reduxSize;
    // coordinates of opaque region in alpha-enabled texture (optional)
    glm::vec2 opaqueStart, opaqueSize;
    TextureInfo (const InternalTexture& glref = InternalTexture::Invalid,
        const glm::vec2& posInAtlas = glm::vec2(0), const glm::vec2& sizeInAtlas = glm::vec2(0.0f), bool rot = false,
        const glm::vec2& atlasSize = glm::vec2(0),
        const glm::vec2& offsetInOriginal = glm::vec2(0), const glm::vec2& originalSize=glm::vec2(0.0f),
        const glm::vec2& opaqueStart = glm::vec2(0), const glm::vec2& opaqueSize=glm::vec2(0.0f),
        int atlasIdx = -1);
};

typedef hash_t TextureRef;
#define InvalidTextureRef 0

class TextureLibrary : public NamedAssetLibrary<TextureInfo, TextureRef, ImageDesc> {
    protected:
        bool doLoad(const char* name, TextureInfo& out, const TextureRef& ref);

        void doUnload(const TextureInfo& in);

        void doReload(const char* name, const TextureRef& ref);

    public:
        const char* asset2FilePrefix() const { return ""; }
        const char* asset2FileSuffix() const;

};
