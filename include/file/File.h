#pragma once
/**
 * File.h
 */

/**
 * File open erroes
 */
typedef enum {
	FileOpen_NoError,
	FileOpen_AlreadyOpen,
	FileOpen_NoMode,
	FileOpen_CantAccess
} E_FileOpen;

/**
 * public accessor
 */
typedef struct _File File;
typedef struct _File_protected File_protected;
struct _File {
	const char* (*path_get)(File*);
	const char* (*dir_get)(File*);
	const char* (*name_get)(File*);
	const char* (*ext_get)(File*);
	E_FileOpen (*Open)(File*);
	void (*Close)(File*);
	long (*size_get)(File*);
	/* protected members */
	File_protected* pro;
};

/**
 * Constructor
 */
File* new_File(const char*);

/**
 * Destractor
 */
void delete_File(File**);

