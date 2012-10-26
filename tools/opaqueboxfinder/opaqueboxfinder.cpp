#include <iostream>
#include <stdio.h>
#include "util/ImageLoader.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: opaqueboxfinder image" << std::endl;
		return -1;
	}
	FileBuffer file;
	
	FILE* fd = fopen(argv[1], "rb");
    if (!fd) {
    	std::cerr << "Unable to open '" << argv[1] << "'" << std::endl;
        return -1;
    }
    fseek(fd, 0, SEEK_END);
    file.size = ftell(fd);
    rewind(fd);
    file.data = new uint8_t[file.size + 1];
    int count = 0;
    do {
		count += fread(&file.data[count], 1, file.size - count, fd);
	} while (count < file.size);
    fclose(fd);
    file.data[file.size] = 0;
    
	ImageDesc image = ImageLoader::loadPng(argv[1], file);
	delete[] file.data;

	std::cout << image.width << " x " << image.height << " @ " << image.channels << std::endl;


	delete[] image.datas;
	return 0;
}
