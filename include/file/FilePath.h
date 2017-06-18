#pragma once
/**
 * FilePath.h
 */

/**
 * public accessor
 */
typedef struct _FilePath FilePath;
typedef struct _FilePath_private FilePath_private;
struct _FilePath {
	/* getter */
	const char* (*path_get)(FilePath*);
	const char* (*dir_get)(FilePath*);
	const char* (*name_get)(FilePath*);
	const char* (*ext_get)(FilePath*);
	/* setter */
	bool (*path_set)(FilePath*, const char*);
	bool (*dir_set)(FilePath*, const char*);
	bool (*name_set)(FilePath*, const char*);
	bool (*ext_set)(FilePath*, const char*);
	/* clone method */
	FilePath* (*Clone)(FilePath*);
	/* private members */
	FilePath_private* pri;
};

/**
 * Constructor
 */
FilePath* new_FilePath(const char*);

/**
 * Destractor
 */
void delete_FilePath(FilePath**);

