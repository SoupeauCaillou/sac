#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"

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
		loadTexture(atlasName, atlasSize, pow2Size, a.glref);
	} else {
		a.glref = InternalTexture::Invalid;
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
		// LOGI("atlas - line: %s", s.c_str());
		std::string assetName;
        Vector2 originalSize, reduxOffset, posInAtlas, sizeInAtlas, opaqueStart(Vector2::Zero), opaqueEnd(Vector2::Zero);
		bool rot;
		parse(s, assetName, originalSize, reduxOffset, posInAtlas, sizeInAtlas, rot, opaqueStart, opaqueEnd);

        const TextureInfo info(a.glref, posInAtlas, sizeInAtlas, rot, atlasSize, reduxOffset, originalSize, opaqueStart, opaqueEnd - opaqueStart, atlasIndex);
        textureLibrary.add(assetName, info);
	} while (!s.empty());

	delete[] file.data;
	VLOG(1) << "Atlas '" << atlasName << "' loaded " << count << " images";
}

void RenderingSystem::invalidateAtlasTextures() {
	for (unsigned int i=0; i<atlas.size(); i++) {
		atlas[i].glref = InternalTexture::Invalid;
	}
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
    LOG(WARNING) << "TODO";
    #if 0
	LOG(INFO) << "Unload atlas '" << atlasName << "' texture";
	for (unsigned int idx=0; idx<atlas.size(); idx++) {
		if (atlasName == atlas[idx].name) {
			for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end(); ++it) {
				if (it->second.atlasIndex == (int)idx) {
					it->second.glref = InternalTexture::Invalid;
				}
			}
			delayedDeletes.insert(atlas[idx].glref);
			atlas[idx].glref = InternalTexture::Invalid;
			break;
		}
	}
    #endif
}

void RenderingSystem::loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size, InternalTexture& out) {
	VLOG(1) << "loadTexture: '" << assetName << "'";

	/* create GL texture */
 	out.color = openGLTextureCreator.loadFromFile(assetAPI, assetName, realSize);
	out.alpha = openGLTextureCreator.loadFromFile(assetAPI, assetName + "_alpha", realSize);
    pow2Size = realSize;
}

void RenderingSystem::reloadTextures() {
    LOG(WARNING) << "TODO";
    #if 0
	LOG(INFO) << "Reloading textures begin";
	LOG(INFO) << "\t- atlas count: " << atlas.size();
	// reload atlas texture
	for (unsigned int i=0; i<atlas.size(); i++) {
		atlas[i].glref = InternalTexture::Invalid;
	}
    LOG(INFO) << "\t- textures count: " << assetTextures.size();
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		TextureInfo& info = textures[it->second];
		if (info.atlasIndex >= 0) {
			info.glref = atlas[info.atlasIndex].glref;
		} else {
            Vector2 size, psize;
			loadTexture(it->first, size, psize, info.glref);
		}
	}
	LOG(INFO) << "Reloading textures done";
    #endif
}

void RenderingSystem::processDelayedTextureJobs() {
	PROFILE("Texture", "processDelayedTextureJobs", BeginEvent);
	#ifndef EMSCRIPTEN
	pthread_mutex_lock(&mutexes[L_TEXTURE]);
	#endif

	// load atlas
	for (std::set<int>::iterator it=delayedAtlasIndexLoad.begin(); it != delayedAtlasIndexLoad.end(); ++it) {
		int atlasIndex = *it;
		Vector2 atlasSize, pow2Size;
	    #ifndef EMSCRIPTEN
		pthread_mutex_unlock(&mutexes[L_TEXTURE]);
		#endif
		loadTexture(atlas[atlasIndex].name, atlasSize, pow2Size, atlas[atlasIndex].glref);
		VLOG(1) << "Atlas '" << atlas[atlasIndex].name << "' loaded (" << atlas[atlasIndex].glref.color << '/' << atlas[atlasIndex].glref.alpha << ')';

        LOG(ERROR) << "TODO: update atlas members glref";
        #if 0
		for (std::map<std::string, TextureRef>::iterator jt=assetTextures.begin(); jt!=assetTextures.end(); ++jt) {
			TextureInfo& info = textures[jt->second];
			if (info.atlasIndex == atlasIndex) {
				info.glref = atlas[atlasIndex].glref;
			}
		}
        #endif
	    #ifndef EMSCRIPTEN
        pthread_mutex_lock(&mutexes[L_TEXTURE]);
        #endif
	}
	delayedAtlasIndexLoad.clear();
    
    textureLibrary.update();

	// delete textures
	for (std::set<InternalTexture>::iterator it=delayedDeletes.begin(); it != delayedDeletes.end(); ++it) {
	    #ifndef EMSCRIPTEN
        pthread_mutex_unlock(&mutexes[L_TEXTURE]);
        #endif
		if (it->color != whiteTexture) {
			VLOG(1) << "Color texture delete:" << it->color;
			glDeleteTextures(1, &it->color);
		}
		if (it->alpha > 0 && it->alpha != whiteTexture) {
			VLOG(1) << "Alpha texture delete: " << it->alpha;
			glDeleteTextures(1, &it->alpha);
		}
	    #ifndef EMSCRIPTEN
        pthread_mutex_lock(&mutexes[L_TEXTURE]);
        #endif
	}
	delayedDeletes.clear();
	#ifndef EMSCRIPTEN
    pthread_mutex_unlock(&mutexes[L_TEXTURE]);
    #endif
	PROFILE("Texture", "processDelayedTextureJobs", EndEvent);
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	PROFILE("Texture", "loadTextureFile", BeginEvent);
    return textureLibrary.load(assetName);
}

Vector2 RenderingSystem::getTextureSize(const std::string& textureName) const {
    const TextureInfo& info = textureLibrary.get(textureName);
    return Vector2(info.originalWidth, info.originalHeight);
}

void RenderingSystem::unloadTexture(TextureRef ref, bool allowUnloadAtlas) {
    LOG(WARNING) << "TODO";
    /*
	if (ref != InvalidTextureRef) {
		TextureInfo i = textures[ref];
		if (i.atlasIndex >= 0 && !allowUnloadAtlas) {
			LOG(ERROR) << "Cannot delete texture '" << ref << "' (is an atlas";
			return;
		}
		for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
			if (it->second == ref) {
				assetTextures.erase(it);
			    #ifndef EMSCRIPTEN
				pthread_mutex_lock(&mutexes[L_TEXTURE]);
				#endif
				delayedDeletes.insert(textures[ref].glref);
				#ifndef EMSCRIPTEN
				pthread_mutex_unlock(&mutexes[L_TEXTURE]);
				#endif
				break;
			}
		}
	} else {
		LOG(ERROR) << "Tried to delete an InvalidTextureRef";
	}
    */
}

