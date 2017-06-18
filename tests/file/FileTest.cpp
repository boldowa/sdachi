/**
 * FileTest.cpp
 */
#include <assert.h>
extern "C"
{
#include "common/types.h"
#include "file/File.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(File)
{
	/* test target */
	File* target;

	void setup()
	{
		target = new_File("/dev/null");
	}

	void teardown()
	{
		delete_File(&target);
	}
};

/**
 * Check object create
 */
TEST(File, new)
{
	CHECK(NULL != target);

	/* check initial value */
	LONGS_EQUAL(0, target->size_get(target));
	STRCMP_EQUAL("/dev/null", target->path_get(target));
	STRCMP_EQUAL("/dev/", target->dir_get(target));
	STRCMP_EQUAL("null", target->name_get(target));
	STRCMP_EQUAL("", target->ext_get(target));
}

/**
 * Check object delete
 */
TEST(File, delete)
{
	delete_File(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

/**
 * Check Open method
 */
TEST(File, Open)
{
	/* check that fails. */
	LONGS_EQUAL(FileOpen_NoMode, target->Open(target));
}

/**
 * Check Close method
 */
TEST(File, Close)
{
	/* check execute, and no error occures. */
	target->Close(target);
}

/**
 * Check size_get method
 */
TEST(File, size_get)
{
	/* Nothing to do */
}
