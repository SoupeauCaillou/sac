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

#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>
#include <cstdint>

class CircularBuffer {
	public:
		CircularBuffer(unsigned int pBufferSize);
		~CircularBuffer();

		unsigned int getBufferSize() const { return bufferSize; }

		// byte count available for reading
		unsigned int readDataAvailable() const;

		unsigned int read(int8_t* out, unsigned int size);

		unsigned int writeSpaceAvailable() const;

		unsigned int write(int8_t* in, unsigned int size);

	private:
		int8_t* buffer;
		unsigned int bufferSize;
		unsigned int readPos, writePos;
		unsigned int readLoopCount, writeLoopCount;
};

