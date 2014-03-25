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



#pragma once

#include <map>
#include <string>
#include <set>
#include <vector>

#include <mutex>
#include <condition_variable>
#include "Log.h"
#include "util/ResourceHotReload.h"
#include "util/MurmurHash.h"

#if SAC_ANDROID
// #undef USE_COND_SIGNALING
#define USE_COND_SIGNALING 1
#else
#define USE_COND_SIGNALING 1
#endif

class AssetAPI;

template <typename T, typename TRef, typename SourceDataType>
class NamedAssetLibrary : public ResourceHotReload {
#define InvalidRef 0
    public:
        NamedAssetLibrary() : assetAPI(0), useDeferredLoading(true) {
        }

        virtual void init(AssetAPI* pAssetAPI, bool pUseDeferredLoading = true) {
            assetAPI = pAssetAPI;
            useDeferredLoading = pUseDeferredLoading;
            assets.reserve(8);
        }

        virtual ~NamedAssetLibrary() {
            nameToRef.clear();
            ref2index.clear();
        }

        TRef name2ref(const std::string& name) const {
            return Murmur::Hash(name.c_str(), name.size());
        }

        TRef load(const std::string& name) {
            TRef result = InvalidRef;
            if (useDeferredLoading)
                mutex.lock();
            typename std::map<std::string, TRef>::iterator it = nameToRef.find(name);
            if (it == nameToRef.end()) {
                result = name2ref(name);
                LOGF_IF(ref2index.find(result) != ref2index.end(), "Hash collision: '" << result << "' - change resource : '" << name << "' name");

                LOGT_EVERY_N(100, "Probably useless, because the load method should take a hash");
                nameToRef.insert(std::make_pair(name, result));

                if (useDeferredLoading) {
                    delayed.loads.insert(name);
                    LOGV(1, "Put asset '" << name << "' (" << name.size() << ") on delayed load queue. Ref value: " << result << ". NameToRef size: " << nameToRef.size());
                } else {
                    unsigned countBefore = assets.size();
                    assets.reserve(countBefore * 2);
                    assets.push_back(T());
                    doLoad(name, assets.back(), result);
                    ref2index[result] = countBefore;
                }
#if SAC_LINUX && SAC_DESKTOP
                // Monitor file change if not loaded from memory
                if (dataSource.find(result) == dataSource.end())
                    registerNewAsset(name);
#endif
            } else {
                result = it->second;
            }
            if (useDeferredLoading)
                mutex.unlock();

            return result;
        }

        void unload(const std::string& name) {
            mutex.lock();
            delayed.unloads.insert(name);
            mutex.unlock();
            if (!useDeferredLoading) update();
        }
        void reload(const std::string& name) {
            if (useDeferredLoading) {
                mutex.lock();
                delayed.reloads.insert(name);
                mutex.unlock();
            } else {
                TRef ref = nameToRef[name];
                doReload(name, ref);
            }
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
            if (!useDeferredLoading) update();
        }

        void reloadAll() {
            LOGV(1, "Reload all - " << nameToRef.size());
            for (typename std::map<std::string, TRef>::const_iterator it=nameToRef.begin(); it!=nameToRef.end(); ++it) {
                reload(it->first);
            }
        }

