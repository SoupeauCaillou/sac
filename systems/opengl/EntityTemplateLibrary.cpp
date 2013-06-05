#include "EntityTemplateLibrary.h"
#include "util/DataFileParser.h"
#include "util/ComponentFactory.h"
#include "systems/System.h"
#include "base/EntityManager.h"


int EntityTemplateLibrary::loadTemplate(const std::string& context, const std::string& prefix, const DataFileParser& dfp, EntityTemplateRef, EntityTemplate& out) {
    int propCount = 0;
    // browse system
    const std::map<std::string, ComponentSystem*>& systems = ComponentSystem::registeredSystems();
    for (auto it = systems.begin(); it!=systems.end(); ++it) {
        const std::string section(prefix + it->first);

        if (dfp.hasSection(section)) {
            std::vector<std::string> subEntities;
            propCount += ComponentFactory::build(
                context, dfp, section, it->second->getSerializer().getProperties(), out, subEntities);

            // if section define a subentity -> load it as template too
            if (!subEntities.empty())
                LOGI("Entity '" << context << "' has " << subEntities.size() << " sub entities");
            for (auto subName : subEntities) {
                std::string fullName (context + std::string("#") + subName);
                EntityTemplateRef subRef = MurmurHash::compute(fullName.c_str(), fullName.length());

                auto jt = ref2asset.find(subRef);
                if (jt == ref2asset.end()) {
                    jt = ref2asset.insert(std::make_pair(subRef, EntityTemplate())).first;
                }
                EntityTemplate& sub = jt->second;
                loadTemplate(context, subName + std::string("#"), dfp, subRef, sub);
                add(fullName, sub);
            }
        }
    }
    return propCount;
}

bool EntityTemplateLibrary::doLoad(const std::string& name, EntityTemplate& out, const EntityTemplateRef& r) {

    DataFileParser dfp;
    FileBuffer fb = assetAPI->loadAsset(asset2File(name));
    if (!fb.size) {
        LOGE("Unable to load '" << asset2File(name) << "'");
        return false;
    }
    if (!dfp.load(fb, asset2File(name))) {
        LOGE("Unable to parse '" << asset2File(name) << "'");
        delete[] fb.data;
        return false;
    }

    // Handle recursive 'extends' attributes
    std::string extends;
    while (dfp.get("", "extends", &extends, 1, false)) {
        LOGI(name << " extends: " << extends);
        // remove extends key
        dfp.remove("", "extends");

        FileBuffer fb2 = assetAPI->loadAsset(asset2File(extends));
        if (!fb2.size) {
            LOGE("Unable to load 'extends' file: '" << extends << "'");
        } else if (!dfp.load(fb2, asset2File(extends))) {
            LOGE("Unable to parse 'extends' file '" << extends << "'");
        }
        registerNewAsset(name, extends);
        delete[] fb2.data;
    }

    int propCount = loadTemplate(name, "", dfp, r, out);
    LOGI( "Loaded entity template: '" << name << "' (" << propCount << " properties)");

    delete[] fb.data;
    return true;
}

void EntityTemplateLibrary::doUnload(const std::string&, const EntityTemplate&) {
	LOGE("TODO");
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

void EntityTemplateLibrary::doReload(const std::string& name, const EntityTemplateRef& ref) {
    LOGE("RELOAD: " << name);
    EntityTemplate newTempl;
    if (doLoad(name, newTempl, ref)) {
        ref2asset[ref] = newTempl;
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
    // typedef std::map<std::string, uint8_t*> PropertyNameValueMap;
    // typedef std::map<ComponentSystem*, PropertyNameValueMap> EntityTemplate;
    const auto tIt = ref2asset.find(templRef);
    LOGF_IF(tIt == ref2asset.end(), "Unknown entity template ref ! " << templRef);;
    const EntityTemplate& templ = tIt->second;
    for (auto it: templ) {
        ComponentSystem* s = it.first;
        theEntityManager.AddComponent(e, s, false);
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

void EntityTemplateLibrary::defineParent(EntityTemplateRef child, EntityTemplateRef parent) {
    template2parent.insert(std::make_pair(child, parent));
#if SAC_LINUX && SAC_DESKTOP
    template2children[parent].insert(child);
#endif
}
