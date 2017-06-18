#pragma once
/**
 * TextFile.h
 */

/**
 * public accessor
 */
typedef struct _TextFile TextFile;
typedef struct _TextFile_protected TextFile_protected;
struct _TextFile {
	/* inherited */
	File super;
	/* member method */
	E_FileOpen (*Open)(TextFile*);
	E_FileOpen (*Open2)(TextFile*, const char*);
	uint (*row_get)(TextFile*);
	const char* (*GetLine)(TextFile*);
	void (*Printf)(TextFile*, const char*, ...);
	/* protected members */
	TextFile_protected* pro;
};

/**
 * Constructor
 */
TextFile* new_TextFile(const char*);

/**
 * Destractor
 */
void delete_TextFile(TextFile**);

