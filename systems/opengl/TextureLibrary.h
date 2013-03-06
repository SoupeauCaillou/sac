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

#include "base/NamedAssetLibrary.h"
#include "base/Vector2.h"
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
    unsigned short rotateUV;
    // which atlas
    short atlasIndex;
    // uv coords in atlas
    Vector2 uv[2];
    // texture original size
    unsigned short originalWidth, originalHeight;
    // texture redux offset/size
    Vector2 reduxStart, reduxSize;
    // coordinates of opaque region in alpha-enabled texture (optional)
    Vector2 opaqueStart, opaqueSize;
    TextureInfo (const InternalTexture& glref = InternalTexture::Invalid,
        const Vector2& posInAtlas = Vector2::Zero, const Vector2& sizeInAtlas = Vector2::Zero, bool rot = false,
        const Vector2& atlasSize = Vector2::Zero,
        const Vector2& offsetInOriginal = Vector2::Zero, const Vector2& originalSize=Vector2::Zero,
        const Vector2& opaqueStart = Vector2::Zero, const Vector2& opaqueSize=Vector2::Zero,
        int atlasIdx = -1);
};

typedef int TextureRef;

class TextureLibrary : public NamedAssetLibrary<TextureInfo, TextureRef, ImageDesc> {
    protected:
        bool doLoad(const std::string& name, TextureInfo& out, const TextureRef& ref);

        void doUnload(const std::string& name, const TextureInfo& in);

        void reload(const std::string& name, TextureInfo& out);
    public:
        void add(const std::string& name, const TextureInfo& info);
};