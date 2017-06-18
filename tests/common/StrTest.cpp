extern "C"
{
#include "common/types.h"
#include "common/Str.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(Str)
{
	const char* ST = "('e' *)< Hello, my son, and my daughter.";
	const char* ST_REP = "('e' *)< Hello, your son, and your daughter.";
	const char* ST_REP2 = "('x' *)< Hxllo, your son, and your daughtxr.";

	char* tgt;

	void setup()
	{
		tgt = (char*)calloc(strlen(ST)+1, sizeof(char));
		strcpy(tgt, ST);
	}

	void teardown()
	{
		free(tgt);
	}
};

TEST(Str, Str_concat)
{
	/* Valid value check */
	char* x = Str_concat("aaa", "bbb");
	STRCMP_EQUAL("aaabbb", x);

	/* Invalid value check */
	char* y = Str_concat(NULL, "bbb");
	POINTERS_EQUAL(NULL, y);
	y = Str_concat("aaa", NULL);
	POINTERS_EQUAL(NULL, y);

	free(x);
}

TEST(Str, Str_replace)
{
	/* Invalid value check */
	char* v;
	v = Str_replace(NULL, "y", tgt);
	POINTERS_EQUAL(NULL, v);
	v = Str_replace("x", NULL, tgt);
	POINTERS_EQUAL(NULL, v);
	v = Str_replace("x", "y", NULL);
	POINTERS_EQUAL(NULL, v);
	v = Str_replace("", "y", tgt);
	POINTERS_EQUAL(NULL, v);

	/* Valid value check(Match) */
	char* x = Str_replace("my", "your", tgt);
	STRCMP_EQUAL(ST_REP, x);

	/* Valid value check(Unmatch) */
	char* y = Str_replace("FOO", "BAR", tgt);
	STRCMP_EQUAL(ST, y);

	/* re-use return value */
	char* z = Str_replace("e", "x", x);
	STRCMP_EQUAL(ST_REP2, z);

	/* clean */
	free(x);
	free(y);
	free(z);
}

TEST(Str, Str_copy)
{
	char* copy;

	copy = Str_copy(ST);

	LONGS_EQUAL(strlen(ST), strlen(copy));
	STRCMP_EQUAL(ST, copy);

	POINTERS_EQUAL(NULL, Str_copy(NULL));

	free(copy);
}

TEST(Str, Str_toupper)
{
	char* work;

	work = Str_copy("ExPoRt");
	Str_toupper(work);

	STRCMP_EQUAL("EXPORT", work);

	free(work);

	Str_toupper(NULL);	/* check that don't fall */
}

TEST(Str, Str_tolower)
{
	char* work;

	work = Str_copy("ExPoRt");
	Str_tolower(work);

	STRCMP_EQUAL("export", work);

	free(work);

	Str_tolower(NULL);	/* check that don't fall */
}
