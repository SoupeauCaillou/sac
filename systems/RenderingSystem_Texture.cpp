#include "RenderingSystem.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
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
	while(!s.empty()) {
		// LOGI("atlas - line: %s", s.c_str());
		std::string assetName;
		int x, y, w, h;
		bool rot;

		parse(s, assetName, x, y, w, h, rot);

		TextureRef result = nextValidRef++;
		assetTextures[assetName] = result;
		textures[result] = TextureInfo(a.glref, x, y, w, h, rot, atlasSize, atlasIndex);
		
		s.clear();
		f >> s;
		count++;
	}
	delete[] file.data;
	LOGI("Atlas '%s' loaded %d images", atlasName.c_str(), count);
}

void RenderingSystem::invalidateAtlasTextures() {
    for (unsigned int i=0; i<atlas.size(); i++) {
        atlas[i].glref = InternalTexture::Invalid;
    }
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
    for (unsigned int i=0; i<atlas.size(); i++) {
        if (atlasName == atlas[i].name) {
            for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end();) {
	            if (it->second.glref == atlas[i].glref) {
	                textures.erase(it++);
                } else {
	                it++;
                }
            }
            delayedDeletes.insert(atlas[i].glref);
            // atlas.erase(atlas.begin() + i);
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
    	file = assetAPI->loadAsset(basename + ".pkm");
	}
#endif
    if (!file.data) {
        file = assetAPI->loadAsset(basename + ".png");
        if (!file.data)
            return whiteTexture;
        png = true;
    }

    // load image
    ImageDesc image = png ? ImageLoader::loadPng(basename, file) : ImageLoader::loadEct1(basename, file);
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
	    LOGW("Using ETC texture version");
	#ifdef ANDROID
		#define ECT1_HEADER_SIZE 16
	    GL_OPERATION(glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, image.width, image.height, 0, file.size - ECT1_HEADER_SIZE, image.datas))
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
    // load atlas
    for (std::set<int>::iterator it=delayedAtlasIndexLoad.begin(); it != delayedAtlasIndexLoad.end(); ++it) {
        int atlasIndex = *it;
        Vector2 atlasSize, pow2Size;
        loadTexture(atlas[atlasIndex].name, atlasSize, pow2Size, atlas[atlasIndex].glref);
        LOGW("Atlas '%s' loaded (%u/%u)", atlas[atlasIndex].name.c_str(), atlas[atlasIndex].glref.color, atlas[atlasIndex].glref.alpha);

        for (std::map<std::string, TextureRef>::iterator jt=assetTextures.begin(); jt!=assetTextures.end(); ++jt) {
            TextureInfo& info = textures[jt->second];
            if (info.atlasIndex == atlasIndex) {
                info.glref = atlas[atlasIndex].glref;
            }
        }
    }
    delayedAtlasIndexLoad.clear();

    // load textures
    for (std::set<std::string>::iterator it=delayedLoads.begin(); it != delayedLoads.end(); ++it) {
        Vector2 size, powSize;
        InternalTexture t;
        loadTexture(*it, size, powSize, t);
        textures[assetTextures[*it]] = TextureInfo(t, 1+1, 1+1, size.X-1, size.Y-1, false, powSize);
    }
    delayedLoads.clear();

    // delete textures
    for (std::set<InternalTexture>::iterator it=delayedDeletes.begin(); it != delayedDeletes.end(); ++it) {
	    if (it->color != whiteTexture) {
		    LOGW("Color texture delete: %u", it->color);
        	glDeleteTextures(1, &it->color);
	    }
	    if (it->alpha > 0 && it->alpha != whiteTexture) {
	    	LOGW("Alpha texture delete: %u", it->alpha);
        	glDeleteTextures(1, &it->alpha);
	    }
    }
    delayedDeletes.clear();
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	TextureRef result = InvalidTextureRef;
	std::string name(assetName);

	if (assetTextures.find(name) != assetTextures.end()) {
		result = assetTextures[name];
	} else {
		result = nextValidRef++;
		assetTextures[name] = result;
	}
	if (textures.find(result) == textures.end()) {
		pthread_mutex_lock(&mutexes[current]);
		delayedLoads.insert(name);
		pthread_mutex_unlock(&mutexes[current]);
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
                pthread_mutex_lock(&mutexes[current]);
                delayedDeletes.insert(textures[ref].glref);
                pthread_mutex_unlock(&mutexes[current]);
                break;
            }
        }
    } else {
        LOGW("Tried to delete an InvalidTextureRef");
    }
}

