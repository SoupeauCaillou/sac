/*
    This file is part of sac.

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
#pragma once

#include <map>
#include <string>
#include <set>

#include <mutex>
#include <condition_variable>
#include "Log.h"

#if defined(SAC_ANDROID)
// #undef USE_COND_SIGNALING
#define USE_COND_SIGNALING 1
#else
#define USE_COND_SIGNALING 1
#endif

#if defined(SAC_LINUX) & defined(SAC_DESKTOP)
#include <sys/inotify.h>
#include <list>
#include <fstream>
#include <unistd.h>
#endif


class AssetAPI;

template <typename T, typename TRef, typename SourceDataType>
class NamedAssetLibrary {
    #define InvalidRef -1
    public:
        NamedAssetLibrary() : nextValidRef(1), assetAPI(0) {
        }

        virtual void init(AssetAPI* pAssetAPI) {
            assetAPI = pAssetAPI;
        }

        virtual ~NamedAssetLibrary() {
        }

        TRef load(const std::string& name) {
            TRef result = InvalidRef;
            mutex.lock();
            typename std::map<std::string, TRef>::iterator it = nameToRef.find(name);
            if (it == nameToRef.end()) {
                delayed.loads.insert(name);
                result = nextValidRef++;
                LOGV(1, "Put asset '" << name << "' on delayed load queue. Ref value: " << result)
                nameToRef.insert(std::make_pair(name, result));
                #if defined(SAC_LINUX) & defined(SAC_DESKTOP)
                registerNewAsset(name);
                #endif
            } else {
                result = it->second;
            }
            mutex.unlock();
            return result;
        }

        void unload(const std::string& name) {
            mutex.lock();
            delayed.unloads.insert(name);
            mutex.unlock();
        }
        void reload(const std::string& name) {
            mutex.lock();
            delayed.reloads.insert(name);
            mutex.unlock();
        }

        void unload(const TRef& ref) {
            mutex.lock();
            for (typename std::map<std::string, TRef>::iterator it=nameToRef.begin(); it!=nameToRef.end(); ++it) {
                if (it->second == ref) {
                    delayed.unloads.insert(it->first);
                    break;
                }
            }
            mutex.unlock();
        }

        void reloadAll() {
            LOGW("TODO")
        }

        void update() {
            LOGV_IF(1, !delayed.loads.empty(), "Process delayed loads")
            for (std::set<std::string>::iterator it=delayed.loads.begin();
                it!=delayed.loads.end();
                ++it) {
                mutex.lock();
                T asset;
                TRef ref = nameToRef[*it];
                LOGV(2, "\tLoad '" << *it << "' -> " << ref)
                doLoad(*it, asset, ref);
                ref2asset.insert(std::make_pair(ref, asset));
                mutex.unlock();
            }

            LOGV_IF(1, !delayed.unloads.empty(), "Process delayed unloads")
            for (std::set<std::string>::iterator it=delayed.unloads.begin();
                it!=delayed.unloads.end();
                ++it) {
                mutex.lock();
                const std::string& name = *it;
                LOGV(2, "\tUnload '" << name << "'")
                TRef ref = nameToRef[name];
                doUnload(name, ref2asset[ref]);
                ref2asset.erase(ref);
                nameToRef.erase(name);
                mutex.unlock();
            }

            LOGV_IF(1, !delayed.unloads.empty(), "Process delayed reloads")
            for (std::set<std::string>::iterator it=delayed.reloads.begin();
                it!=delayed.reloads.end();
                ++it) {
                mutex.lock();
                const std::string& name = *it;
                LOGV(2, "Reload '" << name << "'")
                TRef ref = nameToRef[name];
                doReload(name, ref);
                mutex.unlock();
            }
            mutex.lock();
            delayed.loads.clear();
            delayed.unloads.clear();
            delayed.reloads.clear();
            #if USE_COND_SIGNALING
            cond.notify_all();
            #endif
            mutex.unlock();
        }

        const T* get(const TRef& ref, bool waitIfLoadingInProgress) {
            std::unique_lock<std::mutex> lock(mutex);
            typename std::map<TRef, T>::const_iterator it = ref2asset.find(ref);
            if (it == ref2asset.end()) {
                #if USE_COND_SIGNALING
                if (waitIfLoadingInProgress) {
                    // wait for next load end, the requested resource might be loaded in the next batch
                    while (!delayed.loads.empty())
                        cond.wait(lock);
                    it = ref2asset.find(ref);
                } else {
                #endif
                    lock.unlock();
                    return 0;
                }
            }

            LOGF_IF(it == ref2asset.end(), "Unkown ref requested: " << ref << ". Asset count: " << ref2asset.size())
            lock.unlock();
            return &(it->second);
        }

        const T& get(const std::string& name) {
            typename std::map<std::string, TRef>::const_iterator it = nameToRef.find(name);
            LOGF_IF(it == nameToRef.end(), "Unkown asset requested: " << name << ". Asset count: " << nameToRef.size())
            return *get(it->second, true);
        }

        void registerDataSource(TRef r, SourceDataType type) {
            if (dataSource.find(r) != dataSource.end())
                LOGW("Asset " << r << " already have one data source registered")
            dataSource.insert(std::make_pair(r, type));
        }


    protected:
        void setNextValidRef(TRef r) {
            nextValidRef = r;
        }

    protected:
        virtual bool doLoad(const std::string& name, T& out, const TRef& ref) = 0;
        virtual void doUnload(const std::string& name, const T& in) = 0;
        virtual void doReload(const std::string& name, const TRef& ref) = 0;

    protected:
        std::mutex mutex;
        #if USE_COND_SIGNALING
        std::condition_variable cond;
        #endif
        TRef nextValidRef;
        AssetAPI* assetAPI;

        std::map<std::string, TRef> nameToRef;
        std::map<TRef, T> ref2asset;
        std::map<TRef, SourceDataType> dataSource;
        struct {
            std::set<std::string> loads;
            std::set<std::string> unloads;
            std::set<std::string> reloads;
        } delayed;

    #if defined(SAC_LINUX) & defined(SAC_DESKTOP)
    public:
        void updateInotify() {
            for (auto it : filenames) {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(it.inotifyFd, &fds);
                struct timeval tv = (struct timeval){0,0};
                if (select(it.inotifyFd + 1, &fds, NULL, NULL, &tv) > 0) {
                    char buffer[8192];
                    struct inotify_event *event;

                    if (read(it.inotifyFd, buffer, sizeof(buffer)) > 0) {
                        event = (struct inotify_event *) buffer;
                        //it has changed! reload it
                        if (event->wd == it.wd) {
                            LOGI(it._filename << " has been changed! Reloading.");
                            it.wd = inotify_add_watch(it.inotifyFd, it._filename.c_str(), IN_MODIFY);
                            load(it._assetname);
                        }

                    }
                }
            }
        }
        void registerNewAsset(const std::string & name) {
            #ifdef SAC_ASSETS_DIR
                std::string full = SAC_ASSETS_DIR + name;
            #else
                std::string full = "assets/" + name;
            #endif
            std::ifstream ifile(full);
            if (!ifile) {
                const std::string assetsDirectory = "assets/";
                full.replace(full.find(assetsDirectory), assetsDirectory.length(), "assetspc/");
                ifile.open(full, std::ifstream::in);
                if (!ifile) {
                    LOGW("File " << full << " does not exist! Can't monitore it.");
                    return;
                }
            }
            filenames.push_front(InotifyDatas(full, name));
        }
    private:
        //for inotify
        struct InotifyDatas {
            int wd;
            int inotifyFd;
            std::string _filename;
            std::string _assetname;

            InotifyDatas(const std::string & file, const std::string & asset)
             : _filename(file), _assetname(asset) {
                LOGI("New asset to monitor: " << _assetname << " from file "
                    << _filename);
                inotifyFd = inotify_init();
                wd = inotify_add_watch(inotifyFd, _filename.c_str(), IN_MODIFY);
            }
        };

        std::list<InotifyDatas> filenames;


    #endif
};
