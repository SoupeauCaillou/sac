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
#include <sys/types.h>
#include <ctype.h>
#include <cstdint>
#include <cstring>

#include "base/Log.h"
#include <glm/glm.hpp>



template<typename T>
class CircularBuffer {
        public:
                CircularBuffer(unsigned int pBufferSize);
                ~CircularBuffer();

                unsigned int getBufferSize() const { return bufferSize; }

                unsigned int readDataAvailable() const;

                unsigned int read(T* out, unsigned int size);

                unsigned int writeSpaceAvailable() const;

                unsigned int write(T* in, unsigned int size);

        private:
                T* buffer;
                unsigned int bufferSize;
                unsigned int readPos, writePos;
                unsigned int readLoopCount, writeLoopCount;
};


template<typename T>
CircularBuffer<T>::CircularBuffer(unsigned int pBufferSize) : bufferSize(pBufferSize) {
    buffer = new T[bufferSize];
    readPos = writePos = 0;
    readLoopCount = writeLoopCount = 0;

}

template<typename T>
CircularBuffer<T>::~CircularBuffer() {
    delete[] buffer;
}

template<typename T>
unsigned int CircularBuffer<T>::readDataAvailable() const {
    if (readPos == writePos && readLoopCount == writeLoopCount) {
        return 0;
    } else if (readPos < writePos) {
        return writePos - readPos;
    } else {
        return (bufferSize - readPos) + writePos;
    }
}

template<typename T>
unsigned int CircularBuffer<T>::read(T* out, unsigned int size) {
    unsigned int count = 0;
    LOGW_IF(readDataAvailable() < size, "Inconsistent data available: " << readDataAvailable() << '<' << size);
    if (size == 0)
        return 0;

    do {
        unsigned int amount = glm::min(size - count, bufferSize - readPos);
        memcpy(&out[count], &buffer[readPos], amount * sizeof(T));
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

template<typename T>
unsigned int CircularBuffer<T>::writeSpaceAvailable() const {
    return bufferSize - readDataAvailable();
}

template<typename T>
unsigned int CircularBuffer<T>::write(T* in, unsigned int size) {
    unsigned int count = 0;
    LOGW_IF(writeSpaceAvailable() < size, "Not enough write-space available: " << writeSpaceAvailable() << '<' << size);

    if (size == 0)
        return 0;

    do {
        unsigned int amount = glm::min(size - count, bufferSize - writePos);
        memcpy(&buffer[writePos], &in[count], amount * sizeof(T));
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

