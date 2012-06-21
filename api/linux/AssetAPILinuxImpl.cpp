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
#include "AssetAPILinuxImpl.h"

void AssetAPILinuxImpl::init() {
	
}
#include <iostream>
FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
    fb.data = 0;
#ifdef DATADIR
	std::string full = DATADIR + asset;
#else
    std::string full = "assets/" + asset;
#endif
    FILE* file = fopen(full.c_str(), "rb");
    if (!file)
        return fb;
    fseek(file, 0, SEEK_END);
    fb.size = ftell(file);
    rewind(file);
    fb.data = new uint8_t[fb.size + 1];
    int count = 0;
do {
count += fread(&fb.data[count], 1, fb.size - count, file);
} while (count < fb.size);

    fclose(file);
    fb.data[fb.size] = 0;
    std::cout << asset << ":" << fb.size  << "/" << count << std::endl;
    return fb;
}
