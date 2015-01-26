#pragma once

#include "util/DataFileParser.h"
#include "base/TouchInputManager.h"

class AssetAPI;

class ReplayManager : public NativeTouchState {
    public:
    ReplayManager();

    bool isReplayModeEnabled() const;
    void enableReplayMode(const char* sourceFile, AssetAPI* asset);

    unsigned int getRandomSeed() const;
    void saveRandomSeed(unsigned int seed);

    int maxTouchingCount();
    bool isTouching(int index, glm::vec2* windowCoords);
    bool isMoving(int index);

    void saveMaxTouchingCount(int count);
    void
    saveIsTouching(bool touching, int index, const glm::vec2& windowCoords);

    static ReplayManager* Instance();

    private:
    static ReplayManager* instance;

    bool replayMode;
    DataFileParser sourceDfp, outDfp;
    uint64_t frameCount;
    int pointerCount;
    std::pair<bool, glm::vec2>* touching;
};

#define theReplayManager (*ReplayManager::Instance())
