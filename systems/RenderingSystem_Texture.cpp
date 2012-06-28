/*
	This file is part of Heriswap.

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
#include "RenderingSystem.h"
#ifndef ANDROID
#include <GL/glew.h>
#else
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif
#include "base/EntityManager.h"
#include <cmath>
#include <cassert>
#include <sstream>
#include <sys/select.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../util/ImageLoader.h"

RenderingSystem::TextureInfo::TextureInfo (const InternalTexture& ref, int x, int y, int w, int h, bool rot, const Vector2& size,  int atlasIdx) {
	glref = ref;

	if (size == Vector2::Zero) {
		uv[0].X = uv[0].Y = 0;
		uv[1].X = uv[1].Y = 1;
		rotateUV = 0;
	} else if (atlasIdx >= 0) {
		float blX = x / size.X;
		float trX = (x+w) / size.X;
		float blY = 1 - (y+h) / size.Y;
		float trY = 1 - y / size.Y;

		uv[0].X = blX;
		uv[1].X = trX;
		uv[0].Y = blY;
		uv[1].Y = trY;
		rotateUV = rot;
	} else {
		uv[0].X = x / size.X;
		uv[0].Y = y / size.Y;
		uv[1].X = (x+w) / size.X;
		uv[1].Y = (y+h) / size.Y;
		rotateUV = 0;
	}
	atlasIndex = atlasIdx;
}

#include <fstream>

static void parse(const std::string& line, std::string& assetName, int& x, int& y, int& w, int& h, bool& rot) {
	std::string substrings[6];
	int from = 0, to = 0;
	for (int i=0; i<6; i++) {
		to = line.find_first_of(',', from);
		substrings[i] = line.substr(from, to - from);
		from = to + 1;
	}
	assetName = substrings[0];
	x = atoi(substrings[1].c_str());
	y = atoi(substrings[2].c_str());
	w = atoi(substrings[3].c_str());
	h = atoi(substrings[4].c_str());
	rot = atoi(substrings[5].c_str());
}

void RenderingSystem::loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading) {
	std::string atlasDesc = atlasName + ".desc";
	std::string atlasImage = atlasName;
	
	FileBuffer file = assetAPI->loadAsset(atlasDesc);
	if (!file.data) {
		LOGW("Unable to load atlas desc %s", atlasDesc.c_str());
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
    LOGW("atlas '%s' -> index: %d, glref: [%u, %u], size:[%f,%f] ('%s')", atlasName.c_str(), atlasIndex, a.glref.color, a.glref.alpha, atlasSize.X, atlasSize.Y, s.c_str());
	int count = 0;
	
	do {
		s.clear();
		f >> s;
		if (s.empty())
			break;
		count++;
		// LOGI("atlas - line: %s", s.c_str());
		std::string assetName;
		int x, y, w, h;
		bool rot;

		parse(s, assetName, x, y, w, h, rot);

		TextureRef result = nextValidRef++;
		LOGW("----- %s -> %d", assetName.c_str(), result);
		assetTextures[assetName] = result;
		textures[result] = TextureInfo(a.glref, x, y, w, h, rot, atlasSize, atlasIndex);
		
		
	} while (!s.empty());
	
	delete[] file.data;
	LOGI("Atlas '%s' loaded %d images", atlasName.c_str(), count);
}

void RenderingSystem::invalidateAtlasTextures() {
    for (unsigned int i=0; i<atlas.size(); i++) {
        atlas[i].glref = InternalTexture::Invalid;
    }
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
	LOGW("Unload atlas '%s' texture", atlasName.c_str());
    for (unsigned int idx=0; idx<atlas.size(); idx++) {
        if (atlasName == atlas[idx].name) {
            for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end(); ++it) {
	            if (it->second.atlasIndex == idx) {
		            it->second.glref = InternalTexture::Invalid;
                }
            }
            delayedDeletes.insert(atlas[idx].glref);
            atlas[idx].glref = InternalTexture::Invalid;
            break;
        }
    }
}

static unsigned int alignOnPowerOf2(unsigned int value) {
	for(int i=0; i<32; i++) {
		unsigned int c = 1 << i;
		if (value <= c)
			return c;
	}
	return 0;
}

GLuint RenderingSystem::createGLTexture(const std::string& basename, bool colorOrAlpha, Vector2& realSize, Vector2& pow2Size) {
    FileBuffer file;
    bool png = false;
    file.data = 0;
#ifdef ANDROID
	if (colorOrAlpha) {
		#ifdef PVR_SUPPORT
    	file = assetAPI->loadAsset(basename + ".pvr");
    	#else
    	file = assetAPI->loadAsset(basename + ".pkm");
    	#endif
	}
#endif
    if (!file.data) {
        file = assetAPI->loadAsset(basename + ".png");
        if (!file.data)
            return whiteTexture;
        png = true;
    }

    // load image
    #ifdef PVR_SUPPORT
    ImageDesc image = png ? ImageLoader::loadPng(basename, file) : ImageLoader::loadPvr(basename, file);
    #else
    ImageDesc image = png ? ImageLoader::loadPng(basename, file) : ImageLoader::loadEct1(basename, file);
    #endif
    delete[] file.data;
    if (!image.datas) {
        return 0;
    }

    // for now, just assume power of 2 size
    GLuint out;
    GL_OPERATION(glGenTextures(1, &out))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, out))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))

    GLenum format;
    switch (image.channels) {
        case 1:
            format = GL_ALPHA;
            break;
        case 2:
            format = GL_LUMINANCE_ALPHA;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }
    
    if (png) {
	    LOGW("Using PNG texture version");
    	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, colorOrAlpha ? GL_RGB:GL_ALPHA, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, NULL))
    	GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, image.datas))
    } else {
	   #ifdef ANDROID
	    #ifdef PVR_SUPPORT
	    unsigned imgSize = ( MathUtil::Max(image.width, 8) * MathUtil::Max(image.height, 8) * 4 + 7) / 8; // file.size;// - (20 + 13*sizeof(uint32_t));
	    LOGW("Using PVR texture version (%dx%d, %d)", image.width, image.height, imgSize);
	    GL_OPERATION(glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, image.width, image.height, 0, imgSize, image.datas))
	    #else
	    LOGW("Using ETC texture version");
		#define ECT1_HEADER_SIZE 16
		GL_OPERATION(glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, image.width, image.height, 0, file.size - ECT1_HEADER_SIZE, image.datas))
	   	#endif 
    #else
    	assert (false && "ETC compression not supported");
    #endif
    }
    free (image.datas);
    pow2Size.X = realSize.X = image.width;
    pow2Size.Y = realSize.Y = image.height;

    return out;
}


void RenderingSystem::loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size, InternalTexture& out) {
    LOGW("loadTexture: %s", assetName.c_str());

	/* create GL texture */
