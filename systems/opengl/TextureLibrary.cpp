#include "TextureLibrary.h"
#include "OpenGLTextureCreator.h"

InternalTexture InternalTexture::Invalid;

TextureInfo::TextureInfo (const InternalTexture& ref,
        const Vector2& posInAtlas, const Vector2& sizeInAtlas, bool rot,
        const Vector2& atlasSize,
        const Vector2& offsetInOriginal, const Vector2& pOriginalSize,
        const Vector2& _opaqueStart, const Vector2& _opaqueSize,
        int atlasIdx) {
    glref = ref;

    if (pOriginalSize == Vector2::Zero) {
        uv[0].X = uv[0].Y = 0;
        uv[1].X = uv[1].Y = 1;
        rotateUV = 0;
    } else if (atlasIdx >= 0) {
        float blX = posInAtlas.X / atlasSize.X;
        float trX = (posInAtlas.X + sizeInAtlas.X) / atlasSize.X;
        float blY = 1 - (posInAtlas.Y + sizeInAtlas.Y) / atlasSize.Y;
        float trY = 1 - posInAtlas.Y / atlasSize.Y;

        uv[0].X = blX;
        uv[1].X = trX;
        uv[0].Y = blY;
        uv[1].Y = trY;
        rotateUV = rot;
    } else {
        uv[0].X = posInAtlas.X / atlasSize.X;
        uv[0].Y = posInAtlas.Y / atlasSize.Y;
        uv[1].X = (posInAtlas.X + sizeInAtlas.X) / atlasSize.X;
        uv[1].Y = (posInAtlas.Y + sizeInAtlas.Y) / atlasSize.Y;
        rotateUV = 0;
    }
    atlasIndex = atlasIdx;

    originalSize = pOriginalSize;

    Vector2 _sizeInAtlas(sizeInAtlas);
    if (rot) {
        std::swap(_sizeInAtlas.X, _sizeInAtlas.Y);
    }
    if (_sizeInAtlas.Y > 0) {
        opaqueSize = Vector2(_opaqueSize.X / _sizeInAtlas.X, _opaqueSize.Y / _sizeInAtlas.Y);
        opaqueStart = Vector2(_opaqueStart.X / _sizeInAtlas.X, 1 - (opaqueSize.Y + _opaqueStart.Y / _sizeInAtlas.Y));

        reduxSize = Vector2(_sizeInAtlas.X / originalSize.X, _sizeInAtlas.Y / originalSize.Y);
        reduxStart = Vector2(offsetInOriginal.X / originalSize.X, 1 - (reduxSize.Y + offsetInOriginal.Y / originalSize.Y));
    }
}

bool TextureLibrary::doLoad(const std::string& assetName, TextureInfo& out, const TextureRef& ref) {
    LOGF_IF(assetAPI == 0,"Unitialized assetAPI member")

    std::map<TextureRef, ImageDesc>::iterator it = dataSource.find(ref);
    if (it == dataSource.end()) {
        LOGV(1, "loadTexture: '" << assetName << "' from file")
        out.glref = OpenGLTextureCreator::loadFromFile(assetAPI, assetName, out.originalSize);
    } else {
        const ImageDesc& imageDesc = it->second;
        LOGV(1, "loadTexture: '" << assetName << "' from ImageDesc (" << imageDesc.width << "x" << imageDesc.height << "@" << imageDesc.channels << ')')
        out.glref.color =
            out.glref.alpha =
                OpenGLTextureCreator::loadFromImageDesc(imageDesc, assetName, OpenGLTextureCreator::COLOR_ALPHA, out.originalSize);
        out.reduxSize = Vector2(1,1);
        out.opaqueSize = Vector2::Zero;
    }

    out.rotateUV = false;
    out.atlasIndex = -1;
    out.uv[0] = Vector2::Zero;
    out.uv[1] = Vector2(1, 1);
    out.reduxSize = Vector2(1,1);
    out.reduxStart = out.opaqueStart = out.opaqueSize = Vector2::Zero;
    return true;
}

void TextureLibrary::doUnload(const std::string& /*name*/, const TextureInfo& /*in*/) {
    LOGW("TODO")
}

void TextureLibrary::doReload(const std::string& name, const TextureRef& ref) {
    TextureInfo& info = ref2asset[ref];
    std::map<TextureRef, ImageDesc>::iterator it = dataSource.find(ref);
    if (it == dataSource.end()) {
        LOGV(1, "TODO")
    } else {
        const ImageDesc& imageDesc = it->second;
        LOGV(1, "update texture: '" << name << "' from ImageDesc (" << imageDesc.width << "x" << imageDesc.height << "@" << imageDesc.channels << ')')
        OpenGLTextureCreator::updateFromImageDesc(imageDesc, info.glref.color, OpenGLTextureCreator::COLOR_ALPHA);
    }   
}

void TextureLibrary::add(const std::string& name, const TextureInfo& info) {
    mutex.lock();
    TextureRef ref = nextValidRef++;
    nameToRef.insert(std::make_pair(name, ref));
    ref2asset.insert(std::make_pair(ref, info));
    mutex.unlock();
}
