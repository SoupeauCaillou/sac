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



#include "AssetAPILinuxImpl.h"

#include "base/Log.h"

#include <cstring>

#include <sstream>

#include <sys/stat.h>
#include <cstdlib>
#include <algorithm>

#if SAC_EMSCRIPTEN
#include <emscripten.h>
#endif

#if SAC_WINDOWS
#include <api/windows/dirent.h>
#include <shlobj.h>
#include <Shobjidl.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#if SAC_IOS
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <string.h>
CFBundleRef mainBundle;
#endif

//Returns -1 if the path does not exist, 1 if path is a directory, 2 if its a file, 0 otherwise
static int getPathType(const char* path) {
    struct stat s_buf;
    
    if (stat(path, &s_buf)) {
        LOGW("path does not exist: " << path);
        return -1;
    }
    
    if (s_buf.st_mode & S_IFDIR) {
        return 1;
    } else if (s_buf.st_mode & S_IFREG) {
        return 2;
    }
    return 0;
}

void AssetAPILinuxImpl::init(const std::string & gName) {
    gameName = gName;
#if SAC_IOS
    mainBundle = CFBundleGetMainBundle();
#endif
}

FileBuffer AssetAPILinuxImpl::loadFile(const std::string& full) {
    FileBuffer fb;
    fb.data = 0;
    FILE* file = fopen(full.c_str(), "rb");
    
    if (! file) {
        /* special case for images */
        if (full.size() > 4) {
            const char* ext = full.c_str() + full.size() - 4;
            if (
                strncmp(ext, ".dds", 4) == 0 ||
                strncmp(ext, ".pvr", 4) == 0 ||
                strncmp(ext, ".pkm", 4) == 0) {
                
                int count = 0;
                FileBuffer partial;
                char* name = (char*)alloca(full.size() + 10);
                do {
                    sprintf(name, "%s.%02d", full.c_str(), count);
                    partial = loadFile(name);
                    
                    if (partial.size > 0) {
                        fb.data = (uint8_t*)realloc(fb.data, fb.size + partial.size);
                        memcpy(&fb.data[fb.size], partial.data, partial.size);
                        fb.size += partial.size;
                    }
                    count++;
                } while (partial.size);
            }
            
        }
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

#if SAC_IOS
static char* assetPath = NULL;
#endif
FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
#if SAC_IOS
    if (!assetPath) {
        CFURLRef url = CFBundleCopyResourcesDirectoryURL(mainBundle);
        UInt8 tmp[1024];
        if (!CFURLGetFileSystemRepresentation(url,
                                              true,
                                              tmp,
                                              sizeof(tmp))) {
            LOGE("Failed to convert CFURLRef");
            tmp[0] = 0;
        } else {
            strcat((char*)tmp, "/assets/");
            assetPath = strdup((char*)tmp);
        }
        CFRelease(url);
    }
    std::string full = assetPath + asset;
#else
#ifdef SAC_ASSETS_DIR
    std::string full = SAC_ASSETS_DIR + asset;
#else
    std::string full = "assets/" + asset;
#endif
#endif
    fb = loadFile(full);
#if SAC_DESKTOP
    if (fb.data == 0) {
        // try in pc specific folder
        full.replace(full.find("assets/"), strlen("assets/"), "assetspc/");
        fb = loadFile(full);
        
        if (fb.data == 0) {
            LOGE("Error opening file '" << asset << "'");
        }
    }
#endif
    return fb;
}

std::list<std::string> AssetAPILinuxImpl::listContent(const std::string& directory, const std::string& extension, const std::string& subfolder) {
    std::list<std::string> content;
    std::string realDirectory = directory +  "/" + subfolder + "/";
    size_t pos;
    while ((pos = realDirectory.find("//")) != std::string::npos)
        realDirectory.replace(pos, 2, "/");
    
    DIR* dir = opendir(realDirectory.c_str());
    if (dir == NULL)
        return content;
    dirent* file;
    while ( (file = readdir(dir)) != NULL) {
        // Check if file is a directory
        if (1 == getPathType((realDirectory + file->d_name).c_str())) {
            // Check if file is not current dir (.) or its parent (..)
            if (std::strcmp (file->d_name, "..") != 0 &&
                std::strcmp (file->d_name, ".") != 0) {
                std::list<std::string> tmp;
                tmp = listContent(realDirectory, extension, subfolder + '/' + file->d_name);
                for (auto &i: tmp) {
                    content.push_back(std::string(file->d_name) + '/' + i);
                }
            }
            //otherwise it might be a file
#if SAC_EMSCRIPTEN
        } else if (file->d_type == 8) {
#else
        } else if (2 == getPathType((realDirectory + file->d_name).c_str())) {
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
    return content;
}

std::list<std::string> AssetAPILinuxImpl::listAssetContent(const std::string& extension, const std::string& subfolder) {
#ifdef SAC_ASSETS_DIR
    std::string directory = SAC_ASSETS_DIR;
#elif SAC_IOS
    CFURLRef url = CFBundleCopyResourcesDirectoryURL(mainBundle);
    UInt8 tmp[1024];
    if (!CFURLGetFileSystemRepresentation(url,
                                          true,
                                          tmp,
                                          sizeof(tmp))) {
        LOGE("Failed to convert CFURLRef");
        tmp[0] = 0;
    }
    std::string directory((char*) tmp);
    directory = directory + "/assets/";
    
    CFRelease(url);
#else
    std::string directory = "assets/";
#endif
    return listContent(directory, extension, subfolder);
}

#if SAC_IOS
// set by Obj-C code
char* iosWritablePath = NULL;
#endif

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
        //add game name to the path
        ss << gameName;
#elif SAC_EMSCRIPTEN
        ss << "/sac_temp/";
        //add game name to the path
        ss << gameName;
#elif SAC_WINDOWS
        ss << getenv("APPDATA") << "/";
        //add game name to the path
        ss << gameName;
#elif SAC_IOS
        LOGF_IF(!iosWritablePath, "iosWritablePath not set. Must be done in Obj-C before using AssetAPI");
        ss << iosWritablePath;
#else
        return "not-handled-os";
#endif
        
        
        //update path
        path = ss.str().c_str();
        
        // remove non alphanum characters. Problem yet: it erase '/' too...
        // path.erase(std::remove_if(path.begin(), path.end(),
        // std::not1(std::ptr_fun((int(*)(int))std::isalnum))), path.end());
        
        // create folder if needed
        int permission = 0;
#if ! SAC_WINDOWS
        permission = S_IRWXU | S_IWGRP | S_IROTH;
#endif
        if (! doesExistFileOrDirectory(path.c_str())) {
            createDirectory(path.c_str(), permission);
        }
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

void AssetAPILinuxImpl::createDirectory(const std::string& fullpath, int permission) {
    struct stat st;
    
    if (stat(fullpath.c_str(), &st) == -1) {
        mkdir(fullpath.c_str(), permission);
    } else {
        LOGW("Couldn't create directory '" << fullpath << "' since it already exist!");
    }
}

bool AssetAPILinuxImpl::doesExistFileOrDirectory(const std::string& fullpath) {
    DIR* dp = opendir(fullpath.c_str());
    if (dp != 0) closedir(dp);
    
    return (dp != 0) || (access(fullpath.c_str(), F_OK) != -1);
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
        if (1 == getPathType(subfolder.c_str())) {
            removeFileOrDirectory(subfolder.c_str());
        } else {
            unlink(subfolder.c_str());
        }
    }
    closedir(dp);
    rmdir(fullpath.c_str());
    
}

