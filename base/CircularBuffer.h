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

#include <ctype.h>
#include <sys/types.h>

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

