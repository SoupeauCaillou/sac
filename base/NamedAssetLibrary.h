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

#include <glog/logging.h>
#include <map>
#include <string>
#include <set>

#include <mutex>
#include <condition_variable>

#if !defined(ANDROID)
#define USE_COND_SIGNALING 1
#else
#undef USE_COND_SIGNALING
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
                VLOG(1) << "Put asset '" << name << "' on delayed load queue. Ref value: " << result;
                nameToRef.insert(std::make_pair(name, result));
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
            LOG(WARNING) << "TODO";
        }

        void update() {
            VLOG_IF(1, !delayed.loads.empty()) << "Process delayed loads";
            for (std::set<std::string>::iterator it=delayed.loads.begin();
                it!=delayed.loads.end();
                ++it) {
                mutex.lock();
                T asset;
                TRef ref = nameToRef[*it];
                VLOG(2) << "\tLoad '" << *it << "' -> " << ref;
                doLoad(*it, asset, ref);
                ref2asset.insert(std::make_pair(ref, asset));
                mutex.unlock();
            }

            VLOG_IF(1, !delayed.unloads.empty()) << "Process delayed unloads";
            for (std::set<std::string>::iterator it=delayed.unloads.begin();
                it!=delayed.unloads.end();
                ++it) {
                mutex.lock();
                const std::string& name = *it;
                VLOG(2) << "\tUnload '" << name << "'";
                TRef ref = nameToRef[name];
                doUnload(name, ref2asset[ref]);
                ref2asset.erase(ref);
                nameToRef.erase(name);
                mutex.unlock();
            }

            VLOG_IF(1, !delayed.unloads.empty()) << "Process delayed reloads";
            for (std::set<std::string>::iterator it=delayed.reloads.begin();
                it!=delayed.reloads.end();
                ++it) {
                mutex.lock();
                const std::string& name = *it;
                VLOG(2) << "Reload '" << name << "'";
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

            LOG_IF(FATAL, it == ref2asset.end()) << "Unkown ref requested: " << ref << ". Asset count: " << ref2asset.size();
            lock.unlock();
            return &(it->second);
        }

        const T& get(const std::string& name) {
            typename std::map<std::string, TRef>::const_iterator it = nameToRef.find(name);
            LOG_IF(FATAL, it == nameToRef.end()) << "Unkown asset requested: " << name << ". Asset count: " << nameToRef.size();
            return *get(it->second, true);
        }

        void registerDataSource(TRef r, SourceDataType type) {
            if (dataSource.find(r) != dataSource.end())
                LOG(WARNING) << "Asset " << r << " already have one data source registered";
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
};