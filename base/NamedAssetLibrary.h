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
#include <pthread.h>

class AssetAPI;

template <typename T, typename TRef>
class NamedAssetLibrary {
    #define InvalidRef -1
    public:
        NamedAssetLibrary(AssetAPI* _assetAPI) : nextValidRef(1), assetAPI(_assetAPI) {
            pthread_mutex_init(&mutex, 0);
        }

        virtual ~NamedAssetLibrary() {
            pthread_mutex_destroy(&mutex);
        }

        TRef load(const std::string& name) {
            TRef result = InvalidRef;
            pthread_mutex_lock(&mutex);
            typename std::map<std::string, TRef>::iterator it = nameToRef.find(name);
            if (it == nameToRef.end()) {
                VLOG(1) << "Put asset '" << name << "' on delayed load queue";
                delayed.loads.insert(name);
                result = nextValidRef++;
                nameToRef.insert(std::make_pair(name, result));
            } else {
                result = it->second;
            }
            pthread_mutex_unlock(&mutex);
            return result;
        }

        void unload(const std::string& name) {
            pthread_mutex_lock(&mutex);
            delayed.unloads.insert(name);
            pthread_mutex_unlock(&mutex);
        }

        void update() {
            VLOG_IF(1, !delayed.loads.empty()) << "Process delayed loads";
            for (std::set<std::string>::iterator it=delayed.loads.begin();
                it!=delayed.loads.end();
                ++it) {
                pthread_mutex_lock(&mutex);
                T asset;
                TRef ref = nameToRef[*it];
                VLOG(2) << "\tLoad '" << *it << "' -> " << ref;
                doLoad(*it, asset);
                ref2asset.insert(std::make_pair(ref, asset));
                pthread_mutex_unlock(&mutex);
            }

            VLOG_IF(1, !delayed.unloads.empty()) << "Process delayed unloads";
            for (std::set<std::string>::iterator it=delayed.unloads.begin();
                it!=delayed.unloads.end();
                ++it) {
                pthread_mutex_lock(&mutex);
                const std::string& name = *it;
                VLOG(2) << "\tUnload '" << name << "'";
                TRef ref = nameToRef[name];
                doUnload(name, ref2asset[ref]);
                ref2asset.erase(ref);
                nameToRef.erase(name);
                pthread_mutex_unlock(&mutex);
            }

            pthread_mutex_lock(&mutex);
            delayed.loads.clear();
            delayed.unloads.clear();
            pthread_mutex_unlock(&mutex);
        }

    protected:
        void setNextValidRef(TRef r) {
            nextValidRef = r;
        }

    protected:
        virtual bool doLoad(const std::string& name, T& out, const TRef& ref) = 0;
        virtual void doUnload(const std::string& name, const T& in) = 0;
        virtual void reload(const std::string& name, T& out) = 0;
        AssetAPI* assetAPI;

    private:
        pthread_mutex_t mutex;
        TRef nextValidRef;

        std::map<std::string, TRef> nameToRef;
        std::map<TRef, T> ref2asset;
        struct {
            std::set<std::string> loads;
            std::set<std::string> unloads;
            std::set<std::string> reloads;
        } delayed;
};
