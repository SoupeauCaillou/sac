#pragma once
#include <stdint.h>

struct OpaqueMusicPtr { };

class MusicAPI {
    public:
        virtual void init() = 0;
        // create internal state (source for OpenAL, AudioTrack for Android, etc...)
        virtual OpaqueMusicPtr* createPlayer() = 0;
        virtual void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size)=0;
        virtual bool needData(OpaqueMusicPtr* ptr)=0;
        virtual bool isPlaying(OpaqueMusicPtr* ptr)=0;
        virtual void startPlaying(OpaqueMusicPtr* ptr)=0;
        virtual void stopPlayer(OpaqueMusicPtr* ptr)=0;
        virtual int getPosition(OpaqueMusicPtr* ptr)=0;
        virtual void setPosition(OpaqueMusicPtr* ptr, int pos)=0;
        virtual void setVolume(OpaqueMusicPtr* ptr, float v)=0;
        virtual void deletePlayer(OpaqueMusicPtr* ptr)=0;
};
