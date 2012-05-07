#pragma once
#include <stdint.h>

struct OpaqueMusicPtr { };

#define SAMPLES_TO_SEC(nb, freq) ((nb) / (float)freq)
#define SEC_TO_SAMPLES(s, freq) (int) ((s) * freq)
#define SEC_TO_BYTE(s, freq) (int)((s) * freq * 2)
#define SAMPLES_TO_BYTE(nb, freq) SEC_TO_BYTE(SAMPLES_TO_SEC(nb, freq), freq)

class MusicAPI {
    public:
        virtual void init() = 0;
        // create internal state (source for OpenAL, AudioTrack for Android, etc...)
        virtual OpaqueMusicPtr* createPlayer(int sampleRate) = 0;
        virtual int8_t* allocate(OpaqueMusicPtr* _ptr, int size) = 0;
        virtual void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate)=0;
        virtual int needData(OpaqueMusicPtr* ptr, int sampleRate, bool firstCall)=0;
        virtual bool isPlaying(OpaqueMusicPtr* ptr)=0;
        virtual void startPlaying(OpaqueMusicPtr* ptr, OpaqueMusicPtr* master, int offset)=0;
        virtual void stopPlayer(OpaqueMusicPtr* ptr)=0;
        virtual int getPosition(OpaqueMusicPtr* ptr)=0;
        virtual void setPosition(OpaqueMusicPtr* ptr, int pos)=0;
        virtual void setVolume(OpaqueMusicPtr* ptr, float v)=0;
        virtual void deletePlayer(OpaqueMusicPtr* ptr)=0;
};
