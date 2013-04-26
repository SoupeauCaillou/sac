#include "EntityTemplateLibrary.h"
#include "util/DataFileParser.h"
#include "util/ComponentFactory.h"
#include "systems/System.h"

bool EntityTemplateLibrary::doLoad(const std::string& name, EntityTemplate& out, const EntityTemplateRef& ref) {
    DataFileParser dfp;
    FileBuffer fb = assetAPI->loadAsset(asset2File(name));
    if (!fb.size) {
        LOGE("Unable to load '" << asset2File(name) << "'")
        return false;
    }
    if (!dfp.load(fb)) {
        LOGE("Unable to parse '" << asset2File(name) << "'")
        delete[] fb.data;
        return false;
    }

    int propCount = 0;
    // browse system
    const std::map<std::string, ComponentSystem*>& systems = ComponentSystem::registeredSystems();
    for (auto it = systems.begin(); it!=systems.end(); ++it) {
        if (dfp.hasSection(it->first)) {
            LOGI(it->first)
            propCount += ComponentFactory::build(dfp, it->first, it->second->getSerializer().getProperties(), out);
        }
    }

    LOGI( "Loaded entity template: '" << name << "' (" << propCount << " properties)")
    delete[] fb.data;
    return true;
}

void EntityTemplateLibrary::doUnload(const std::string& name, const EntityTemplate& in) {
	LOGE("TODO")
}

void EntityTemplateLibrary::doReload(const std::string& name, const EntityTemplateRef& ref) {
    LOGV(1, "RELOAD: " << name)
    EntityTemplate newTempl;
    if (doLoad(name, newTempl, ref)) {
        ref2asset[ref] = newTempl;
#if SAC_LINUX && SAC_DESKTOP
        auto it = template2entities.find(ref);
        if (it != template2entities.end()) {
            for (auto ent: (*it).second) {
                LOGV(2, "apply template to '" << theEntityManager.entityName(ent) << "'")
                applyEntityTemplate(ent, ref);
            }
        }
#endif
    } else {
        LOGW("Unable to reload '" << name << "'")
    }
}

void EntityTemplateLibrary::applyEntityTemplate(Entity e, const EntityTemplateRef& templRef) {
    LOGV(2, "apply template " << templRef << " to '" << theEntityManager.entityName(e) << "'")
    // typedef std::map<std::string, uint8_t*> PropertyNameValueMap;
    // typedef std::map<ComponentSystem*, PropertyNameValueMap> EntityTemplate;
    const EntityTemplate& templ = ref2asset[templRef];
    for (auto it: templ) {
        ComponentSystem* s = it.first;
        s->applyEntityTemplate(e, it.second);
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
