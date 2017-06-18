/**
 * FilePathTest.cpp
 */
#include <assert.h>
extern "C"
{
#include "common/types.h"
#include "file/FilePath.h"
}

#include "CppUTest/TestHarness.h"


#define Path "/foo/bar.biz"
#define Dir  "/foo/"
#define Name "bar"
#define Ext  ".biz"

TEST_GROUP(FilePath_normal)
{
	/* test target */
	FilePath* target;

	void setup()
	{
		target = new_FilePath(Path);
	}

	void teardown()
	{
		delete_FilePath(&target);
	}
};

/**
 * Check object create
 */
TEST(FilePath_normal, new)
{
	CHECK(NULL != target);

	/* check initial value */
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL(Path, target->path_get(target));
}

/**
 * Check object delete
 */
TEST(FilePath_normal, delete)
{
	delete_FilePath(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

/**
 * Check getter methods
 */
TEST(FilePath_normal, getter)
{
	/* Nothing to fo */
}

/**
 * Check setter methods
 */
TEST(FilePath_normal, setter)
{
	/* Set dir */
	CHECK(target->dir_set(target, "/bar/"));
	STRCMP_EQUAL("/bar/",  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL("/bar/" Name Ext, target->path_get(target));
	CHECK(target->dir_set(target, Dir));

	/* Set name */
	CHECK(target->name_set(target, "foo"));
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL("foo", target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL(Dir "foo" Ext, target->path_get(target));
	CHECK(target->name_set(target, Name));

	/* Set ext */
	CHECK(target->ext_set(target, ".z"));
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(".z",  target->ext_get(target));
	STRCMP_EQUAL(Dir Name ".z", target->path_get(target));
	CHECK(target->ext_set(target, Ext));

	/* Set path */
	CHECK(target->path_set(target, "../aaa/bbb/cc.ddddd"));
	STRCMP_EQUAL("../aaa/bbb/",  target->dir_get(target));
	STRCMP_EQUAL("cc", target->name_get(target));
	STRCMP_EQUAL(".ddddd",  target->ext_get(target));
	STRCMP_EQUAL("../aaa/bbb/cc.ddddd", target->path_get(target));
	CHECK(target->ext_set(target, Ext));

}

TEST(FilePath_normal, Clone)
{
	FilePath* clone;

	/* clone target generate check */
	CHECK(target->path_set(target, "/xxx/yyy/zzz.txt"));
	clone = target->Clone(target);
	CHECK(NULL != clone);

	/* check cloned target parameter */
	STRCMP_EQUAL(clone->dir_get(clone), target->dir_get(target));
	STRCMP_EQUAL(clone->name_get(clone), target->name_get(target));
	STRCMP_EQUAL(clone->ext_get(clone), target->ext_get(target));
	STRCMP_EQUAL(clone->path_get(clone), target->path_get(target));

	/* check different value can be set */
	CHECK(target->path_set(target, Path));
	CHECK(clone->path_set(clone, "/this/is/test.clone"));
	STRCMP_EQUAL(Path, target->path_get(target));
	STRCMP_EQUAL("/this/is/test.clone", clone->path_get(clone));

	delete_FilePath(&clone);
}

TEST_GROUP(FilePath_semiNormal)
{
	/* parameter */
	const char* CurrentDir = "./";

	/* test target */
	FilePath* target;

	void setup()
	{
		target = new_FilePath(CurrentDir);
	}

	void teardown()
	{
		delete_FilePath(&target);
	}
};

TEST(FilePath_semiNormal, new)
{
	FilePath* tgtNull;

	CHECK(NULL != target);

	/* check initial value */
	STRCMP_EQUAL(CurrentDir,  target->dir_get(target));
	STRCMP_EQUAL("", target->name_get(target));
	STRCMP_EQUAL("",  target->ext_get(target));
	STRCMP_EQUAL(CurrentDir, target->path_get(target));

	/* check null initial */
	tgtNull = new_FilePath(NULL);
	STRCMP_EQUAL(NULL,  tgtNull->dir_get(tgtNull));
	STRCMP_EQUAL(NULL, tgtNull->name_get(tgtNull));
	STRCMP_EQUAL(NULL,  tgtNull->ext_get(tgtNull));
	STRCMP_EQUAL(NULL, tgtNull->path_get(tgtNull));
	delete_FilePath(&tgtNull);
}

TEST(FilePath_semiNormal, delete)
{
	delete_FilePath(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

TEST(FilePath_semiNormal, set)
{
	/* Set die-less */
	CHECK(target->path_set(target, "xxx.yyy"));
	STRCMP_EQUAL("", target->dir_get(target));
	STRCMP_EQUAL("xxx", target->name_get(target));
	STRCMP_EQUAL(".yyy", target->ext_get(target));
	STRCMP_EQUAL("xxx.yyy", target->path_get(target));

	/* Set name-less */
	CHECK(target->path_set(target, "/xxx/.yyy"));
	STRCMP_EQUAL("/xxx/", target->dir_get(target));
	STRCMP_EQUAL("", target->name_get(target));
	STRCMP_EQUAL(".yyy", target->ext_get(target));
	STRCMP_EQUAL("/xxx/.yyy", target->path_get(target));

	/* Set ext-less */
	CHECK(target->path_set(target, "/xxx/yyy"));
	STRCMP_EQUAL("/xxx/", target->dir_get(target));
	STRCMP_EQUAL("yyy", target->name_get(target));
	STRCMP_EQUAL("", target->ext_get(target));
	STRCMP_EQUAL("/xxx/yyy", target->path_get(target));

	/* Set dir-only */
	CHECK(target->path_set(target, "/xxx/yyy/"));
	STRCMP_EQUAL("/xxx/yyy/", target->dir_get(target));
	STRCMP_EQUAL("", target->name_get(target));
	STRCMP_EQUAL("", target->ext_get(target));
	STRCMP_EQUAL("/xxx/yyy/", target->path_get(target));

	/* Set name-only */
	CHECK(target->path_set(target, "xxx"));
	STRCMP_EQUAL("", target->dir_get(target));
	STRCMP_EQUAL("xxx", target->name_get(target));
	STRCMP_EQUAL("", target->ext_get(target));
	STRCMP_EQUAL("xxx", target->path_get(target));

	/* Set ext-only */
	CHECK(target->path_set(target, ".yyy"));
	STRCMP_EQUAL("", target->dir_get(target));
	STRCMP_EQUAL("", target->name_get(target));
	STRCMP_EQUAL(".yyy", target->ext_get(target));
	STRCMP_EQUAL(".yyy", target->path_get(target));


	CHECK(target->path_set(target, Path));


	/* Check path overflow */
	CHECK_FALSE(target->path_set(target, "/aaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/bbbbbbbb.cc"));
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL(Path, target->path_get(target));

	/* Check dir overflow */
	CHECK_FALSE(target->dir_set(target, "/aaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/xxxx/"));
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL(Path, target->path_get(target));

	/* Check name overflow */
	CHECK_FALSE(target->name_set(target, "x12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"));
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL(Path, target->path_get(target));

	/* Check ext overflow */
	CHECK_FALSE(target->ext_set(target, ".x12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"));
	STRCMP_EQUAL(Dir,  target->dir_get(target));
	STRCMP_EQUAL(Name, target->name_get(target));
	STRCMP_EQUAL(Ext,  target->ext_get(target));
	STRCMP_EQUAL(Path, target->path_get(target));

	/* Check limit */
	CHECK(target->ext_set(target, ".x1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));
	STRCMP_EQUAL(".x1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", target->ext_get(target));
}

