#include "RenderingSystem.h"
#include <GLES/gl.h>
#include "base/EntityManager.h"
#include <cmath>
#include <cassert>
#include <sstream>

RenderingSystem::TextureInfo::TextureInfo (GLuint r, int x, int y, int w, int h, bool rot, const Vector2& size,  int atlasIdx) {
	glref = r;		
	if (size == Vector2::Zero) {
		uv[0].X = uv[0].Y = 0;
		uv[1].X = uv[1].Y = 1;
		rotateUV = 0;
	} else {
		float blX = x / size.X;
		float trX = (x+w) / size.X;
		float blY = 1 - (y+h) / size.Y;
		float trY = 1 - y / size.Y;

		uv[0].X = blX;
		uv[1].X = trX;
		uv[0].Y = blY;
		uv[1].Y = trY;
		rotateUV = rot;
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
	
	int w, h;
	GLuint glref = loadTexture(atlasImage, w,h );
	Atlas a;
	a.name = atlasImage;
	a.texture = glref;
	atlas.push_back(a);
	int atlasIndex = atlas.size() - 1;
	Vector2 atlasSize(w, h);
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
	LOGI("Atlas '%s' loaded %d images", atlasName.c_str(), count);
}

static unsigned int alignOnPowerOf2(unsigned int value) {
	for(int i=0; i<32; i++) {
		unsigned int c = 1 << i;
		if (value <= c)
			return c;
	}
	return 0;
}

GLuint RenderingSystem::loadTexture(const std::string& assetName, int& w, int& h) {
	char* data = assetLoader->decompressPngImage(assetName, &w, &h);

	if (!data)
		return 0;

	/* create GL texture */
	if (!opengles2)
		GL_OPERATION(glEnable(GL_TEXTURE_2D))

	int powerOf2W = alignOnPowerOf2(w);
	int powerOf2H = alignOnPowerOf2(h);

	GLuint texture;
	GL_OPERATION(glGenTextures(1, &texture))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, texture))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, powerOf2W,
                powerOf2H, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                NULL))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,
                h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data))
	free(data);
	return texture;
}

void RenderingSystem::reloadTextures() {
	int w, h;
	
	// reload atlas texture
	for (int i=0; i<atlas.size(); i++) {
		atlas[i].texture = loadTexture(atlas[i].name, w, h);
	}
	// todo: atlas handling
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		TextureInfo& info = textures[it->second];
		if (info.atlasIndex >= 0)
			info.glref = atlas[info.atlasIndex].texture;
		else
			textures[it->second] = loadTexture(it->first, w, h);
	}
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

