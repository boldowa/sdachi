#pragma once
/**
 * List.h
 */

#include "common/Iterator.h"

typedef void* (*ListDataCloner_t)(const void*);
typedef void (*ListDataDeleter_t)(void*);
typedef bool (*ListSearcher_t)(const void*, const void*);

/**
 * public accessor
 */
typedef struct _List List;
typedef struct _List_protected List_protected;
struct _List {
	size_t (*length)(List*);
	bool (*enqueue)(List*, void* data);
	bool (*push)(List*, void*);
	bool (*pushFront)(List*, void*);
	void* (*dequeue)(List*);
	void* (*pop)(List*);
	void* (*popFront)(List*);
	void* (*index)(List*, const size_t);
	Iterator* (*begin)(List*);
	Iterator* (*end)(List*);
	Iterator* (*search)(List*, const void*, ListSearcher_t);
	Iterator* (*searchex)(List*, const void*, ListSearcher_t, Iterator*, const bool);
	List* (*clone)(List*);
	/* protected members */
	List_protected* pro;
};

/**
 * Constructor
 */
List* new_List(ListDataCloner_t, ListDataDeleter_t);

/**
 * Destractor
 */
void delete_List(List**);

