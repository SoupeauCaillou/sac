#pragma once

#include "../MusicAPI.h"
#include "JNIWrapper.h"

namespace jni_music_api {
    enum Enum {
        CreatePlayer,
        PcmBufferSize,
        Allocate,
        Deallocate,
        InitialPacketCount,
        QueueMusicData,
        StartPlaying,
        StopPlayer,
        PausePlayer,
        GetPosition,
        SetPosition,
        SetVolume,
        DeletePlayer,
        IsPlaying
    };
}

class MusicAPIAndroidImpl : public MusicAPI, public JNIWrapper<jni_music_api::Enum> {
    public:
    	MusicAPIAndroidImpl();

        OpaqueMusicPtr* createPlayer(int sampleRate);
        int pcmBufferSize(int sampleRate);
        int8_t* allocate(int size);
        void deallocate(int8_t* b);
        int initialPacketCount(OpaqueMusicPtr* ptr);
        void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate);
        void startPlaying(OpaqueMusicPtr* ptr, OpaqueMusicPtr* master, int offset);
        void stopPlayer(OpaqueMusicPtr* ptr);
        void pausePlayer(OpaqueMusicPtr* ptr);
        int getPosition(OpaqueMusicPtr* ptr);
        void setPosition(OpaqueMusicPtr* ptr, int pos);
        void setVolume(OpaqueMusicPtr* ptr, float v);
        void deletePlayer(OpaqueMusicPtr* ptr);
        bool isPlaying(OpaqueMusicPtr* ptr);
    private:
        std::map<int8_t*, jbyteArray> ptr2array;
};
