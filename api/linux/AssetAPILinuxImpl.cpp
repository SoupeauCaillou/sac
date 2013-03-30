    #include "AssetAPILinuxImpl.h"
#include <cstring>
#include <base/Log.h>

#ifdef SAC_WINDOWS

#else
    #include <dirent.h>
#endif

void AssetAPILinuxImpl::init() {

}

FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
    fb.data = 0;
#ifdef SAC_ASSETS_DIR
	std::string full = SAC_ASSETS_DIR + asset;
#else
    std::string full = "assets/" + asset;
#endif
    FILE* file = fopen(full.c_str(), "rb");
    if (!file) {
        // try in pc specific folder
        full.replace(full.find("assets/"), strlen("assets/"), "assetspc/");
        file = fopen(full.c_str(), "rb");
        if (!file) {
            LOGE("Error opening file '" << full << "'");
            return fb;
        }
    }
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
    return fb;
}

std::list<std::string> AssetAPILinuxImpl::listContent(const std::string& extension, const std::string& subfolder) {
    #ifdef SAC_ASSETS_DIR
        std::string directory = SAC_ASSETS_DIR + subfolder;
    #else
        std::string directory = "assets/" + subfolder;
    #endif

    std::list<std::string> content;
    #ifdef SAC_WINDOWS
        // TODO
    #else
        // TODO : Use scandir ?
        DIR* dir = opendir(directory.c_str());
        if (dir == NULL)
            return content;
        dirent* file;
        while ( (file = readdir(dir)) != NULL) {
            // Check if file is a directory
            if (file->d_type == DT_DIR) {
                // Check if file is not current dir (.) or its parent (..)
                if (std::strcmp (file->d_name, "..") != 0 &&
                    std::strcmp (file->d_name, ".") != 0) {
                    std::list<std::string> tmp;
                    tmp = listContent(extension, subfolder + '/' + file->d_name);
                    for (auto i: tmp) {
                        content.push_back(std::string(file->d_name) + '/' + i);
                    }
                }
            } else if (file->d_type == DT_REG) {
                std::string s = file->d_name;
                size_t pos;
                // We're looking for file with good extension
                 if ((pos = s.find(extension)) != std::string::npos) {
                    content.push_back(s.substr(0, pos));
                }
            }
        }
        closedir(dir);
    #endif
    return content;
}
