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

typedef int EntityTemplateRef;
#define InvalidEntityTemplateRef -1

struct Void {};

#include <map>
#include <set>
#include <string>
class WWWAPI;
class DataFileParser;
struct FileBuffer;
class ComponentSystem;
typedef std::map<std::string, uint8_t*> PropertyNameValueMap;
typedef std::map<ComponentSystem*, PropertyNameValueMap> EntityTemplate;

class EntityTemplateLibrary : public NamedAssetLibrary<EntityTemplate, EntityTemplateRef, FileBuffer> {
    protected:
        bool doLoad(const std::string& name, EntityTemplate& out, const EntityTemplateRef& ref);

        void doUnload(const std::string& name, const EntityTemplate& in);

        void doReload(const std::string& name, const EntityTemplateRef& ref);

    public:
        std::string asset2File(const std::string& assetName) const { return "entities/" + assetName + ".entity"; }

        void applyEntityTemplate(Entity e, const EntityTemplateRef& templ);

        void defineParent(EntityTemplateRef child, EntityTemplateRef parent);

        void registerDataSource(EntityTemplateRef ref, FileBuffer fb);
        bool isRegisteredDataSource(EntityTemplateRef r);
        void unregisterDataSource(EntityTemplateRef r);

        ~EntityTemplateLibrary();

#if SAC_LINUX && SAC_DESKTOP
        void remove(Entity e);
#endif
    private:
        #if SAC_LINUX && SAC_DESKTOP
        std::map<EntityTemplateRef, std::set<Entity> > template2entities;
        std::map<EntityTemplateRef, std::set<EntityTemplateRef> > template2children;
        #endif
        std::map<EntityTemplateRef, EntityTemplateRef> template2parent;

        void applyTemplateToAll(const EntityTemplateRef& ref);

        int loadTemplate(const std::string& context, const std::string& prefix, const DataFileParser& dfp, EntityTemplateRef r, EntityTemplate& out);
};
