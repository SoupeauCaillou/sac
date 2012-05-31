#include "AssetAPILinuxImpl.h"

void AssetAPILinuxImpl::init() {
	
}
#include <iostream>
FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
    std::string full = "assets/" + asset;
    FILE* file = fopen(full.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    fb.size = ftell(file);
    rewind(file);
    fb.data = new uint8_t[fb.size];
    int count = 0;
do {
count += fread(&fb.data[count], 1, fb.size - count, file);
} while (count < fb.size);

    fclose(file);
    std::cout << asset << ":" << fb.size  << "/" << count << std::endl;
    return fb;
}
