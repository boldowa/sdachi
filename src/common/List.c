/**
 * List.c
 */
#include "common/types.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "common/Iterator.h"
#include "Iterator.protected.h"
#include "common/List.h"

/* this header isn't read from anything other */
/* than inherited object.                     */ 
#include "List.protected.h"

/* prototypes */
static size_t Length(List*);
static bool PushFront(List*, void*);
static bool PushBack(List*, void*);
static void* PopFront(List*);
static void* PopBack(List*);
static void* Index(List*, const size_t);
static Iterator* Begin(List*);
static Iterator* End(List*);
static Iterator* Search(List*, const void*, ListSearcher_t);
static Iterator* SearchEx(List*, const void*, ListSearcher_t, Iterator*, const bool);
static List* Clone(List*);


/*--------------- Constructor / Destructor ---------------*/

/**
 * @brief Create List object
 *
 * @return the pointer of object
 */
List* new_List(ListDataCloner_t cloner, ListDataDeleter_t deleter)
{
	List* self;
	List_protected* pro;

	/* make objects */
	self = malloc(sizeof(List));
	pro = malloc(sizeof(List_protected));

	/* check whether object creatin succeeded */
	assert(pro);
	assert(self);

	/*--- set protected member ---*/
	pro->cloner = cloner;
	pro->deleter = deleter;
	pro->length = 0;
	pro->top = NULL;
	pro->tail = NULL;

	/*--- set public member ---*/
	self->length = Length;
	self->enqueue = PushBack;
	self->dequeue = PopFront;
	self->push = PushBack;
	self->pushFront = PushFront;
	self->pop = PopBack;
	self->popFront = PopFront;
	self->index = Index;
	self->begin = Begin;
	self->end = End;
	self->search = Search;
	self->searchex = SearchEx;
	self->clone = Clone;

	/* init List object */
	self->pro = pro;
	return self;
}

/**
 * @brief delete own member variables
 *
 * @param the pointer of object
 */
void delete_List_members(List* self)
{
	List_protected* lp;
	Iterator* it;
	Iterator* nx;
	/* delete protected members */
	lp = self->pro;
	it = lp->top;
	while(NULL != it)
	{
		nx = it->next(it);
		lp->deleter(it->pro->data);
		delete_Iterator(&it);
		it = nx;
	}
}

/**
 * @brief Delete List object
 *
 * @param the pointer of object
 */
void delete_List(List** self)
{
	/* This is the template that default destractor. */
	assert(self);
	if(NULL == (*self)) return;
	delete_List_members(*self);
	free((*self)->pro);
	free(*self);
	(*self) = NULL;
}


/*--------------- internal methods ---------------*/

static size_t Length(List* self)
{
	assert(self);
	return self->pro->length;
}

static bool PushFront(List* self, void* data)
{
	Iterator* it;
	Iterator_protected* itp;
	List_protected* sep;

	assert(self);
	it = new_Iterator(data);
	itp = it->pro;
	sep = self->pro;

	itp->next = sep->top;
	if(NULL != sep->top)
	{
		sep->top->pro->prev = it;
	}
	sep->top = it;
	if(0 == sep->length)
	{
		sep->tail = sep->top;
	}
	sep->length++;
	return true;
}

static bool PushBack(List* self, void* data)
{
	Iterator* it;
	Iterator_protected* itp;
	List_protected* sep;

	assert(self);
	it = new_Iterator(data);
	itp = it->pro;
	sep = self->pro;

	itp->prev = sep->tail;
	if(NULL != sep->tail)
	{
		sep->tail->pro->next = it;
	}
	sep->tail = it;
	if(0 == sep->length)
	{
		sep->top = sep->tail;
	}
	sep->length++;
	return true;
}

static void* PopFront(List* self)
{
	Iterator* it;
	Iterator_protected* itp;
	List_protected* sep;
	void* data;

	assert(self);
	sep = self->pro;
	if(NULL == sep->top)
	{
		return NULL;
	}

	/* get data */
	itp = sep->top->pro;
	data = itp->data;

	/* delete iterator */
	it = sep->top->pro->next;
	delete_Iterator(&sep->top);

	/* list update */
	if(NULL != it)
	{
		it->prev = NULL;
	}
	sep->top = it;
	sep->length--;
	if(0 == sep->length)
	{
		sep->tail = sep->top;
	}

	return data;
}

static void* PopBack(List* self)
{
	Iterator* it;
	Iterator_protected* itp;
	List_protected* sep;
	void* data;

	assert(self);
	sep = self->pro;
	if(NULL == sep->tail)
	{
		return NULL;
	}

	/* get data */
	itp = sep->tail->pro;
	data = itp->data;

	/* delete iterator */
	it = sep->tail->pro->prev;
	delete_Iterator(&sep->tail);

	/* list update */
	if(NULL != it)
	{
		it->next = NULL;
	}
	sep->tail = it;
	sep->length--;
	if(0 == sep->length)
	{
		sep->top = sep->tail;
	}

	return data;
}
static void* Index(List* self, const size_t inx)
{
	Iterator_protected* itp;
	List_protected* sep;
	size_t i;

	assert(self);

	/* valid value check */
	if(0 > inx) return NULL;
	if(self->pro->length <= inx) return NULL;

	/* list empty check */
	sep = self->pro;
	if(NULL == sep->top) return NULL;

	sep = self->pro;

	/* seek from top */
	if(sep->length/2 > inx)
	{
		itp = sep->top->pro;
		for(i = 0; i != inx; i++)
		{
			itp = itp->next->pro; 
		}
		return itp->data;
	}

	/* seek from tail */
	itp = sep->tail->pro;
	for(i = sep->length-1; i != inx; i--)
	{
		itp = itp->prev->pro;
	}
	return itp->data;
}

static Iterator* Begin(List* self)
{
	assert(self);
	return self->pro->top;
}
static Iterator* End(List* self)
{
	assert(self);
	return self->pro->tail;
}

static Iterator* SearchEx(List* self, const void* data, ListSearcher_t match, Iterator* beg, const bool isPrevSearch)
{
	Iterator* it;
	Iterator_protected* itp;
	assert(self);

	it = self->pro->top;
	if(true == isPrevSearch)
	{
		it = self->pro->tail;
	}
	if(NULL != beg)
	{
		it = beg;
	}

	if(false == isPrevSearch)
	{
		while(NULL != it)
		{
			itp = it->pro;
			if(true == match(data, itp->data))
			{
				return it;
			}
			it = itp->next;
		}
		return NULL;
	}

	while(NULL != it)
	{
		itp = it->pro;
		if(true == match(data, itp->data))
		{
			return it;
		}
		it = itp->prev;
	}
	return NULL;
}

static Iterator* Search(List* self, const void* data, ListSearcher_t match)
{
	/* search from top */
	return SearchEx(self, data, match, NULL, false);
}

static List* Clone(List* src)
{
	List* dest;
	List_protected* srp;
	Iterator* sit;
	void* data;

	assert(src);

	srp = src->pro;
	sit = srp->top;
	dest = new_List(srp->cloner, srp->deleter);
	assert(dest);

	while(sit != NULL)
	{
		data = srp->cloner(sit->data(sit));
		assert(data);
		dest->push(dest, data);
		sit = sit->next(sit);
	}

	return dest;
}

