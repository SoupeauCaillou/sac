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



#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"
#include "opengl/OpenGLTextureCreator.h"

#include "util/DataFileParser.h"

#include <stdint.h>
#include <fstream>

void RenderingSystem::loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading) {
    const std::string atlasDesc = atlasName + ".atlas";

    FileBuffer file = assetAPI->loadAsset(atlasDesc);
    if (!file.data) {
        LOGF("Unable to load atlas description file '" << atlasDesc << "'");
        return;
    }
    DataFileParser dfp;
    if (!dfp.load(file, atlasDesc)) {
        LOGE("Unable to parse '" << atlasDesc << "'");
        delete[] file.data;
        return;
    }


    Atlas a;
    a.name = atlasName;
    if (forceImmediateTextureLoading) {
        a.ref = textureLibrary.load(atlasName.c_str());
    } else {
        a.ref = InvalidTextureRef;
    }
    atlas.push_back(a);
    int atlasIndex = atlas.size() - 1;
    int count = 0;

    glm::vec2 atlasSize;
    if (!dfp.get("", "atlas_size", &atlasSize.x, 2)) {
        LOGE("Missing 'atlas_size' attribute in '" << atlasDesc << "'");;
        goto cleanup;
    }
    LOGV(1, "atlas '" << atlasName << "' -> index: " << atlasIndex);
    do {
        std::stringstream sectionB;
        sectionB << "image" << count;
        const std::string section(sectionB.str());

        if (!dfp.hasSection(section))
            break;

        std::string assetName;
        if (!dfp.get(section, "name", &assetName, 1)) {
            LOGE(atlasDesc << ": missing 'name' in section '" << section << "'");
            goto cleanup;
        }
        glm::vec2 originalSize;
        if (!dfp.get(section, "original_size", &originalSize.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'original_size' in section '" << section << "'");
            goto cleanup;
        }
        glm::vec2 posInAtlas;
        if (!dfp.get(section, "position_in_atlas", &posInAtlas.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'position_in_atlas' in section '" << section << "'");
            goto cleanup;
        }
        glm::vec2 sizeInAtlas;
        if (!dfp.get(section, "size_in_atlas", &sizeInAtlas.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'size_in_atlas' in section '" << section << "'");
            goto cleanup;
        }
        glm::vec2 reduxOffset;
        if (!dfp.get(section, "crop_offset", &reduxOffset.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'crop_offset' in section '" << section << "'");
            goto cleanup;
        }
        int rotate;
        if (!dfp.get(section, "rotated", &rotate, 1)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'rotated' in section '" << section << "'");
            goto cleanup;
        }
        glm::vec4 opaqueRect;
        if (!dfp.get(section, "opaque_rect", &opaqueRect.x, 4, false)) {
            LOGV(1, "No 'opaque_rect' in section '" << section << "' for image " << assetName);
            opaqueRect = glm::vec4(0.0f);
        }
        glm::vec2 start(opaqueRect.x, opaqueRect.y);
        glm::vec2 size(opaqueRect.z, opaqueRect.w);
        const TextureInfo info(InternalTexture::Invalid, posInAtlas, sizeInAtlas, rotate, atlasSize, reduxOffset, originalSize, start, size, atlasIndex);
        textureLibrary.add(assetName, info);
        count++;
    } while (true);

    LOGV(1, "Atlas '" << atlasName << "' loaded " << count << " images");
cleanup:
    delete[] file.data;
}

void RenderingSystem::invalidateAtlasTextures() {
    for (unsigned atlasIdx=0; atlasIdx<atlas.size(); atlasIdx++) {
        // Invalidate only loaded textures
        if (atlas[atlasIdx].ref != InvalidTextureRef) {
            LOGI("Invalidate atlas: #" << atlasIdx << ": ref='" << atlas[atlasIdx].ref << "', name='" << atlas[atlasIdx].name << "'");
            textureLibrary.reload(atlas[atlasIdx].name.c_str());
        }
    }
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
    std::stringstream realName;
    realName << OpenGLTextureCreator::DPI2Folder(OpenGLTextureCreator::dpi)
        << '/' << atlasName;
    textureLibrary.unload(Murmur::RuntimeHash(realName.str().c_str()));
    for (unsigned i=0; i<atlas.size(); i++) {
        if (atlas[i].name == realName.str()) {
            atlas[i].ref = InvalidTextureRef;
        }
    }

}

void RenderingSystem::reloadTextures() {
    // Mark atlas textures invalid
    invalidateAtlasTextures();

    // reload individual textures
    // textureLibrary.reloadAll();
    effectLibrary.reloadAll();

    // rebuild framebuffers too
    for (auto& fb: nameToFramebuffer) {
        const Framebuffer f = ref2Framebuffers[fb.second];
        createFramebuffer(fb.first, f.width, f.height);
    }
}

void RenderingSystem::processDelayedTextureJobs() {
    PROFILE("Texture", "processDelayedTextureJobs", BeginEvent);

    textureLibrary.update();
    effectLibrary.update();

    PROFILE("Texture", "processDelayedTextureJobs", EndEvent);
}

TextureRef RenderingSystem::loadTextureFile(const char* assetName) {
    PROFILE("Texture", "loadTextureFile", BeginEvent);
#ifndef SAC_EMSCRIPTEN
    mutexes[L_QUEUE].lock();
#endif
    TextureRef result = textureLibrary.load(assetName);
#ifndef SAC_EMSCRIPTEN
    mutexes[L_QUEUE].unlock();
#endif
    PROFILE("Texture", "loadTextureFile", EndEvent);
    return result;
}

glm::vec2 RenderingSystem::getTextureSize(const char* textureName) {
    return getTextureSize(Murmur::RuntimeHash(textureName));
}

glm::vec2 RenderingSystem::getTextureSize(const TextureRef& textureRef) {
    const TextureInfo* info = textureLibrary.get(textureRef, false);

    switch (OpenGLTextureCreator::dpi) {
        case DPI::Low:
            return info->originalSize * 4.0f;
        case DPI::Medium:
            return info->originalSize * 2.0f;
        default:
            break;
    }
    return info->originalSize;
}

void RenderingSystem::unloadTexture(TextureRef ref, bool allowUnloadAtlas) {
    if (ref != InvalidTextureRef) {
        const TextureInfo* info = textureLibrary.get(ref, true);
        if (info) {
            if (info->atlasIndex >= 0 && !allowUnloadAtlas) {
                LOGE("Cannot delete texture '" << ref << "' (is an atlas)");
            } else {
                textureLibrary.unload(ref);
            }
        }
    } else {
        LOGE("Tried to delete an InvalidTextureRef");
    }
}
