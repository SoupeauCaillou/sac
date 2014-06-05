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



#include "EntityTemplateLibrary.h"
#include "util/DataFileParser.h"
#include "util/ComponentFactory.h"
#include "systems/System.h"
#include "base/EntityManager.h"
#include "systems/TransformationSystem.h"
#include "api/LocalizeAPI.h"

EntityTemplateLibrary::~EntityTemplateLibrary() {
    for (auto& tmp: assets) {
        for (auto it: tmp) {
            for (auto jt: it.second) {
                delete[] jt.second;
            }
        }
    }
    assets.clear();
}

void EntityTemplateLibrary::registerDataSource(EntityTemplateRef ref, FileBuffer fb) {
    NamedAssetLibrary::registerDataSource(ref, fb);
}

bool EntityTemplateLibrary::isRegisteredDataSource(EntityTemplateRef ref) {
    return NamedAssetLibrary::isRegisteredDataSource(ref);
}

void EntityTemplateLibrary::unregisterDataSource(EntityTemplateRef ) {
    LOGT("Pfff");
    #if 0
    NamedAssetLibrary::unregisterDataSource(ref);
    ref2asset.erase(ref);
    for (auto it = nameToRef.begin(); it != nameToRef.end(); ++it) {
        if (it->second == ref) {
            nameToRef.erase(it);
            return;
        }
    }
    #endif
}


int EntityTemplateLibrary::loadTemplate(const std::string& context, const DataFileParser& dfp, EntityTemplateRef, EntityTemplate& out) {
    int propCount = 0;
    // browse system
    const auto& systems = ComponentSystem::registeredSystems();
    for (auto it = systems.begin(); it!=systems.end(); ++it) {
        hash_t id = it->first;

        if (dfp.hasSection(id)) {
            propCount += ComponentFactory::build(
                context, dfp, id, it->second->getSerializer().getProperties(), out);
        }
    }
    return propCount;
}

#define FILE_PREFIX "entities/"
#define FILE_PREFIX_LEN sizeof(FILE_PREFIX)

#define FILE_SUFFIX ".entity"
#define FILE_SUFFIX_LEN sizeof(FILE_PREFIX)

const char* EntityTemplateLibrary::asset2FilePrefix() const { return FILE_PREFIX; }
const char* EntityTemplateLibrary::asset2FileSuffix() const { return FILE_SUFFIX; }

bool EntityTemplateLibrary::doLoad(const char* name, EntityTemplate& out, const EntityTemplateRef& r) {
    std::map<EntityTemplateRef, FileBuffer>::iterator it = dataSource.find(r);
    DataFileParser dfp;
    FileBuffer fb;

    int l = FILE_PREFIX_LEN + strlen(name) + FILE_SUFFIX_LEN + 1;
    char* filename = (char*)alloca(l);
    asset2File(name, filename, l);

    if (it == dataSource.end()) {
        LOGV(1, "loadEntityTemplate: '" << name << "' from file");
        fb = assetAPI->loadAsset(filename);
        if (!fb.size) {
            LOGF("Unable to load '" << filename << "'");
            return false;
        }
    } else {
        LOGV(1, "loadEntityTemplate: '" << name << "' from InMemoryEntityTemplate (" << fb.size << ')');
        fb = it->second;
    }

    if (!dfp.load(fb, filename)) {
        LOGE("Unable to parse '" << filename << "'");
        delete[] fb.data;
        return false;
    }

    // Handle recursive 'extends' attributes
    std::string extends;
    while (dfp.get(DataFileParser::GlobalSection, "extends", &extends, 1, false)) {
        LOGV(1, name << " extends: " << extends);
        // remove extends key
        dfp.remove(DataFileParser::GlobalSection, "extends");

        int l2 = FILE_PREFIX_LEN + extends.length() + FILE_SUFFIX_LEN + 1;
        char* filename2 = (char*)alloca(l2);
        asset2File(extends.c_str(), filename2, l);

        FileBuffer fb2 = assetAPI->loadAsset(filename2);
        if (!fb2.size) {
            LOGE("Unable to load 'extends' file: '" << extends << "'");
        } else if (!dfp.load(fb2, filename2)) {
            LOGE("Unable to parse 'extends' file '" << extends << "'");
        }

        //register only if we are not using custom resource
        if (it == dataSource.end()) {
            registerNewAsset(name, extends);
        }
        delete[] fb2.data;
    }

    LOG_USAGE_ONLY(int propCount =) loadTemplate(name, dfp, r, out);
    LOGV(1, "Loaded entity template: '" << name << "' (" << propCount << " properties)");

    delete[] fb.data;

    return true;
}

void EntityTemplateLibrary::doUnload(const EntityTemplate&) {
    LOGT("TODO");
}

#if SAC_LINUX && SAC_DESKTOP
void EntityTemplateLibrary::applyTemplateToAll(const EntityTemplateRef& ref) {
    auto it = template2entities.find(ref);
    if (it != template2entities.end()) {
        for (auto ent: (*it).second) {
            LOGV(2, "apply template to '" << theEntityManager.entityName(ent) << "'");
            applyEntityTemplate(ent, ref);
        }
    }
    // also reload all template being children of this one
    auto jt = template2children.find(ref);
    if (jt != template2children.end()) {
        for (auto child: jt->second)
            applyTemplateToAll(child);
    }
}
#endif

void EntityTemplateLibrary::doReload(const char* name, const EntityTemplateRef& ref) {
    LOGV(1, "RELOAD: " << name);
    EntityTemplate newTempl;
    if (doLoad(name, newTempl, ref)) {
        assets[ref2Index(ref)] = newTempl;
        #if SAC_LINUX && SAC_DESKTOP
            applyTemplateToAll(ref);
        #endif
    } else {
        LOGW("Unable to reload '" << name << "'");
    }
}

void EntityTemplateLibrary::applyEntityTemplate(Entity e, const EntityTemplateRef& templRef) {
    LOGV(2, "apply template " << templRef << " to '" << theEntityManager.entityName(e) << "'");

    auto parentIt = template2parent.find(templRef);
    if (parentIt != template2parent.end()) {
        LOGV(1, "apply parent first");
        applyEntityTemplate(e, parentIt->second);
    }

    const auto tIt = ref2Index(templRef);// ref2index.find(templRef);
    LOGF_IF(tIt < 0, "Unknown entity template ref ! " << templRef);;
    const EntityTemplate& templ = assets[tIt];
    // Small hack, always load Transformation first
    auto transfS = TransformationSystem::GetInstancePointer();
    auto hack = templ.find(transfS);
    if (hack != templ.end()) {
        theEntityManager.AddComponent(e, transfS, false);
        transfS->applyEntityTemplate(e, (*hack).second, localizeAPI);
    }
    for (auto it: templ) {
        ComponentSystem* s = it.first;
        if (s == transfS) continue;

        theEntityManager.AddComponent(e, s, false);
        s->applyEntityTemplate(e, it.second, localizeAPI);
    }
    #if SAC_LINUX && SAC_DESKTOP
        template2entities[templRef].insert(e);
    #endif
}

#if SAC_LINUX && SAC_DESKTOP
void EntityTemplateLibrary::remove(Entity e) {
    for (auto it : template2entities) {
        if (template2entities.erase(e) > 0)
            break;
    }
}
#endif

void EntityTemplateLibrary::defineParent(EntityTemplateRef child, EntityTemplateRef parent) {
    template2parent.insert(std::make_pair(child, parent));
    #if SAC_LINUX && SAC_DESKTOP
        template2children[parent].insert(child);
    #endif
}
