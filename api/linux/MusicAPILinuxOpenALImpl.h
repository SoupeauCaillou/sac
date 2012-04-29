#pragma once

#include "../MusicAPI.h"

class MusicAPILinuxOpenALImpl : public MusicAPI {
    public:
        void init();
        OpaqueMusicPtr* createPlayer();
        void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size);
        bool needData(OpaqueMusicPtr* ptr);
        void startPlaying(OpaqueMusicPtr* ptr);
        void stopPlayer(OpaqueMusicPtr* ptr);
        int getPosition(OpaqueMusicPtr* ptr);
        void setPosition(OpaqueMusicPtr* ptr, int pos);
        void setVolume(OpaqueMusicPtr* ptr, float v);
        void deletePlayer(OpaqueMusicPtr* ptr);
        bool isPlaying(OpaqueMusicPtr* ptr);
};
