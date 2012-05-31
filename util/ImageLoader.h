#pragma once

#include <string>
#include "../api/AssetAPI.h"

struct ImageDesc {
	char* datas;
	int width, height;
	enum {
		RGBA, // 32 bpp
		RGB,  // 24 bpp
		ALPHA //  8 bpp
	} format;
	enum {
		RAW,
		ECT1
	} type;
};

class ImageLoader {
	public:
		static ImageDesc loadPng(const std::string& context, const FileBuffer& file);
		
		static ImageDesc loadEct1(const std::string& context, const FileBuffer& file);
};
