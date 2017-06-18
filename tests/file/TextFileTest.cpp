/**
 * TextFileTest.cpp
 */
#include <assert.h>
extern "C"
{
#include "common/types.h"
#include "file/File.h"
#include "file/TextFile.h"
}

#define TestRoot "testdata/file/"
#define ReadFile "test.txt"
#define WriteFile "wtest.txt"

#define WriteLine1 "12345"
#define WriteLine2 "12 = %d"

#include "CppUTest/TestHarness.h"

TEST_GROUP(TextFile)
{
	/* test target */
	TextFile* target;

	void setup()
	{
		target = new_TextFile(TestRoot ReadFile);
	}

	void teardown()
	{
		delete_TextFile(&target);
	}
};

/**
 * Check object create
 */
TEST(TextFile, new)
{
	CHECK(NULL != target);

	/* check initial value */
	LONGS_EQUAL(0, target->super.size_get(&target->super));
	STRCMP_EQUAL(TestRoot ReadFile, target->super.path_get(&target->super));
	STRCMP_EQUAL(TestRoot, target->super.dir_get(&target->super));
	STRCMP_EQUAL("test", target->super.name_get(&target->super));
	STRCMP_EQUAL(".txt", target->super.ext_get(&target->super));
}

/**
 * Check object delete
 */
TEST(TextFile, delete)
{
	delete_TextFile(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

/**
 * Check Open method
 */
TEST(TextFile, Open)
{
	/* check to success */
	LONGS_EQUAL(FileOpen_NoError,target->Open(target));

	/* check size */
	LONGS_EQUAL(302, target->super.size_get(&target->super));

	/* check to fail */
	LONGS_EQUAL(FileOpen_AlreadyOpen, target->Open(target));
}

/**
 * Check Open2 method
 */
TEST(TextFile, Open2)
{
	/* Nothing to do here */
}

/**
 * Check size_get method
 */
TEST(TextFile, size_get)
{
	/* Nothing to do here */
}

/**
 * Check row_get method
 */
TEST(TextFile, row_get)
{
	/* Nothing to do here */
}


/**
 * Check GetLine method
 */
TEST(TextFile, GetBuffer)
{
	const char* line;
	LONGS_EQUAL(FileOpen_NoError,target->Open(target));

	/* line 0 */
	LONGS_EQUAL(0, target->row_get(target));

	/* line 1 */
	line = target->GetLine(target);
	CHECK(NULL != line);
	LONGS_EQUAL(1, target->row_get(target));
	STRCMP_EQUAL("Testing... golo", line);

	/* line 2 */
	line = target->GetLine(target);
	CHECK(NULL != line);
	LONGS_EQUAL(2, target->row_get(target));
	STRCMP_EQUAL("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", line);

	/* line 3 */
	line = target->GetLine(target);
	CHECK(NULL != line);
	LONGS_EQUAL(3, target->row_get(target));
	STRCMP_EQUAL("xxx", line);

	/* line 4 */
	line = target->GetLine(target);
	CHECK(NULL != line);
	LONGS_EQUAL(4, target->row_get(target));
	STRCMP_EQUAL("", line);

	/* line end */
	line = target->GetLine(target);
	LONGS_EQUAL(4, target->row_get(target));
	POINTERS_EQUAL(NULL, line);
}

TEST_GROUP(TextFile2)
{
	/* test target */
	TextFile* target;
	TextFile* reader;

	const char* line1 = "12345\n";
	const char* line2 = "12 = %d\n";

	void setup()
	{
		target = new_TextFile(TestRoot WriteFile);
		reader = new_TextFile(TestRoot WriteFile);

		remove(TestRoot WriteFile);
	}

	void teardown()
	{
		delete_TextFile(&target);
		delete_TextFile(&reader);
		remove(TestRoot WriteFile);
	}
};

/**
 * Check Printf method
 */
TEST(TextFile2, Printf)
{
	const char* line;

	/* check file not exists */
	LONGS_EQUAL(FileOpen_CantAccess, reader->Open(target));

	/* Create new file */
	LONGS_EQUAL(FileOpen_NoError, target->Open2(target, "w"));

	/* puts */
	target->Printf(target, WriteLine1 "\n");
	target->Printf(target, WriteLine2, 12);

	target->super.Close(&target->super);

	/* check file read */
	LONGS_EQUAL(FileOpen_NoError, reader->Open(reader));

	line = reader->GetLine(reader);
	CHECK(NULL != line);
	STRCMP_EQUAL(WriteLine1, line);

	line = reader->GetLine(reader);
	CHECK(NULL != line);
	STRCMP_EQUAL("12 = 12", line);

	reader->super.Close(&reader->super);
}