        void update() {
            LOGV_IF(1, !delayed.loads.empty(), "Process delayed loads");

            unsigned countBefore = assets.size();
            assets.reserve(countBefore + delayed.loads.size() * 2);
            assets.resize(countBefore + delayed.loads.size());

            for (std::set<std::string>::iterator it=delayed.loads.begin();
                it!=delayed.loads.end();
                ++it) {
                mutex.lock();
                TRef ref = nameToRef[*it];
                LOGV(2, "\tLoad '" << *it << "' -> " << ref);
                doLoad(*it, assets[countBefore], ref);
                ref2index[ref] = countBefore++;
                mutex.unlock();
            }

            LOGV_IF(1, !delayed.unloads.empty(), "Process delayed unloads");
            for (std::set<std::string>::iterator it=delayed.unloads.begin();
                it!=delayed.unloads.end();
                ++it) {
                mutex.lock();
                const std::string& name = *it;
                LOGV(2, "\tUnload '" << name << "'");
                TRef ref = nameToRef[name];
                doUnload(name, assets[ref2index[ref]]);
                ref2index.erase(ref);
                nameToRef.erase(name);
                mutex.unlock();
            }

            LOGV_IF(1, !delayed.reloads.empty(), "Process delayed reloads");
            for (std::set<std::string>::iterator it=delayed.reloads.begin();
                it!=delayed.reloads.end();
                ++it) {
                mutex.lock();
                const std::string& name = *it;
                LOGV(2, "Reload '" << name << "'");
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
            auto it = ref2index.find(ref);
            if (it == ref2index.end()) {
#if SAC_DEBUG
                LOGW("Unavailable resource requested " << ref << " '" << ref2Name(ref) << "'");
#else
                LOGW("Unavailable resource requested " << ref);
#endif
#if USE_COND_SIGNALING
                if (waitIfLoadingInProgress) {
                    // wait for next load end, the requested resource might be loaded in the next batch
                    while (!delayed.loads.empty())
                        cond.wait(lock);
                    it = ref2index.find(ref);
                } else {
#endif
                    lock.unlock();
                    return 0;
                }
            }

            LOGF_IF(it == ref2index.end(), "Unkown ref requested: " << ref << ". Asset count: " << ref2index.size());
            lock.unlock();
            return &assets[it->second];
        }

        const T& get(const std::string& name) {
            typename std::map<std::string, TRef>::const_iterator it = nameToRef.find(name);
            LOGF_IF(it == nameToRef.end(), "Unkown asset requested: " << name << ". Asset count: " << nameToRef.size());
            return *get(it->second, true);
        }

        void registerDataSource(TRef r, SourceDataType type) {
            if (dataSource.find(r) != dataSource.end())
                LOGW("Asset " << r << " already have one data source registered");
            dataSource.insert(std::make_pair(r, type));
        }
        bool isRegisteredDataSource(TRef r) {
            return (dataSource.find(r) != dataSource.end());
        }

        void unregisterDataSource(TRef r) {
            auto it = dataSource.find(r);
            if (it == dataSource.end()) {
                LOGW("Asset " << r << " is not registered, can't unregister it");
            } else {
                dataSource.erase(it);
            }
        }

        void add(const std::string& name, const T& info) {
            if (useDeferredLoading) mutex.lock();
            TRef ref = name2ref(name);
            nameToRef.insert(std::make_pair(name, ref));
            ref2index[ref] = assets.size();
            assets.push_back(info);
            if (useDeferredLoading) mutex.unlock();
        }

        #if SAC_DEBUG || SAC_INGAME_EDITORS
        const std::string& ref2Name(TRef ref) const {
            for (const auto& p: nameToRef) {
                if (p.second == ref)
                    return p.first;
            }
            static const std::string empty("");
            return empty;
        }
        #endif

    protected:
        virtual bool doLoad(const std::string& name, T& out, const TRef& ref) = 0;
        virtual void doUnload(const std::string& name, const T& in) = 0;
        virtual void doReload(const std::string& name, const TRef& ref) = 0;

    protected:
        std::mutex mutex;
#if USE_COND_SIGNALING
        std::condition_variable cond;
#endif
        AssetAPI* assetAPI;
        bool useDeferredLoading;

        std::map<std::string, TRef> nameToRef;
        std::map<TRef, int> ref2index;
        std::vector<T> assets;
        std::map<TRef, SourceDataType> dataSource;
        struct {
            std::set<std::string> loads;
            std::set<std::string> unloads;
            std::set<std::string> reloads;
        } delayed;
};
