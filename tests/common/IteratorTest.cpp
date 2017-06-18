/**
 * IteratorTest.cpp
 */
#include <assert.h>
extern "C"
{
#include "common/types.h"
#include "common/Iterator.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(Iterator)
{
	/* test target */
	Iterator* target;
	int* itest;

	void setup()
	{
		itest = (int*)malloc(sizeof(int));
		assert(itest);
		*itest = 30;
		target = new_Iterator(itest);
	}

	void teardown()
	{
		delete_Iterator(&target);
		free(itest);
	}
};

/**
 * Check object create
 */
TEST(Iterator, new)
{
	CHECK(NULL != target);
}

/**
 * Check object delete
 */
TEST(Iterator, delete)
{
	delete_Iterator(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

/**
 * Check prev method
 */
TEST(Iterator, prev)
{
	POINTERS_EQUAL(NULL, target->prev(target));
}

/**
 * Check next method
 */
TEST(Iterator, next)
{
	POINTERS_EQUAL(NULL, target->next(target));
}

/**
 * Check data method
 */
TEST(Iterator, data)
{
	int *i;
	i = (int*)target->data(target);
	CHECK(NULL != i);
	LONGS_EQUAL(30, *i);
}