#ifdef GLES2_SUPPORT
	if (!opengles2)
#endif
		GL_OPERATION(glEnable(GL_TEXTURE_2D))

	out.color = createGLTexture(assetName, true, realSize, pow2Size);
	out.alpha = createGLTexture(assetName + "_alpha", false, realSize, pow2Size);
}

void RenderingSystem::reloadTextures() {
	Vector2 size, psize;
	LOGW("Reloading textures begin");
    LOGW("\t- atlas : %lu", atlas.size());
	// reload atlas texture
	for (unsigned int i=0; i<atlas.size(); i++) {
		atlas[i].glref = InternalTexture::Invalid;
	}
    LOGW("\t - textures: %lu", assetTextures.size());
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		TextureInfo& info = textures[it->second];
		if (info.atlasIndex >= 0) {
			info.glref = atlas[info.atlasIndex].glref;
		} else {
			InternalTexture t;
			loadTexture(it->first, size, psize, t);
			textures[it->second] = TextureInfo(t, 0, 0, size.X, size.Y, false, psize);
		}
	}
    LOGW("Reloading textures done");
}

void RenderingSystem::processDelayedTextureJobs() {
	pthread_mutex_lock(&mutexes);
	
    // load atlas
    for (std::set<int>::iterator it=delayedAtlasIndexLoad.begin(); it != delayedAtlasIndexLoad.end(); ++it) {
        int atlasIndex = *it;
        Vector2 atlasSize, pow2Size;
        pthread_mutex_unlock(&mutexes);
        loadTexture(atlas[atlasIndex].name, atlasSize, pow2Size, atlas[atlasIndex].glref);
        LOGW("Atlas '%s' loaded (%u/%u)", atlas[atlasIndex].name.c_str(), atlas[atlasIndex].glref.color, atlas[atlasIndex].glref.alpha);

        for (std::map<std::string, TextureRef>::iterator jt=assetTextures.begin(); jt!=assetTextures.end(); ++jt) {
            TextureInfo& info = textures[jt->second];
            if (info.atlasIndex == atlasIndex) {
                info.glref = atlas[atlasIndex].glref;
            }
        }
        pthread_mutex_lock(&mutexes);
    }
    delayedAtlasIndexLoad.clear();

    // load textures
    for (std::set<std::string>::iterator it=delayedLoads.begin(); it != delayedLoads.end(); ++it) {
        Vector2 size, powSize;
        InternalTexture t;
        pthread_mutex_unlock(&mutexes);
        loadTexture(*it, size, powSize, t);
        textures[assetTextures[*it]] = TextureInfo(t, 1+1, 1+1, size.X-1, size.Y-1, false, powSize);
        pthread_mutex_lock(&mutexes);
    }
    delayedLoads.clear();

    // delete textures
    for (std::set<InternalTexture>::iterator it=delayedDeletes.begin(); it != delayedDeletes.end(); ++it) {
	    pthread_mutex_unlock(&mutexes);
	    if (it->color != whiteTexture) {
		    LOGW("Color texture delete: %u", it->color);
        	glDeleteTextures(1, &it->color);
	    }
	    if (it->alpha > 0 && it->alpha != whiteTexture) {
	    	LOGW("Alpha texture delete: %u", it->alpha);
        	glDeleteTextures(1, &it->alpha);
	    }
	    pthread_mutex_lock(&mutexes);
    }
    delayedDeletes.clear();
    pthread_mutex_unlock(&mutexes);
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	TextureRef result = InvalidTextureRef;
	std::string name(assetName);

	if (assetTextures.find(name) != assetTextures.end()) {
		result = assetTextures[name];
	} else {
		result = nextValidRef++;
		assetTextures[name] = result;
		LOGW("Texture '%s' doesn't belong to any atlas. Will be loaded individually", assetName.c_str());
	}
	if (textures.find(result) == textures.end()) {
		pthread_mutex_lock(&mutexes);
		delayedLoads.insert(name);
		pthread_mutex_unlock(&mutexes);
	}
		
	return result;
}

void RenderingSystem::unloadTexture(TextureRef ref, bool allowUnloadAtlas) {
    if (ref != InvalidTextureRef) {
        TextureInfo i = textures[ref];
        if (i.atlasIndex >= 0 && !allowUnloadAtlas) {
            LOGW("Cannot delete texture '%d' (is an atlas)", ref);
            return;
        }
        for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
            if (it->second == ref) {
                assetTextures.erase(it);
                pthread_mutex_lock(&mutexes);
                delayedDeletes.insert(textures[ref].glref);
                pthread_mutex_unlock(&mutexes);
                break;
            }
        }
    } else {
        LOGW("Tried to delete an InvalidTextureRef");
    }
}

