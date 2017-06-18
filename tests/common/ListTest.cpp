/**
 * ListTest.cpp
 */
#include <assert.h>
extern "C"
{
#include "common/types.h"
#include "common/List.h"
}

#include "CppUTest/TestHarness.h"

/**
 * Test method
 */
static void* iClone(const void* src)
{
	int* i = (int*)malloc(sizeof(int));
	(*i) = (*(int*)src);
	return i;
}

static void iDelete(void* dst)
{
	free(dst);
}

TEST_GROUP(List)
{
	/* test target */
	List* target;

	void setup()
	{
		target = new_List(iClone, iDelete);
	}

	void teardown()
	{
		delete_List(&target);
	}
};

/**
 * Check object create
 */
TEST(List, new)
{
	CHECK(NULL != target);

	/* initial status check */
	LONGS_EQUAL(0, target->length(target));
	POINTERS_EQUAL(NULL, target->begin(target));
	POINTERS_EQUAL(NULL, target->end(target));
	POINTERS_EQUAL(NULL, target->dequeue(target));
	POINTERS_EQUAL(NULL, target->pop(target));
	POINTERS_EQUAL(NULL, target->index(target,0));
}

/**
 * Check object delete
 */
TEST(List, delete)
{
	delete_List(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

/**
 * Check length method
 */
TEST(List, length)
{
	/* Nothing to do here */
}

/**
 * check push and pop
 */
TEST(List, push_pop)
{
	static const int v1 = 5;
	static const int v2 = 200;
	static const int v3 = 102;
	int* v;
	size_t len;

	/* get default length */
	len = target->length(target);

	/* push check */
	CHECK(target->push(target, iClone(&v1)));
	LONGS_EQUAL(len+1, target->length(target));
	CHECK(target->push(target, iClone(&v2)));
	LONGS_EQUAL(len+2, target->length(target));
	CHECK(target->push(target, iClone(&v3)));
	LONGS_EQUAL(len+3, target->length(target));

	/* check pushed value */
	v = (int*)target->index(target, 0);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v1, *v);
	v = (int*)target->index(target, 1);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v2, *v);
	v = (int*)target->index(target, 2);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v3, *v);

	/* inx over check */
	POINTERS_EQUAL(NULL, target->index(target, 3));

	/* pop check */
	v = (int*)target->pop(target);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(len+2, target->length(target));
	LONGS_EQUAL(v3, *v);
	iDelete(v);
	v = (int*)target->pop(target);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(len+1, target->length(target));
	LONGS_EQUAL(v2, *v);
	iDelete(v);
	v = (int*)target->pop(target);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(len, target->length(target));
	LONGS_EQUAL(v1, *v);
	iDelete(v);

	/* null pop check */
	v = (int*)target->pop(target);
	POINTERS_EQUAL(NULL, v);
	LONGS_EQUAL(len, target->length(target));
}

/**
 * check enqueue and dequeue
 */
TEST(List, enqueu_dequeue)
{
	static const int v1 = 5;
	static const int v2 = 200;
	int* v;
	size_t len;

	/* get default length */
	len = target->length(target);

	/* enqueue check */
	CHECK(target->enqueue(target, iClone(&v1)));
	LONGS_EQUAL(len+1, target->length(target));
	CHECK(target->enqueue(target, iClone(&v2)));
	LONGS_EQUAL(len+2, target->length(target));

	/* check enqueued value */
	v = (int*)target->index(target, 0);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v1, *v);
	v = (int*)target->index(target, 1);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v2, *v);

	/* inx over check */
	POINTERS_EQUAL(NULL, target->index(target, 2));

	/* dequeue check */
	v = (int*)target->dequeue(target);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(len+1, target->length(target));
	LONGS_EQUAL(v1, *v);
	iDelete(v);
	v = (int*)target->dequeue(target);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(len, target->length(target));
	LONGS_EQUAL(v2, *v);
	iDelete(v);

	/* null dequeue check */
	v = (int*)target->dequeue(target);
	POINTERS_EQUAL(NULL, v);
	LONGS_EQUAL(len, target->length(target));
}

/**
 * pushFront method check
 */
