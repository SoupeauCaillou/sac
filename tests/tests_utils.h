
#include "base/EntityManager.h"

struct NeedsEntityManager {
    NeedsEntityManager() {
        EntityManager::CreateInstance();
    }

    void uninit() {
        theEntityManager.deleteAllEntities();
        EntityManager::DestroyInstance();
    }
};
