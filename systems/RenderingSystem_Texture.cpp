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

void RenderingSystem::loadAtlas(const std::string& atlasName) {
	std::string atlasDesc = atlasName + ".desc";
	std::string atlasImage = atlasName + ".png";
	
	const char* desc = assetLoader->loadShaderFile(atlasDesc);
	if (!desc) {
		LOGW("Unable to load atlas desc %s", atlasDesc.c_str());
		return;
	}
	
	Vector2 atlasSize, pow2Size;
	Atlas a;
	a.name = atlasImage;
	a.glref = 0;
	atlas.push_back(a);
	int atlasIndex = atlas.size() - 1;

	std::stringstream f(std::string(desc), std::ios_base::in);
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
	delete[] desc;
	LOGI("Atlas '%s' loaded %d images", atlasName.c_str(), count);
}

void RenderingSystem::invalidateAtlasTextures() {
    for (unsigned int i=0; i<atlas.size(); i++) {
        atlas[i].glref = 0;
    }
}

void RenderingSystem::unloadAtlas(const std::string& atlasName) {
    for (unsigned int i=0; i<atlas.size(); i++) {
        if (atlasName == atlas[i].name) {
            for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end();) {
                std::map<TextureRef, TextureInfo>::iterator next = ++it;
                if (it->second.glref == atlas[i].glref) {
                    textures.erase(it);
                }
                it = next;
            }
            delayedDeletes.insert(atlas[i].glref);
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

void RenderingSystem::loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size, InternalTexture& out) {
	int w,h;
    // LOGW("loadTexture: %s", assetName.c_str());
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
		return;

	/* create GL texture */
#ifdef GLES2_SUPPORT
	if (!opengles2)
#endif
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

	GL_OPERATION(glGenTextures(1, &out.color))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, out.color))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, powerOf2W,
                powerOf2H, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                NULL))
	GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w + 4*border,
                h + 4*border, GL_RGBA, GL_UNSIGNED_BYTE, data))

	GL_OPERATION(glGenTextures(1, &out.alpha))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, out.alpha))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, powerOf2W,
                powerOf2H, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                NULL))
	GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w + 4*border,
                h + 4*border, GL_RGBA, GL_UNSIGNED_BYTE, data))

	free(data);
	
	realSize.X = w;
	realSize.Y = h;
	pow2Size.X = powerOf2W;
	pow2Size.Y = powerOf2H;
}

void RenderingSystem::reloadTextures() {
	Vector2 size, psize;
	LOGW("Reloading textures begin");
    LOGW("\t- atlas : %lu", atlas.size());
	// reload atlas texture
	for (unsigned int i=0; i<atlas.size(); i++) {
		atlas[i].glref = 0;
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
	    if (it->alpha > 0) {
	    	LOGW("Alpha texture delete: %u", it->alpha);
        	glDeleteTextures(1, &it->alpha);
	    }
    }
    delayedDeletes.clear();
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	TextureRef result = InvalidTextureRef;
	std::string name(assetName);
	if (assetName.find(".png") == std::string::npos) {
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
                delayedDeletes.insert(textures[ref].glref);
                break;
            }
        }
    } else {
        LOGW("Tried to delete an InvalidTextureRef");
    }
}

