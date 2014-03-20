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



#include "ResourceHotReload.h"
#include "base/Log.h"

#if SAC_LINUX && SAC_DESKTOP
#include <sys/inotify.h>
#include <fstream>
#include <unistd.h>
#include <stdio.h>

ResourceHotReload::InotifyDatas::InotifyDatas(int fd, const std::string & file, const std::string & asset)
    : _filename(file), _assetname(asset) {
    LOGV(1, "New asset to monitor: " << _assetname << " from file " << _filename);

    wd = inotify_add_watch(fd, _filename.c_str(), IN_CLOSE_WRITE);
}
#endif

ResourceHotReload::ResourceHotReload() {
#if SAC_LINUX && SAC_DESKTOP
    inotifyFd = inotify_init();

    if (inotifyFd == -1) {
        perror(0);
        LOGF("Inotify failure, stopping");
    }
#endif
}

void ResourceHotReload::updateReload() {
#if SAC_LINUX && SAC_DESKTOP

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(inotifyFd, &fds);
    struct timeval tv = (struct timeval){0,0};

    int toRead = select(inotifyFd + 1, &fds, NULL, NULL, &tv);

    LOGV_IF(1, toRead, toRead << " inotify events");
    char* buffer = (char*)alloca(8192);
    for (int i=0; i<toRead; i++) {
        if (read(inotifyFd, buffer, 8192) > 0) {
            struct inotify_event *event;
            event = (struct inotify_event *) buffer;

            // look up file
            for (auto& it: filenames) {
                InotifyDatas& idata = it.second;
                if (idata.wd == event->wd) {
                    LOGV(1, idata._filename << " has changed! Reloading." << event->wd);
                    idata.wd = inotify_add_watch(inotifyFd,
                        idata._filename.c_str(),
                        IN_CLOSE_WRITE);
                    reload(idata._assetname);
                }
            }

        } else {
            LOGW("Failed to read event #" << i << " from inotify");
        }
    }
#endif
}

#if SAC_LINUX && SAC_DESKTOP
void ResourceHotReload::registerNewAsset(const std::string & asset, const std::string & override) {
    std::string filename = asset;
    if (!override.empty())
        filename = override;
    std::string full = SAC_ASSETS_DIR + asset2File(filename);

    std::ifstream ifile(full);
    if (!ifile) {
        const std::string assetsDirectory = "assets/";
        size_t idx = full.find(assetsDirectory);
        if (idx == std::string::npos) {
            LOGW("File " << asset2File(filename) << " does not exist! Can't monitore it");
            return;
        }
        full.replace(idx, assetsDirectory.length(), "assetspc/");
        ifile.open(full, std::ifstream::in);
        if (!ifile) {
            LOGW("File " << asset2File(filename) << " does not exist! Can't monitore it");
            return;
        }
    }

    if (filenames.find(full) == filenames.end())
        filenames.insert(std::make_pair(full, InotifyDatas(inotifyFd, full, asset)));
}
#else
    void ResourceHotReload::registerNewAsset(const std::string &, const std::string &) {}
#endif

