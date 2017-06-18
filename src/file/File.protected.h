#pragma once
/**
 * File.protected.h
 */

/**
 * File main instance
 */
struct _File_protected {
	/* File path */
	FilePath* filePath;
	/* File pointer */
	FILE *fp;
	/* File mode */
	char* mode;
	/* File size */
	long size;
};

/**
 * detete own members method prototype
 * I recommend it that you move this method in "File.c"
 * if you plan to make the final class.
 */
void delete_File_members(File*);

