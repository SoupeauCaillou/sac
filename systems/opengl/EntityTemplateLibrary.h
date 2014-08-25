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

#include "base/NamedAssetLibrary.h"
#include "base/Entity.h"
#include "api/AssetAPI.h"

typedef int EntityTemplateRef;
#define InvalidEntityTemplateRef -1

struct Void {};

#include <map>
#include <set>
#include <string>

class DataFileParser;
class ComponentSystem;
class LocalizeAPI;
typedef std::map<hash_t, uint8_t*> PropertyNameValueMap;
struct EntityTemplate {
    std::map<ComponentSystem*, PropertyNameValueMap> properties;
    std::vector<hash_t> dependencies;
};

class EntityTemplateLibrary : public NamedAssetLibrary<EntityTemplate, EntityTemplateRef, FileBuffer> {
    protected:
        bool doLoad(const char* name, EntityTemplate& out, const EntityTemplateRef& ref);

        void doUnload(const EntityTemplate& in);

        void doReload(const char* name, const EntityTemplateRef& ref);

    public:
        void setLocalizeAPI(LocalizeAPI* api) { localizeAPI = api; }

        void applyEntityTemplate(Entity e, const EntityTemplateRef& templ);

        void defineParent(EntityTemplateRef child, EntityTemplateRef parent);

        void registerDataSource(EntityTemplateRef ref, FileBuffer fb);
        bool isRegisteredDataSource(EntityTemplateRef r);
        void unregisterDataSource(EntityTemplateRef r);

        ~EntityTemplateLibrary();

        const char* asset2FilePrefix() const;
        const char* asset2FileSuffix() const;

#if SAC_LINUX && SAC_DESKTOP
        void remove(Entity e);
#endif
    private:
        #if SAC_LINUX && SAC_DESKTOP
        std::map<EntityTemplateRef, std::set<Entity> > template2entities;
        std::map<EntityTemplateRef, std::set<EntityTemplateRef> > template2children;
        #endif
        std::map<EntityTemplateRef, EntityTemplateRef> template2parent;
        LocalizeAPI* localizeAPI;

        void applyTemplateToAll(const EntityTemplateRef& ref);

        int loadTemplate(const std::string& context, const DataFileParser& dfp, EntityTemplateRef r, EntityTemplate& out);
};
