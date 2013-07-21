/*
    This file is part of sac.

    @author Soupe au Caillou

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
        //more info at http://publib.boulder.ibm.com/infocenter/comphelp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8a.doc%2Flanguage%2Fref%2Foverload_member_fn_base_derived.htm
        using NamedAssetLibrary<EntityTemplate, EntityTemplateRef, FileBuffer>::init;
        void init(AssetAPI* pAssetAPI, bool pUseDeferredLoading, WWWAPI* wwwAPI, const std::string & url = "");

        std::string asset2File(const std::string& assetName) const { return "entities/" + assetName + ".entity"; }

        void applyEntityTemplate(Entity e, const EntityTemplateRef& templ);

        void defineParent(EntityTemplateRef child, EntityTemplateRef parent);

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
