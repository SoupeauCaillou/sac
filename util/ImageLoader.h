/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include <string>
struct FileBuffer;

struct ImageDesc {
    union {
	    char* datas;
        unsigned char* udatas;
    };
	int width, height;
    int channels, mipmap;
	enum {
		RAW,
		ETC1,
		PVR,
        S3TC
	} type;
};

class ImageLoader {
	public:
		static ImageDesc loadPng(const std::string& context, const FileBuffer& file);

		static ImageDesc loadEtc1(const std::string& context, const FileBuffer& file, bool etc1supported);

		static ImageDesc loadPvr(const std::string& context, const FileBuffer& file);

        static ImageDesc loadDDS(const std::string& context, const FileBuffer& file);
};
