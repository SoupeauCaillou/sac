#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"
#include "opengl/OpenGLTextureCreator.h"

#include "base/EntityManager.h"
#include "util/DataFileParser.h"
#include <cmath>
#include <cassert>
#include <sstream>

#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

#if 0
static void parse(const std::string& line, std::string& assetName, glm::vec2& originalSize, glm::vec2& reduxOffset, glm::vec2& posInAtlas, glm::vec2& sizeInAtlas, bool& rotate, glm::vec2& opaqueStart, glm::vec2& opaqueEnd) {
	std::string substrings[14];
	int from = 0, to = 0, count = 0;
	for (count=0; count<14; count++) {
		to = line.find_first_of(',', from);
		substrings[count] = line.substr(from, to - from);
        if (to == (int)std::string::npos) {
            count++;
            break;
        }
		from = to + 1;
	}
    // image,original width, original height, redux offset x, redux offset y, pox x in atlas, pos y in atlas, width, height, rotate[, opaque box min x, min y, max x, max y]
	assetName = substrings[0];
	originalSize.x = atoi(substrings[1].c_str());
	originalSize.y = atoi(substrings[2].c_str());
	reduxOffset.x = atoi(substrings[3].c_str());
	reduxOffset.y = atoi(substrings[4].c_str());
    posInAtlas.x = atoi(substrings[5].c_str());
    posInAtlas.y = atoi(substrings[6].c_str());
    sizeInAtlas.x = atoi(substrings[7].c_str());
    sizeInAtlas.y = atoi(substrings[8].c_str());
	rotate = atoi(substrings[9].c_str());
    if (count == 14) {
        opaqueStart.x = atoi(substrings[10].c_str());
        opaqueStart.y = atoi(substrings[11].c_str());
        opaqueEnd.x = atoi(substrings[12].c_str());
        opaqueEnd.y = atoi(substrings[13].c_str());
    }
}
#endif
void RenderingSystem::loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading) {
	const std::string atlasDesc = atlasName + ".atlas";

	FileBuffer file = assetAPI->loadAsset(atlasDesc);
	if (!file.data) {
		LOGF("Unable to load atlas description file '" << atlasDesc << "'")
		return;
	}
    DataFileParser dfp;
    if (!dfp.load(file)) {
        LOGE("Unable to parse '" << atlasDesc << "'")
        delete[] file.data;
        return;
    }


	Atlas a;
	a.name = atlasName;
	if (forceImmediateTextureLoading) {
        a.ref = textureLibrary.load(atlasName);
	} else {
		a.ref = InvalidTextureRef;
	}
	atlas.push_back(a);
	int atlasIndex = atlas.size() - 1;
    int count = 0;

    glm::vec2 atlasSize;
    if (!dfp.get("", "atlas_size", &atlasSize.x, 2)) {
        LOGE("Missing 'atlas_size' attribute in '" << atlasDesc << "'");
        goto cleanup;
    }
	LOGV(1, "atlas '" << atlasName << "' -> index: " << atlasIndex)
	do {
        std::stringstream sectionB;
        sectionB << "image" << count;
        const std::string section(sectionB.str());

        if (!dfp.hasSection(section))
            break;

		std::string assetName;
        if (!dfp.get(section, "name", &assetName, 1)) {
            LOGE(atlasDesc << ": missing 'name' in section '" << section << "'")
            goto cleanup;
        }
        glm::vec2 originalSize;
        if (!dfp.get(section, "original_size", &originalSize.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'original_size' in section '" << section << "'")
            goto cleanup;
        }
        glm::vec2 posInAtlas;
        if (!dfp.get(section, "position_in_atlas", &posInAtlas.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'position_in_atlas' in section '" << section << "'")
            goto cleanup;
        }
        glm::vec2 sizeInAtlas;
        if (!dfp.get(section, "size_in_atlas", &sizeInAtlas.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'size_in_atlas' in section '" << section << "'")
            goto cleanup;
        }
        glm::vec2 reduxOffset;
        if (!dfp.get(section, "crop_offset", &reduxOffset.x, 2)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'crop_offset' in section '" << section << "'")
            goto cleanup;
        }
        int rotate;
        if (!dfp.get(section, "rotated", &rotate, 1)) {
            LOGE(atlasDesc << '/' << assetName << ": missing 'rotated' in section '" << section << "'")
            goto cleanup;
        }
        glm::vec4 opaqueRect;
        if (!dfp.get(section, "opaque_rect", &opaqueRect.x, 4, false)) {
            LOGV(1, "No 'opaque_rect' in section '" << section << "' for image " << assetName)
            opaqueRect = glm::vec4(0.0f);
        }
        glm::vec2 start(opaqueRect.swizzle(glm::X, glm::Y));
        glm::vec2 size(opaqueRect.swizzle(glm::Z, glm::W));
        const TextureInfo info(InternalTexture::Invalid, posInAtlas, sizeInAtlas, rotate, atlasSize, reduxOffset, originalSize, start, size, atlasIndex);
        textureLibrary.add(assetName, info);
        count++;
	} while (true);

	LOGV(1, "Atlas '" << atlasName << "' loaded " << count << " images")
cleanup:
    delete[] file.data;
}

void RenderingSystem::invalidateAtlasTextures() {
	for (unsigned int i=0; i<atlas.size(); i++) {
		atlas[i].ref = InvalidTextureRef;
	}
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
    textureLibrary.unload(atlasName);
}

void RenderingSystem::reloadTextures() {
    // Mark atlas textures invalid
	invalidateAtlasTextures(); // not useful me thinks

	// reload individual textures
    textureLibrary.reloadAll();
}

void RenderingSystem::processDelayedTextureJobs() {
	PROFILE("Texture", "processDelayedTextureJobs", BeginEvent);

    textureLibrary.update();

	PROFILE("Texture", "processDelayedTextureJobs", EndEvent);
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	PROFILE("Texture", "loadTextureFile", BeginEvent);
    mutexes[L_QUEUE].lock();
    TextureRef result = textureLibrary.load(assetName);
    mutexes[L_QUEUE].unlock();
    return result;
}

const glm::vec2& RenderingSystem::getTextureSize(const std::string& textureName) {
    const TextureInfo& info = textureLibrary.get(textureName);
    return info.originalSize;
}

void RenderingSystem::unloadTexture(TextureRef ref, bool allowUnloadAtlas) {
	if (ref != InvalidTextureRef) {
		const TextureInfo* info = textureLibrary.get(ref, true);
        if (info) {
    		if (info->atlasIndex >= 0 && !allowUnloadAtlas) {
                LOGE("Cannot delete texture '" << ref << "' (is an atlas)")
    	    } else {
                textureLibrary.unload(ref);
            }
        }
	} else {
		LOGE("Tried to delete an InvalidTextureRef")
	}
}
