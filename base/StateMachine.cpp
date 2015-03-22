#include "Entity.h"
#include "EntityManager.h"
#include <map>
#include "api/AssetAPI.h"
#include "base/Log.h"
#include "util/MurmurHash.h"
#include <algorithm>
#include <sstream>

static char* buildFullName(const char* stateName, const char* filename, char* tmp) {
    auto* fullname = tmp;
    strcpy(fullname, stateName);
    strcat(fullname, "/");
    strcat(fullname, filename);

    return tmp;
}

void initStateEntities(AssetAPI* assetAPI, const char* stateName, std::map<hash_t, Entity>& entities) {
    LOGF_IF(!assetAPI, "Missing assetAPI");

    // Retrieve .entity in scene subfolder
    char* subfolder = (char*) alloca(sizeof("entities/") + strlen(stateName) + 100);
    strcpy(subfolder, "entities/");
    strcat(subfolder, stateName);
    auto files = assetAPI->listAssetContent(".entity", subfolder);

    std::list<Entity> entitiesBuilt;

    unsigned buildCount = entitiesBuilt.size();

    // build all entities (inefficient but only used ~10 times)
    while (!files.empty()) {
        bool earlyExit = false;

        for (auto f=files.begin(); f!=files.end(); ++f) {
            auto* fullname = buildFullName(stateName, (*f).c_str(), subfolder);

            const EntityTemplateRef tmpl = theEntityManager.entityTemplateLibrary.load(fullname);
            const EntityTemplate* t = theEntityManager.entityTemplateLibrary.get(tmpl, false);

            if (!t->autoCreate) {
                files.erase(f);
                earlyExit = true;
                break;
            }

            // Only build if all dependencies have been built
            bool dependenciesReady = true;
            std::for_each(t->dependencies.begin(), t->dependencies.end(), [&entitiesBuilt, &dependenciesReady] (hash_t h) -> void {
                Entity e = theEntityManager.getEntityByName(h);
                if (e == 0) {
                    if (std::find(entitiesBuilt.begin(), entitiesBuilt.end(), h) == entitiesBuilt.end()) {
                        LOGV(2, "Missing " << INV_HASH(h));
                        dependenciesReady = false;
                    }
                }
            });

            if (dependenciesReady) {
                hash_t h = Murmur::RuntimeHash(fullname);
                entities[h] =
                    theEntityManager.CreateEntityFromTemplate(fullname);
                entitiesBuilt.push_back(h);

                files.erase(f);
                break;
            }
        }

        if (!earlyExit) {
            unsigned newCount = entitiesBuilt.size();
            if (newCount <= buildCount) {
                #if SAC_ENABLE_LOG
                LOGE("Entities built:");
                for (auto e: entitiesBuilt) LOGE("\t* " << INV_HASH(e) << ".entity");
                LOGE("Entities not-built");
                for (auto e: files) {
                    auto* fullname = buildFullName(stateName, (e).c_str(), subfolder);
                    const EntityTemplateRef tmpl = theEntityManager.entityTemplateLibrary.load(fullname);
                    const EntityTemplate* t = theEntityManager.entityTemplateLibrary.get(tmpl, false);

                    std::stringstream d;
                    d << "dependencies=";
                    for (auto dep: t->dependencies) {
                        d << INV_HASH(dep) << "  ";
                    }
                    LOGE("\t* " << stateName << '/' << e << ".entity (" << d.str() << ')');
                }
                #endif
                LOGF("Entities defined for state " << stateName << " have circular dependencies (" << entitiesBuilt.size() << " built and " << files.size() << " not built)");
            }
            buildCount = newCount;
        }
    }
}
