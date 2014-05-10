#include "ReplayManager.h"
#include "api/AssetAPI.h"

ReplayManager* ReplayManager::instance = 0;

ReplayManager* ReplayManager::Instance() {
    if (!instance)
        instance = new ReplayManager;
    return instance;
}

ReplayManager::ReplayManager() : replayMode(false) {
    #if SAC_DESKTOP
    outDfp.init();
    #endif
    touching = 0;
    frameCount = 0;
}

bool ReplayManager::isReplayModeEnabled() const {
    return replayMode;
}

void ReplayManager::enableReplayMode(const char* sourceFile, AssetAPI* assetAPI) {
    LOGF_IF(replayMode, "Multiple calls to " << __FUNCTION__);

    FileBuffer fb = assetAPI->loadFile(sourceFile);
    LOGF_IF (!fb.size, "Invalid replay source file: " << sourceFile);

    LOGV(1, "Init replay from '" << sourceFile << "'");
    sourceDfp.load(fb, sourceFile);
}


unsigned int ReplayManager::getRandomSeed() const {
    LOGF_IF(!replayMode, __FUNCTION__ << " used but replay is disabled");
    unsigned int seed = 0;
    int result = sourceDfp.get("Misc", "random_seed", &seed);
    LOGF_IF(!result, "Unable to read seed from replay file");
    return seed;
}

void ReplayManager::saveRandomSeed(unsigned int seed) {
    #if SAC_DESKTOP
    outDfp.set("Misc", "random_seed", &seed);
    #endif
}

void ReplayManager::saveMaxTouchingCount(int count) {
    #if SAC_DESKTOP
    outDfp.set("Misc", "max_touching_count", &count);
    touching = new std::pair<bool, glm::vec2>[count];
    pointerCount = count;
    for (int i=0; i<count; i++) {
        touching[i].first = false;
        touching[i].second = glm::vec2(0.0f);
    }
    #endif
}

int ReplayManager::maxTouchingCount() {
    LOGF_IF(!replayMode, __FUNCTION__ << " used but replay is disabled");
    int result = sourceDfp.get("Misc", "max_touching_count", &pointerCount);
    LOGF_IF(!result, "Unable to read max_touching_count from replay file");

    // update touching state
    char* tmp1 = (char*) alloca(50);
    char* tmp2 = (char*) alloca(50);
    sprintf(tmp2, "frame_%lu", frameCount);
    for (int i=0; i<pointerCount; i++) {
        glm::vec2 c;
        sprintf(tmp1, "Pointer_%d", i);
        if (sourceDfp.get(tmp1, tmp2, &c.x, 2)) {
            touching[i].first = true;
            touching[i].second = c;
        } else {
            touching[i].first = false;
        }
    }

    frameCount++;
    return pointerCount;
}

bool ReplayManager::isTouching(int index, glm::vec2* windowCoords) {
    LOGF_IF(!replayMode, __FUNCTION__ << " used but replay is disabled");

    if (!touching)
        return false;

    *windowCoords = touching[index].second;
    return touching[index].first;
}

void ReplayManager::saveIsTouching(bool touching, int index, const glm::vec2& windowCoords) {
    #if SAC_DESKTOP
    if (touching) {
        char* tmp1 = (char*) alloca(50);
        sprintf(tmp1, "Pointer_%d", index);

        char* tmp2 = (char*) alloca(50);
        sprintf(tmp2, "frame_%lu", frameCount);
        outDfp.set(tmp1, tmp2,  &windowCoords.x, 2);
    }
    #endif
}

bool ReplayManager::isMoving (int ) {
    return false;
}
