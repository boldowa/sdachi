extern "C"
{
#include "common/types.h"
#include "common/ReadWrite.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(ReadWrite)
{
	uint8 testarr[8];
	void setup()
	{
		testarr[0] = 0x12;
		testarr[1] = 0x34;
		testarr[2] = 0x56;
	}

	void teardown()
	{
	}
};

TEST(ReadWrite, read16)
{
	LONGS_EQUAL(0x3412, read16(&testarr[0]));
	LONGS_EQUAL(0x5634, read16(&testarr[1]));
}

TEST(ReadWrite, read24)
{
	LONGS_EQUAL(0x563412, read24(&testarr[0]));
}

TEST(ReadWrite, write16)
{
	write16(&testarr[0], 0x1122);
	LONGS_EQUAL(0x1122, read16(&testarr[0]));
}

TEST(ReadWrite, write24)
{
	write24(&testarr[0], 0x334566);
	LONGS_EQUAL(0x334566, read24(&testarr[0]));
}

