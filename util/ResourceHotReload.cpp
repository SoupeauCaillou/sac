#include "ResourceHotReload.h"
#include "base/Log.h"

#if SAC_LINUX && SAC_DESKTOP
#include <sys/inotify.h>
#include <list>
#include <fstream>
#include <unistd.h>

ResourceHotReload::InotifyDatas::InotifyDatas(const std::string & file, const std::string & asset)
	: _filename(file), _assetname(asset) {
	LOGI("New asset to monitor: " << _assetname << " from file " << _filename)

	inotifyFd = inotify_init();
	wd = inotify_add_watch(inotifyFd, _filename.c_str(), IN_CLOSE_WRITE);
}
#endif

void ResourceHotReload::updateReload() {
#if SAC_LINUX && SAC_DESKTOP
    for (auto it : filenames) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET((it).second.inotifyFd, &fds);
        struct timeval tv = (struct timeval){0,0};
        if (select((it).second.inotifyFd + 1, &fds, NULL, NULL, &tv) > 0) {
            char buffer[8192];
            struct inotify_event *event;

            if (read((it).second.inotifyFd, buffer, sizeof(buffer)) > 0) {
                event = (struct inotify_event *) buffer;
                //it has changed! reload it
                if (event->wd == (it).second.wd) {
                    LOGI((it).second._filename << " has changed! Reloading.");
                    (it).second.wd = inotify_add_watch((it).second.inotifyFd, (it).second._filename.c_str(), IN_CLOSE_WRITE);
                    reload((it).second._assetname);
                }

            }
        }
    }
#endif
}

void ResourceHotReload::registerNewAsset(const std::string & name) {
#if SAC_LINUX && SAC_DESKTOP
    std::string full = SAC_ASSETS_DIR + asset2File(name);

    std::ifstream ifile(full);
    if (!ifile) {
        const std::string assetsDirectory = "assets/";
        size_t idx = full.find(assetsDirectory);
        if (idx == std::string::npos) {
            LOGW("File " << asset2File(name) << " does not exist! Can't monitore it")
            return;
        }
        full.replace(idx, assetsDirectory.length(), "assetspc/");
        ifile.open(full, std::ifstream::in);
        if (!ifile) {
            LOGW("File " << asset2File(name) << " does not exist! Can't monitore it")
            return;
        }
    }

    if (filenames.find(full) == filenames.end())
    	filenames.insert(std::make_pair(name, InotifyDatas(full, name)));
#endif
}
