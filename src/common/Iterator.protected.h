#pragma once
/**
 * Iterator.protected.h
 */

/**
 * Iterator main instance
 */
struct _Iterator_protected{
	Iterator *prev;
	Iterator *next;
	void * data;
};

