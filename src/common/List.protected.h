#pragma once
/**
 * List.protected.h
 */

/**
 * List main instance
 *   TODO : Fix it
 */
struct _List_protected {
	Iterator* top;
	Iterator* tail;
	size_t length;
	ListDataCloner_t cloner;
	ListDataDeleter_t deleter;
};

/**
 * detete own members method prototype
 * I recommend it that you move this method in "List.c"
 * if you plan to make the final class.
 */
void delete_List_members(List*);

