#include "AssetAPILinuxImpl.h"

#include "base/Log.h"

#include <cstring>

#include <sstream>

#include <sys/stat.h>
//#include <sys/types.h>

//for getenv
#include <cstdlib>

//for rmdir
#include <unistd.h>

#if SAC_EMSCRIPTEN
#include <emscripten.h>
#endif

#if SAC_WINDOWS

#else
#include <dirent.h>
#endif

void AssetAPILinuxImpl::init() {

}

FileBuffer AssetAPILinuxImpl::loadFile(const std::string& full) {
    FileBuffer fb;
    fb.data = 0;
    FILE* file = fopen(full.c_str(), "rb");

    if (! file) {
        return fb;
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

FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
#ifdef SAC_ASSETS_DIR
	std::string full = SAC_ASSETS_DIR + asset;
#else
    std::string full = "assets/" + asset;
#endif
    fb = loadFile(full);
    if (fb.data == 0) {
        // try in pc specific folder
        full.replace(full.find("assets/"), strlen("assets/"), "assetspc/");
        fb = loadFile(full);

        if (fb.data == 0) {
            LOGE("Error opening file '" << asset << "'");
        }
    }
    return fb;
}

std::list<std::string> AssetAPILinuxImpl::listContent(const std::string& directory, const std::string& extension, const std::string& subfolder) {
    std::list<std::string> content;
    const std::string realDirectory = directory +  "/" + subfolder;
#if SAC_WINDOWS
        // TODO
#else
        // TODO : Use scandir ?
        DIR* dir = opendir(realDirectory.c_str());
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
                    tmp = listContent(realDirectory, extension, subfolder + '/' + file->d_name);
                    for (auto i: tmp) {
                        content.push_back(std::string(file->d_name) + '/' + i);
                    }
                }
#if SAC_EMSCRIPTEN
            } else if (file->d_type == 8) {
#else
            } else if (file->d_type == DT_REG) {
#endif
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

std::list<std::string> AssetAPILinuxImpl::listAssetContent(const std::string& extension, const std::string& subfolder) {
#ifdef SAC_ASSETS_DIR
        std::string directory = SAC_ASSETS_DIR;
#else
        std::string directory = "assets/";
#endif
        return listContent(directory, extension, subfolder);
}

const std::string & AssetAPILinuxImpl::getWritableAppDatasPath() {
    static std::string path;

    if (path.empty()) {
        std::stringstream ss;
#if SAC_LINUX
        char * pPath = getenv ("XDG_DATA_HOME");
        if (pPath) {
            ss << pPath;
        } else if ((pPath = getenv ("HOME")) != 0) {
            ss << pPath << "/.local/share/";
        } else {
            ss << "/tmp/";
        }
        ss << "sac/";

        // create folder if needed
        struct stat statFolder;
        int st = stat(ss.str().c_str(), &statFolder);
        if (st || (statFolder.st_mode & S_IFMT) != S_IFDIR) {
            if (mkdir(ss.str().c_str(), S_IRWXU | S_IWGRP | S_IROTH)) {
                LOGE("Failed to create : '" << ss.str() << "'");
            }
        }

#elif SAC_EMSCRIPTEN
        ss << "/sac_temp/";
#else
        ss << "not-handled-os";
#endif

        path = ss.str();
    }
    return path;
 }

void AssetAPILinuxImpl::synchronize() {
#if SAC_EMSCRIPTEN
    const char* script = "" \
        "localStorage[\"sac_root\"] = window.JSON.stringify(FS.root.contents['sac_temp']);" \
        "localStorage[\"sac_nextInode\"] = window.JSON.stringify(FS.nextInode);";
    emscripten_run_script(script);
#endif
}

static int isPathADirectory (const char* path) {
    struct stat s_buf;

    if (stat(path, &s_buf))
        return 0;

    return S_ISDIR(s_buf.st_mode);
}

void AssetAPILinuxImpl::createDirectory(const std::string& fullpath) {
    struct stat st;

    if (stat(fullpath.c_str(), &st) == -1) {
        mkdir(fullpath.c_str(), 0700);
    } else {
        LOGE("Couldn't create directory '" << fullpath << "' since it already exist!");
    }
}

bool AssetAPILinuxImpl::doesExistFileOrDirectory(const std::string& fullpath) {
    DIR* dp;

    dp = opendir(fullpath.c_str());
    if (dp != 0) closedir(dp);

    return dp != 0;
}

void AssetAPILinuxImpl::removeFileOrDirectory(const std::string& fullpath) {
    DIR*            dp;
    struct dirent*  ep;

    std::string subfolder;

    dp = opendir(fullpath.c_str());

    if (dp == 0) {
        LOGE("Couldn't open fileOrDirectory at path " << fullpath << ". Does it exist?" );
        return;
    }
    while ((ep = readdir(dp)) != 0) {
        if (std::strcmp (ep->d_name, "..") == 0 ||
            std::strcmp (ep->d_name, ".") == 0) {
            continue;
        }
        subfolder = fullpath + "/" + ep->d_name;
        if (isPathADirectory(subfolder.c_str())) {
            removeFileOrDirectory(subfolder.c_str());
        } else {
            unlink(subfolder.c_str());
        }
    }
    closedir(dp);
    rmdir(fullpath.c_str());
}




