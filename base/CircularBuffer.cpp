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
#include "CircularBuffer.h"
#include "base/MathUtil.h"
#include <cassert>
#include <cstring>

CircularBuffer::CircularBuffer(unsigned int pBufferSize) : bufferSize(pBufferSize) {
	buffer = new int8_t[bufferSize];
	readPos = writePos = 0;
	readLoopCount = writeLoopCount = 0;

}

CircularBuffer::~CircularBuffer() {
	delete[] buffer;
}

// byte count available for reading
unsigned int CircularBuffer::readDataAvailable() const {
	if (readPos == writePos && readLoopCount == writeLoopCount) {
		return 0;
	} else if (readPos < writePos) {
		return writePos - readPos;
	} else {
		return (bufferSize - readPos) + writePos;
	}
}

unsigned int CircularBuffer::read(int8_t* out, unsigned int size) {
	unsigned int count = 0;
	assert(readDataAvailable() >= size);
	if (size == 0)
		return 0;
	
	do {
		unsigned int amount = MathUtil::Min(size - count, bufferSize - readPos);
		memcpy(&out[count], &buffer[readPos], amount);
		count += amount;
		readPos += amount;
		assert (readPos <= bufferSize);
		if (readPos == bufferSize) {
			readPos = 0;
			readLoopCount++;
		}	
	} while (count < size);

	return count;
}

unsigned int CircularBuffer::writeSpaceAvailable() const {
	return bufferSize - readDataAvailable();
}

unsigned int CircularBuffer::write(int8_t* in, unsigned int size) {
	unsigned int count = 0;
	assert(writeSpaceAvailable() >= size);
	
	if (size == 0)
		return 0;

	do {
		unsigned int amount = MathUtil::Min(size - count, bufferSize - writePos);
		memcpy(&buffer[writePos], &in[count], amount);
		count += amount;
		writePos  += amount;
		assert (writePos <= bufferSize);
		if (writePos == bufferSize) {
			writePos = 0;
			writeLoopCount++;
		}	
	} while (count < size);
	return count;
}
