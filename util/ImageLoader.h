#pragma once

#include <string>
#include "../api/AssetAPI.h"

struct ImageDesc {
	char* datas;
	int width, height;
    int channels, mipmap;
	enum {
		RAW,
		ECT1,
		PVR
	} type;
};

class ImageLoader {
	public:
		static ImageDesc loadPng(const std::string& context, const FileBuffer& file);

		static ImageDesc loadEct1(const std::string& context, const FileBuffer& file);

		static ImageDesc loadPvr(const std::string& context, const FileBuffer& file);
};
