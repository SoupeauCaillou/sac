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



#if 0
#include "AtlasLibrary.h"
#include "api/AssetAPI.h"

void AtlasLibrary::doLoad(const std::string& name, Atlas& atlas, const TRef& ref) {
    const std::string atlasDesc(atlasName + ".desc");
    const std::string atlasImage(atlasName);

    // Load file content
    FileBuffer file = assetAPI->loadAsset(atlasDesc);
    if (!file.data) {
        LOGE("Unable to load atlas desc %s", atlasDesc.c_str());
        return;
    }

    // Init atlas' fields
    atlas.name = atlasImage;
    atlas.glref = InternalTexture::Invalid;

    Vector2 atlasSize, pow2Size;

    // Parse atlas description file content
    std::stringstream f(std::string((const char*)file.data, file.size), std::ios_base::in);
    std::string s;
    f >> s;
    
    // First: read texture size
    sscanf(s.c_str(), "%f,%f", &atlasSize.X, &atlasSize.Y);
    int count = 0;
    // Then: one line per texture
    do {
        s.clear();
        f >> s;
        // Stop on empty line
        if (s.empty())
            break;
        count++;
        std::string assetName;
        // Parse texture description content
        Vector2 originalSize, reduxOffset, posInAtlas, sizeInAtlas, opaqueStart(Vector2::Zero), opaqueEnd(Vector2::Zero);
        bool rot;
        parse(s, assetName, originalSize, reduxOffset, posInAtlas, sizeInAtlas, rot, opaqueStart, opaqueEnd);

        TextureRef result = nextValidRef++;
        assetTextures[assetName] = result;
        textures[result] = TextureInfo(a.glref, posInAtlas, sizeInAtlas, rot, atlasSize, reduxOffset, originalSize, opaqueStart, opaqueEnd - opaqueStart, atlasIndex);
    } while (!s.empty());

    delete[] file.data;
    LOGI("Atlas '%s' loaded %d images", atlasName.c_str(), count);

}


void AtlasLibrary::doUnload(const std::string& name, const T& in) {

}

void AtlasLibrary::reload(const std::string& name, T& out) {

}
#endif
