#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"
#include "opengl/OpenGLTextureCreator.h"

#include "base/EntityManager.h"
#include <cmath>
#include <cassert>
#include <sstream>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

static void parse(const std::string& line, std::string& assetName, Vector2& originalSize, Vector2& reduxOffset, Vector2& posInAtlas, Vector2& sizeInAtlas, bool& rotate, Vector2& opaqueStart, Vector2& opaqueEnd) {
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
	originalSize.X = atoi(substrings[1].c_str());
	originalSize.Y = atoi(substrings[2].c_str());
	reduxOffset.X = atoi(substrings[3].c_str());
	reduxOffset.Y = atoi(substrings[4].c_str());
    posInAtlas.X = atoi(substrings[5].c_str());
    posInAtlas.Y = atoi(substrings[6].c_str());
    sizeInAtlas.X = atoi(substrings[7].c_str());
    sizeInAtlas.Y = atoi(substrings[8].c_str());
	rotate = atoi(substrings[9].c_str());
    if (count == 14) {
        opaqueStart.X = atoi(substrings[10].c_str());
        opaqueStart.Y = atoi(substrings[11].c_str());
        opaqueEnd.X = atoi(substrings[12].c_str());
        opaqueEnd.Y = atoi(substrings[13].c_str());
    }
}

void RenderingSystem::loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading) {
	std::string atlasDesc = atlasName + ".desc";
	std::string atlasImage = atlasName;

	FileBuffer file = assetAPI->loadAsset(atlasDesc);
	if (!file.data) {
		LOG(FATAL) << "Unable to load atlas description file '" << atlasDesc << "'";
		return;
	}

	Vector2 atlasSize, pow2Size;
	Atlas a;
	a.name = atlasImage;
	if (forceImmediateTextureLoading) {
        a.ref = textureLibrary.load(atlasName);
	} else {
		a.ref = InvalidTextureRef;
	}
	atlas.push_back(a);
	int atlasIndex = atlas.size() - 1;

	std::stringstream f(std::string((const char*)file.data, file.size), std::ios_base::in);
	std::string s;
	f >> s;

	// read texture size
	sscanf(s.c_str(), "%f,%f", &atlasSize.X, &atlasSize.Y);
	VLOG(1) << "atlas '" << atlasName << "' -> index: " << atlasIndex;
	int count = 0;

	do {
		s.clear();
		f >> s;
		if (s.empty())
			break;
		count++;
		VLOG(2) << "atlas - line: " << s;
		std::string assetName;
        Vector2 originalSize, reduxOffset, posInAtlas, sizeInAtlas, opaqueStart(Vector2::Zero), opaqueEnd(Vector2::Zero);
		bool rot;
		parse(s, assetName, originalSize, reduxOffset, posInAtlas, sizeInAtlas, rot, opaqueStart, opaqueEnd);

        const TextureInfo info(InternalTexture::Invalid, posInAtlas, sizeInAtlas, rot, atlasSize, reduxOffset, originalSize, opaqueStart, opaqueEnd - opaqueStart, atlasIndex);
        textureLibrary.add(assetName, info);
	} while (!s.empty());

	delete[] file.data;
	VLOG(1) << "Atlas '" << atlasName << "' loaded " << count << " images";
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
    pthread_mutex_lock(&mutexes[L_QUEUE]);
    TextureRef result = textureLibrary.load(assetName);
    pthread_mutex_unlock(&mutexes[L_QUEUE]);
    return result;
}

Vector2 RenderingSystem::getTextureSize(const std::string& textureName) {
    const TextureInfo& info = textureLibrary.get(textureName);
    return Vector2(info.originalWidth, info.originalHeight);
}

void RenderingSystem::unloadTexture(TextureRef ref, bool allowUnloadAtlas) {
	if (ref != InvalidTextureRef) {
		const TextureInfo* info = textureLibrary.get(ref, true);
        if (info) {
    		if (info->atlasIndex >= 0 && !allowUnloadAtlas) {
                LOG(ERROR) << "Cannot delete texture '" << ref << "' (is an atlas)";
    	    } else {
                textureLibrary.unload(ref);
            }
        }
	} else {
		LOG(ERROR) << "Tried to delete an InvalidTextureRef";
	}
}

