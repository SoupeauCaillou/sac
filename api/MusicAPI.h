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
#include <stdint.h>

struct OpaqueMusicPtr { };

#define SAMPLES_TO_SEC(nb, freq) ((nb) / (float)freq)
#define SEC_TO_SAMPLES(s, freq) (int) ((s) * freq)
#define SEC_TO_BYTE(s, freq) (int)((s) * freq * 2)
#define SAMPLES_TO_BYTE(nb, freq) SEC_TO_BYTE(SAMPLES_TO_SEC(nb, freq), freq)

class MusicAPI {
    public:
        // create internal state (source for OpenAL, AudioTrack for Android, etc...)
        virtual OpaqueMusicPtr* createPlayer(int sampleRate) = 0;
        virtual int pcmBufferSize(int sampleRate) = 0;
        virtual int8_t* allocate(int size) = 0;
        virtual void deallocate(int8_t* deallocate) = 0;
        virtual int initialPacketCount(OpaqueMusicPtr* ptr) = 0;
        virtual void queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate)=0;
        virtual bool isPlaying(OpaqueMusicPtr* ptr)=0;
        virtual void startPlaying(OpaqueMusicPtr* ptr, OpaqueMusicPtr* master, int offset)=0;
        virtual void stopPlayer(OpaqueMusicPtr* ptr)=0;
        virtual int getPosition(OpaqueMusicPtr* ptr)=0;
        virtual void setPosition(OpaqueMusicPtr* ptr, int pos)=0;
        virtual void setVolume(OpaqueMusicPtr* ptr, float v)=0;
        virtual void deletePlayer(OpaqueMusicPtr* ptr)=0;
};
