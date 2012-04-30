#pragma once

#include "../MusicAPI.h"

class MusicAPILinuxOpenALImpl : public MusicAPI {
    public:
        void init();
        OpaqueMusicPtr* createPlayer(int sampleRate);
        void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate);
        bool needData(OpaqueMusicPtr* ptr, int sampleRate);
        void startPlaying(OpaqueMusicPtr* ptr);
        void stopPlayer(OpaqueMusicPtr* ptr);
        int getPosition(OpaqueMusicPtr* ptr);
        void setPosition(OpaqueMusicPtr* ptr, int pos);
        void setVolume(OpaqueMusicPtr* ptr, float v);
        void deletePlayer(OpaqueMusicPtr* ptr);
        bool isPlaying(OpaqueMusicPtr* ptr);
};
