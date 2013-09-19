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



#include "CircularBuffer.h"
#include <cstring>
#include "base/Log.h"
#include <glm/glm.hpp>

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
	LOGW_IF(readDataAvailable() < size, "Inconsistent data available: " << readDataAvailable() << '<' << size);
	if (size == 0)
		return 0;

	do {
		unsigned int amount = glm::min(size - count, bufferSize - readPos);
		memcpy(&out[count], &buffer[readPos], amount);
		count += amount;
		readPos += amount;
		LOGW_IF (readPos > bufferSize, "Invalid readPos value: " << readPos << '>' << bufferSize);
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
	LOGW_IF(writeSpaceAvailable() < size, "Not enough write-space available: " << writeSpaceAvailable() << '<' << size);

	if (size == 0)
		return 0;

	do {
		unsigned int amount = glm::min(size - count, bufferSize - writePos);
		memcpy(&buffer[writePos], &in[count], amount);
		count += amount;
		writePos  += amount;
		LOGW_IF (writePos > bufferSize, "Incoherent writePos " << writePos << " is > " << bufferSize);
		if (writePos == bufferSize) {
			writePos = 0;
			writeLoopCount++;
		}
	} while (count < size);
	return count;
}
