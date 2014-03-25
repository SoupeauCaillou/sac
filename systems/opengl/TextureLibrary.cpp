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



#include "TextureLibrary.h"
#include "OpenGLTextureCreator.h"

InternalTexture InternalTexture::Invalid;

TextureInfo::TextureInfo (const InternalTexture& ref,
        const glm::vec2& posInAtlas, const glm::vec2& sizeInAtlas, bool rot,
        const glm::vec2& atlasSize,
        const glm::vec2& offsetInOriginal, const glm::vec2& pOriginalSize,
        const glm::vec2& _opaqueStart, const glm::vec2& _opaqueSize,
        int atlasIdx) {
    glref = ref;

    if (pOriginalSize == glm::vec2(0.0f)) {
        uv[0].x = uv[0].y = 0;
        uv[1].x = uv[1].y = 1;
        rotateUV = 0;
    } else if (atlasIdx >= 0) {
        // position is specified in pixels, from top left corner
        float lX = posInAtlas.x / atlasSize.x;
        float rX = (posInAtlas.x + sizeInAtlas.x) / atlasSize.x;
        float tY = 1 - (posInAtlas.y + sizeInAtlas.y) / atlasSize.y;
        float bY = 1 - posInAtlas.y / atlasSize.y;

        uv[0].x = lX;
        uv[1].x = rX;
        uv[0].y = tY;
        uv[1].y = bY;
        rotateUV = rot;
    } else {
        uv[0].x = posInAtlas.x / atlasSize.x;
        uv[0].y = posInAtlas.y / atlasSize.y;
        uv[1].x = (posInAtlas.x + sizeInAtlas.x) / atlasSize.x;
        uv[1].y = (posInAtlas.y + sizeInAtlas.y) / atlasSize.y;
        rotateUV = 0;
    }
    atlasIndex = atlasIdx;

    originalSize = pOriginalSize;

    glm::vec2 _sizeInAtlas(sizeInAtlas);
    if (rot) {
        std::swap(_sizeInAtlas.x, _sizeInAtlas.y);
    }
    if (_sizeInAtlas.y > 0) {
        opaqueSize = glm::vec2(_opaqueSize.x / _sizeInAtlas.x, _opaqueSize.y / _sizeInAtlas.y);
        if (0) {//if (0 && rot) {
            opaqueStart = glm::vec2(_opaqueStart.x / _sizeInAtlas.x, _opaqueStart.y / _sizeInAtlas.y);
        } else {
            opaqueStart = glm::vec2(_opaqueStart.x / _sizeInAtlas.x, 1 - (opaqueSize.y + _opaqueStart.y / _sizeInAtlas.y));
        }

        reduxSize = glm::vec2(_sizeInAtlas.x / originalSize.x, _sizeInAtlas.y / originalSize.y);
        reduxStart = glm::vec2(offsetInOriginal.x / originalSize.x, 1 - (reduxSize.y + offsetInOriginal.y / originalSize.y));
    }
}

bool TextureLibrary::doLoad(const std::string& assetName, TextureInfo& out, const TextureRef& ref) {
    LOGF_IF(assetAPI == 0,"Unitialized assetAPI member");

    std::map<TextureRef, ImageDesc>::iterator it = dataSource.find(ref);
    if (it == dataSource.end()) {
        LOGV(1, "loadTexture: '" << assetName << "' from file");
        out.glref = OpenGLTextureCreator::loadFromFile(assetAPI, assetName, out.originalSize);
        #if SAC_LINUX && SAC_DESKTOP
        registerNewAsset(assetName + "_alpha");
        #endif
    } else {
        const ImageDesc& imageDesc = it->second;
        LOGV(1, "loadTexture: '" << assetName << "' from ImageDesc (" << imageDesc.width << "x" << imageDesc.height << "@" << imageDesc.channels << ')');
        out.glref.color =
            out.glref.alpha =
                OpenGLTextureCreator::loadFromImageDesc(imageDesc, assetName, OpenGLTextureCreator::COLOR_ALPHA, out.originalSize);
        out.reduxSize = glm::vec2(1.0f,1.0f);
        out.opaqueSize = glm::vec2(0.0f);
    }

    out.rotateUV = false;
    out.atlasIndex = -1;
    out.uv[0] = glm::vec2(0.0f);
    out.uv[1] = glm::vec2(1.0f);
    out.reduxSize = glm::vec2(1.0f);
    out.reduxStart = out.opaqueStart = out.opaqueSize = glm::vec2(0.0f);
    return true;
}

void TextureLibrary::doUnload(const std::string& LOG_USAGE_ONLY(name), const TextureInfo& in) {

    LOGI("Unload atlas: '" << name << "'");
    if (in.glref.color) {
        LOGV(1, "   delete color texture");
        glDeleteTextures(1, &in.glref.color);
    }
    if (in.glref.alpha) {
        LOGV(1, "   delete alpha texture");
        glDeleteTextures(1, &in.glref.alpha);
    }
}

void TextureLibrary::doReload(const std::string& name, const TextureRef& ref) {
    TextureInfo& info = assets[ref2Index(ref)];
    if (info.atlasIndex == -1) {
        doLoad(name, info, ref);
    }
    return;
    //std::map<TextureRef, ImageDesc>::iterator it = dataSource.find(ref);
    //if (it == dataSource.end()) {
    //    LOGT("TextureLibrary::doReload (" << name << ")");
    //} else {
    //    const ImageDesc& imageDesc = it->second;
    //    LOGV(1, "update texture: '" << name << "' from ImageDesc (" << imageDesc.width << "x" << imageDesc.height << "@" << imageDesc.channels << ')');
    //    OpenGLTextureCreator::updateFromImageDesc(imageDesc, info.glref.color, OpenGLTextureCreator::COLOR_ALPHA);
    //}
}
