#include <UnitTest++.h>

#include "base/CircularBuffer.h"


TEST(InitialDataAndSpaceAvailable)
{
	CircularBuffer buffer(10);
	CHECK_EQUAL((unsigned)0, buffer.readDataAvailable());
	CHECK_EQUAL((unsigned)10, buffer.writeSpaceAvailable());
}

TEST(DataAndSpaceAvailableAfterPartialWrite)
{
	CircularBuffer buffer(10);
	int8_t i[] = {0, 1, 2, 3, 4};
	buffer.write(i, 5);
	CHECK_EQUAL((unsigned)5, buffer.readDataAvailable());
	CHECK_EQUAL((unsigned)5, buffer.writeSpaceAvailable());
}

TEST(DataAndSpaceAvailableAfterTwoWrite)
{
	CircularBuffer buffer(10);
	int8_t i[] = {0, 1, 2, 3, 4};
	buffer.write(i, 5);
	buffer.write(i, 5);
	CHECK_EQUAL((unsigned)10, buffer.readDataAvailable());
	CHECK_EQUAL((unsigned)0, buffer.writeSpaceAvailable());
}

TEST(DataAndSpaceAvailableAfterWriteLoop)
{
	CircularBuffer buffer(10);
	int8_t i[] = {0, 1, 2, 3, 4, 5, 6};
	int8_t o[] = {0, 0, 0, 0, 0, 0, 0, 0};
	// R X X X X W - - - -
	buffer.write(i, 5);
	// - - - - R W - - - -
	CHECK_EQUAL((unsigned)4, buffer.read(o, 4));
	for(int idx=0; idx<4; idx++) {
		CHECK_EQUAL(i[idx], o[idx]);
	}
	CHECK_EQUAL((unsigned)1, buffer.readDataAvailable());
	// X X W - R X X X X X
	buffer.write(i, 7);
	CHECK_EQUAL((unsigned)8, buffer.readDataAvailable());
	CHECK_EQUAL((unsigned)8, buffer.read(o, 8));
	CHECK_EQUAL((unsigned)0, buffer.readDataAvailable());
}
