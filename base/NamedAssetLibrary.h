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

    protected:
    struct RefIndex {
        TRef ref;
        int index;

        RefIndex(TRef r = InvalidRef, int i = 0) : ref(r), index(i) {}

        bool operator<(const RefIndex& other) const { return ref < other.ref; }
    };

    int ref2Index(TRef ref, bool logIfMissing = true) const {
        auto it = std::lower_bound(ref2indexv.begin(), ref2indexv.end(), ref);

        if (it == ref2indexv.end() || (*it).ref != ref) {
            if (logIfMissing) {
                LOGV(1, "Uh, ref is missing: " << (unsigned int)ref);
                if (sizeof(ref) == sizeof(hash_t))
                    LOGV(1,
                         "\tinv_hash(" << (unsigned int)ref << ") = '"
                                       << INV_HASH(ref) << "'");
                LOGV(1, "ref2indexv content (" << ref2indexv.size() << ":");
                for (int i = 0; i < (int)ref2indexv.size(); i++) {
                    LOGV(1,
                         "  " << i << " -> ref=" << ref2indexv[i].ref
                              << ", index=" << ref2indexv[i].index);
                }
            }
            return -1;
        }
        return (*it).index;
    }

    void _addRef2Index(TRef r, int index) {
        const auto e = ref2indexv.end();
        auto it = ref2indexv.begin();
        while (it != e && (*it).ref < r) ++it;
        ref2indexv.emplace(it, RefIndex(r, index));
    }

    public:
    NamedAssetLibrary() : assetAPI(0), useDeferredLoading(true) {}

    virtual void init(AssetAPI* pAssetAPI, bool pUseDeferredLoading = true) {
        assetAPI = pAssetAPI;
        useDeferredLoading = pUseDeferredLoading;
        assets.reserve(8);
    }

    virtual ~NamedAssetLibrary() { ref2indexv.clear(); }

    TRef name2ref(const std::string& name) const {
        return Murmur::RuntimeHash(name.c_str(), name.size());
    }

    TRef load(const char* name) {
        TRef result = Murmur::RuntimeHash(name);

        if (useDeferredLoading) mutex.lock();

        int existingIndex = ref2Index(result, false);

        if (existingIndex == -1) {
            if (useDeferredLoading) {
                delayed.loads.insert(name);
                LOGV(1, "Put asset '" << name << "' on delayed load queue.");
            } else {
                unsigned countBefore = assets.size();
                assets.reserve(countBefore * 2);
                assets.push_back(T());

                doLoad(name, assets.back(), result);

                _addRef2Index(result, countBefore);
            }
#if SAC_LINUX && SAC_DESKTOP
            // Monitor file change if not loaded from memory
            if (dataSource.find(result) == dataSource.end())
                registerNewAsset(name);
#endif
            ref2name[result] = name;
        }
        if (useDeferredLoading) mutex.unlock();

        return result;
    }

    void unload(const char* name) {
        mutex.lock();
        delayed.unloads.insert(Murmur::RuntimeHash(name));
        mutex.unlock();
        if (!useDeferredLoading) update();
    }
    void unload(const TRef& ref) {
        mutex.lock();
        delayed.unloads.insert(ref);
        mutex.unlock();
        if (!useDeferredLoading) update();
    }

    void reload(const char* name) {
        if (useDeferredLoading) {
            mutex.lock();
            delayed.reloads.insert(name);
            mutex.unlock();
        } else {
            TRef ref = Murmur::RuntimeHash(name);
            doReload(name, ref);
        }
    }
    void reloadAll() {
        LOGV(1, "Reload all - " << ref2indexv.size());
        for (auto& it : ref2indexv) { reload(ref2name[it.ref].c_str()); }
    }

    void update() {
        LOGV_IF(1, !delayed.loads.empty(), "Process delayed loads");

        if (!delayed.loads.empty()) {
            unsigned countBefore = assets.size();
            assets.reserve(countBefore + delayed.loads.size() * 2);
            assets.resize(countBefore + delayed.loads.size());

            for (auto& name : delayed.loads) {
                mutex.lock();
                TRef ref = Murmur::RuntimeHash(name.c_str());
                LOGV(2, "\tLoad '" << name << "' -> " << ref);
                doLoad(name.c_str(), assets[countBefore], ref);
                _addRef2Index(ref, countBefore++);
                mutex.unlock();
            }
        }

        LOGV_IF(1, !delayed.unloads.empty(), "Process delayed unloads");
        for (auto ref : delayed.unloads) {
            mutex.lock();
            LOGV(2, "\tUnload '" << ref << "'");
            int idx = ref2Index(ref);
            LOGF_IF(idx < 0, "Trying to unload invalid resources:" << ref);
            doUnload(assets[idx]);
            auto it =
                std::lower_bound(ref2indexv.begin(), ref2indexv.end(), ref);
            ref2indexv.erase(it);
            mutex.unlock();
        }

        LOGV_IF(1, !delayed.reloads.empty(), "Process delayed reloads");
        for (const auto& name : delayed.reloads) {
            mutex.lock();
            LOGV(2, "Reload '" << name << "'");
            doReload(name.c_str(), Murmur::RuntimeHash(name.c_str()));
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
        auto index = ref2Index(ref);
        if (index < 0) {
#if SAC_DEBUG
            LOGW("Unavailable resource requested " << ref << " '"
                                                   << ref2Name(ref) << "'");
#else
            LOGW("Unavailable resource requested " << ref);
#endif
#if USE_COND_SIGNALING
            if (waitIfLoadingInProgress) {
                // wait for next load end, the requested resource might be
                // loaded in the next batch
                while (!delayed.loads.empty()) cond.wait(lock);
                index = ref2Index(ref);
            } else {
#endif
                lock.unlock();
                return 0;
            }
        }

        LOGF_IF(index < 0,
                "Unkown ref requested: " << ref << ". Asset count: "
                                         << ref2indexv.size());
        lock.unlock();
        return &assets[index];
    }

    void registerDataSource(TRef r, SourceDataType type) {
        if (dataSource.find(r) != dataSource.end())
            LOGW("Asset 0x" << LogFmt("%08x") << (int)r << LogFmt()
                            << " already have one data source registered");
        dataSource.insert(std::make_pair(r, type));
    }
    bool isRegisteredDataSource(TRef r) {
        return (dataSource.find(r) != dataSource.end());
    }

    void unregisterDataSource(TRef r) {
        auto it = dataSource.find(r);
        if (it == dataSource.end()) {
            LOGW("Asset " << r << " is not registered, can't unregister it");
        } else { dataSource.erase(it); }
    }

    void add(const std::string& name, const T& info) {
        if (useDeferredLoading) mutex.lock();
        TRef ref = Murmur::RuntimeHash(name.c_str());
        _addRef2Index(ref, assets.size());
        assets.push_back(info);
        if (useDeferredLoading) mutex.unlock();
    }

#if SAC_DEBUG || SAC_INGAME_EDITORS
    const std::string& ref2Name(TRef ref) const {
        auto it = ref2name.find(ref);
        if (it == ref2name.end()) {
            const static std::string s("");
            return s;
        } else
            return it->second;
    }

    const std::map<TRef, std::string>& getAllNames() const { return ref2name; }
    const std::vector<RefIndex>& getAllIndexes() const { return ref2indexv; }
#endif

    protected:
    virtual bool doLoad(const char* name, T& out, const TRef& ref) = 0;
    virtual void doUnload(const T& in) = 0;
    virtual void doReload(const char* name, const TRef& ref) = 0;

    protected:
    std::mutex mutex;
#if USE_COND_SIGNALING
    std::condition_variable cond;
#endif
    AssetAPI* assetAPI;
    bool useDeferredLoading;

    std::map<TRef, std::string> ref2name;
    std::vector<RefIndex> ref2indexv;
    std::vector<T> assets;
    std::map<TRef, SourceDataType> dataSource;
    struct {
        std::set<std::string> loads;
        std::set<TRef> unloads;
        std::set<std::string> reloads;
    } delayed;
};
