/**
 * Iterator.c
 */
#include "common/types.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "common/Iterator.h"
#include "Iterator.protected.h"

static Iterator* prev(Iterator*);
static Iterator* next(Iterator*);
static void* data(Iterator*);
static bool insert(Iterator*, void*);
static void itremove(Iterator*, bool);

/*--------------- Constructor / Destructor ---------------*/

/**
 * @brief Create Iterator object
 *
 * @return the pointer of object
 */
Iterator* new_Iterator(void* d)
{
	Iterator* self;
	Iterator_protected* pro;

	/* make objects */
	self = malloc(sizeof(Iterator));
	pro = malloc(sizeof(Iterator_protected));

	/* check whether object creatin succeeded */
	assert(pro);
	assert(self);

	/*--- set protected member ---*/
	pro->data = d;
	pro->prev = NULL;
	pro->next = NULL;

	/*--- set public member ---*/
	self->data = data;
	self->prev = prev;
	self->next = next;
	self->insert = insert;
	self->remove = itremove;

	/* init Iterator object */
	self->pro = pro;
	return self;
}

/**
 * @brief Delete Iterator object
 *
 * @param the pointer of object
 */
void delete_Iterator(Iterator** self)
{
	/* This is the template that default destractor. */
	assert(self);
	if(NULL == (*self)) return;
	free((*self)->pro);
	free(*self);
	(*self) = NULL;
}

static Iterator* prev(Iterator* self)
{
	assert(self);
	return self->pro->prev;
}

static Iterator* next(Iterator* self)
{
	assert(self);
	return self->pro->next;
}

static void* data(Iterator* self)
{
	assert(self);
	return self->pro->data;
}

static bool insert(Iterator* self, void* data)
{
	Iterator *itPrev;
	Iterator *itNew;

	itNew = new_Iterator(data);
	assert(itNew);
	itPrev = self->prev(self);

	if(NULL != itPrev)
	{
		itPrev->pro->next = itNew;
	}

	itNew->pro->prev = self->pro->prev;
	itNew->pro->next = self;
	self->pro->prev = itNew;

	return true;
}

static void itremove(Iterator* self, bool isKeepData)
{
	/* TODO: Implement */
	return;
}
