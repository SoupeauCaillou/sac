#include "RenderingSystem.h"
#include <GLES/gl.h>
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


RenderingSystem::TextureInfo::TextureInfo (GLuint r, int x, int y, int w, int h, bool rot, const Vector2& size,  int atlasIdx) {
	glref = r;		
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

void RenderingSystem::loadAtlas(const std::string& atlasName) {
	std::string atlasDesc = atlasName + ".desc";
	std::string atlasImage = atlasName + ".png";
	
	const char* desc = assetLoader->loadShaderFile(atlasDesc);
	if (!desc) {
		LOGW("Unable to load atlas desc %s", atlasDesc.c_str());
		return;
	}
	
	Vector2 atlasSize, pow2Size;
	GLuint glref = loadTexture(atlasImage, atlasSize, pow2Size);
	Atlas a;
	a.name = atlasImage;
	a.texture = glref;
	atlas.push_back(a);
	int atlasIndex = atlas.size() - 1;
	LOGW("atlas '%s' -> index: %d, glref: %u", atlasName.c_str(), atlasIndex, glref);
	
	std::stringstream f(std::string(desc), std::ios_base::in);
	std::string s;
	f >> s;
	int count = 0;
	while(!s.empty()) {
		// LOGI("atlas - line: %s", s.c_str());
		std::string assetName;
		int x, y, w, h;
		bool rot;

		parse(s, assetName, x, y, w, h, rot);

		TextureRef result = nextValidRef++;
		assetTextures[assetName] = result;
		textures[result] = TextureInfo(glref, x, y, w, h, rot, atlasSize, atlasIndex);
		
		s.clear();
		f >> s;
		count++;
	}
	delete[] desc;
	LOGI("Atlas '%s' loaded %d images", atlasName.c_str(), count);
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
    for (int i=0; i<atlas.size(); i++) {
        if (atlasName == atlas[i].name) {
            for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end();) {
                std::map<TextureRef, TextureInfo>::iterator next = ++it;
                if (it->second.glref == atlas[i].texture) {
                    textures.erase(it);
                }
                it = next;
            }
            unloadTexture(atlas[i].texture, true);
            atlas.erase(atlas.begin() + i);
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

GLuint RenderingSystem::loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size) {
	int w,h;
	char* data = assetLoader->decompressPngImage(assetName, &w, &h);

#ifndef ANDROID
{
    std::stringstream s;
    s << "./assets/" << assetName;
    NotifyInfo info;
    info.wd = inotify_add_watch(inotifyFd, s.str().c_str(), IN_CLOSE_WRITE | IN_ONESHOT);
    info.asset = assetName;
    notifyList.push_back(info);
}
#endif

	if (!data)
		return 0;

	/* create GL texture */
	if (!opengles2)
		GL_OPERATION(glEnable(GL_TEXTURE_2D))

	int powerOf2W = alignOnPowerOf2(w);
	int powerOf2H = alignOnPowerOf2(h);
	int border = 0;
	
	// hmm hmm: hacky stuff to add a border
	if (w != powerOf2W || h != powerOf2H) {
		border = 1;
		powerOf2W = alignOnPowerOf2(w + 4);
		powerOf2H = alignOnPowerOf2(h + 4);
		
		int stride1 = (w)*4*sizeof(char);
		int stride2 = (w+4)*4*sizeof(char);
		char* pdatas = (char*) malloc(stride2 * (h+4));
		memset(pdatas, 0, stride2 * (h+4));
		for (int i=2; i<(h+2); i++) {
			memcpy(&pdatas[i * stride2 + 4*sizeof(char)], &data[(i-2) * stride1], stride1);
		}
		free(data);
		data = pdatas;
	}

	GLuint texture;
	GL_OPERATION(glGenTextures(1, &texture))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, texture))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, powerOf2W,
                powerOf2H, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                NULL))
	GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w + 4*border,
                h + 4*border, GL_RGBA, GL_UNSIGNED_BYTE, data))
	free(data);
	
	realSize.X = w;
	realSize.Y = h;
	pow2Size.X = powerOf2W;
	pow2Size.Y = powerOf2H;
	
	return texture;
}

void RenderingSystem::reloadTextures() {
	Vector2 size, psize;
	
	// reload atlas texture
	for (int i=0; i<atlas.size(); i++) {
		atlas[i].texture = loadTexture(atlas[i].name, size, psize);
	}

	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		TextureInfo& info = textures[it->second];
		if (info.atlasIndex >= 0)
			info.glref = atlas[info.atlasIndex].texture;
		else {
			GLuint ref = loadTexture(it->first, size, psize);
			textures[it->second] = TextureInfo(ref, 0, 0, size.X, size.Y, false, psize);
		}
	}
}

void RenderingSystem::processDelayedTextureJobs() {
    // load textures
    for (std::set<std::string>::iterator it=delayedLoads.begin(); it != delayedLoads.end(); ++it) {
        Vector2 size, powSize;
        GLuint ref = loadTexture(*it, size, powSize);
        textures[assetTextures[*it]] = TextureInfo(ref, 1+1, 1+1, size.X-1, size.Y-1, false, powSize);
    }
    delayedLoads.clear();

    // delete textures
    for (std::set<TextureRef>::iterator it=delayedDeletes.begin(); it != delayedDeletes.end(); ++it) {
        GLuint r = textures[*it].glref;
        glDeleteTextures(1, &r);
    }
    delayedDeletes.clear();
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	TextureRef result = InvalidTextureRef;
	std::string name(assetName);
	if (assetName.find(".png") == -1) {
		name = name + ".png";
	}

	if (assetTextures.find(name) != assetTextures.end()) {
		result = assetTextures[name];
	} else {
		result = nextValidRef++;
		assetTextures[name] = result;
	}
	if (textures.find(result) == textures.end())
		delayedLoads.insert(name);
		
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
                break;
            }
        }
        delayedDeletes.insert(ref);
    } else {
        LOGW("Tried to delete an InvalidTextureRef");
    }
}

