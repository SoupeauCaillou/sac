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

