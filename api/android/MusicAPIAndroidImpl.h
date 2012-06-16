/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "../MusicAPI.h"
#include <jni.h>

class MusicAPIAndroidImpl : public MusicAPI {
    public:
    	MusicAPIAndroidImpl();
    	~MusicAPIAndroidImpl();
        void init(JNIEnv *env);
        void uninit();
        OpaqueMusicPtr* createPlayer(int sampleRate);
        int pcmBufferSize(int sampleRate);
        int8_t* allocate(int size);
        void deallocate(int8_t* b);
        int initialPacketCount(OpaqueMusicPtr* ptr);
        void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate);
        void startPlaying(OpaqueMusicPtr* ptr, OpaqueMusicPtr* master, int offset);
        void stopPlayer(OpaqueMusicPtr* ptr);
        int getPosition(OpaqueMusicPtr* ptr);
        void setPosition(OpaqueMusicPtr* ptr, int pos);
        void setVolume(OpaqueMusicPtr* ptr, float v);
        void deletePlayer(OpaqueMusicPtr* ptr);
        bool isPlaying(OpaqueMusicPtr* ptr);
        
	private:
		JNIEnv *env;
		struct MusicAPIAndroidImplData;
		MusicAPIAndroidImplData* datas;
};