TEST(List, pushFront)
{
	static const int v1 = 5;
	static const int v2 = 200;
	int* v;
	size_t len;

	/* get default length */
	len = target->length(target);

	/* pushFront check */
	CHECK(target->pushFront(target, iClone(&v1)));
	LONGS_EQUAL(len+1, target->length(target));
	CHECK(target->pushFront(target, iClone(&v2)));
	LONGS_EQUAL(len+2, target->length(target));

	/* check pushed value */
	v = (int*)target->index(target, 0);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v2, *v);
	v = (int*)target->index(target, 1);
	CHECK_FALSE(NULL == v);
	LONGS_EQUAL(v1, *v);

	/* inx over check */
	POINTERS_EQUAL(NULL, target->index(target, 2));

	CHECK(target->push(target, iClone(&v2)));
	LONGS_EQUAL(len+3, target->length(target));
}

/**
 * index method check
 */
TEST(List, index)
{
	/* nothing to do here */
}

/**
 * begin and end method check
 */
TEST(List, begin_end)
{
	static const int v1 = 5;
	static const int v2 = 200;
	size_t len;
	Iterator* it;

	/* get default length */
	len = target->length(target);

	/* pushFront check */
	CHECK(target->pushFront(target, iClone(&v1)));
	LONGS_EQUAL(len+1, target->length(target));
	it = target->begin(target);
	CHECK(NULL != it);
	POINTERS_EQUAL(target->begin(target), target->end(target));

	CHECK(target->pushFront(target, iClone(&v2)));
	LONGS_EQUAL(len+2, target->length(target));
	POINTERS_EQUAL(it,  target->end(target));
	CHECK(it != target->begin(target));
	CHECK(target->begin(target) != target->end(target));
}

/**
 * clone method check
 */
TEST(List, clone)
{
	static const int v1 = 5;
	static const int v2 = 200;
	size_t len;
	int* v;
	int *cv;
	List* clone;

	/* get default length */
	len = target->length(target);

	/* push check */
	CHECK(target->push(target, iClone(&v1)));
	LONGS_EQUAL(len+1, target->length(target));
	CHECK(target->push(target, iClone(&v2)));
	LONGS_EQUAL(len+2, target->length(target));

	clone = target->clone(target);
	CHECK(clone != target);

	/* check cloned object */
	LONGS_EQUAL(clone->length(clone), target->length(target));
	cv = (int*)clone->index(clone, 0);
	v = (int*)target->index(target, 0);
	LONGS_EQUAL((*cv), (*v));
	cv = (int*)clone->index(clone, 1);
	v = (int*)target->index(target, 1);
	LONGS_EQUAL((*cv), (*v));

	CHECK(target->push(target, iClone(&v2)));
	LONGS_EQUAL(len+3, target->length(target));
	CHECK(clone->length(clone) != target->length(target));

	delete_List(&clone);
}

/* Search test function */
static bool MatchInt(const void* arg1, const void* arg2)
{
	int *search = (int*)arg1;
	int *target = (int*)arg2;

	return (*search == *target);
}

/**
 * search and searchex method check
 */
TEST(List, searchex)
{
	int vvv;
	Iterator* it;

	vvv = 1;
	CHECK(target->push(target, iClone(&vvv)));
	vvv = 2;
	CHECK(target->push(target, iClone(&vvv)));
	vvv = 125;
	CHECK(target->push(target, iClone(&vvv)));
	vvv = 999;
	CHECK(target->push(target, iClone(&vvv)));
	vvv = 8;
	CHECK(target->push(target, iClone(&vvv)));

	/* coverage up check */
	LONGS_EQUAL(2, *((int*)target->index(target, 1)));

	/* no match check */
	vvv = 0;
	POINTERS_EQUAL(NULL, target->search(target, &vvv, MatchInt));
	POINTERS_EQUAL(NULL, target->searchex(target, &vvv, MatchInt, NULL, true));

	/* match */
	vvv = 125;
	CHECK(NULL != target->search(target, &vvv, MatchInt));
	CHECK(NULL != target->searchex(target, &vvv, MatchInt, NULL, false));

	/* match from tail */
	CHECK(NULL != target->searchex(target, &vvv, MatchInt, NULL, true));

	/* multiple search */
	vvv = 2;
	it = target->searchex(target, &vvv, MatchInt, NULL, false);
	CHECK(NULL != it);
	CHECK(NULL == target->searchex(target, &vvv, MatchInt, it->next(it), false));
}
