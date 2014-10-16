/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



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
        int initialPacketCount(OpaqueMusicPtr* ptr);
        void queueMusicData(OpaqueMusicPtr* ptr, const short* data, int size, int sampleRate);
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
