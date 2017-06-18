/**
 * File.c
 */
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include "common/types.h"
#include "file/FilePath.h"
#include "file/File.h"

/* this header isn't read from anything other */
/* than inherited object.                     */ 
#include "File.protected.h"

/* prototypes */
static void Close(File*);
static E_FileOpen Open(File*);
static long size_get(File*);
static const char* path_get(File*);
static const char* dir_get(File*);
static const char* name_get(File*);
static const char* ext_get(File*);


/*--------------- Constructor / Destructor ---------------*/

/**
 * @brief Create File object
 *
 * @return the pointer of object
 */
File* new_File(const char* path)
{
	File* self;
	File_protected* pro;

	/* make objects */
	pro = malloc(sizeof(File_protected));
	self = malloc(sizeof(File));

	/* check whether object creatin succeeded */
	assert(pro);
	assert(self);

	/*--- set protected member ---*/
	pro->filePath = new_FilePath(path);
	assert(pro->filePath);
	pro->fp = NULL;
	pro->mode = NULL;
	pro->size = 0;

	/*--- set public member ---*/
	self->path_get = path_get;
	self->dir_get = dir_get;
	self->name_get = name_get;
	self->ext_get = ext_get;
	self->Close = Close;
	self->Open = Open;
	self->size_get = size_get;

	/* init File object */
	self->pro = pro;
	return self;
}

/**
 * @brief delete own member variables
 *
 * @param the pointer of object
 */
void delete_File_members(File* self)
{
	/* delete protected members */
	free(self->pro->mode);
	delete_FilePath(&self->pro->filePath);
	if(NULL != self->pro->fp) fclose(self->pro->fp);
	free(self->pro);
}

/**
 * @brief Delete File object
 *
 * @param the pointer of object
 */
void delete_File(File** self)
{
	/* This is the template that default destractor. */
	assert(self);
	if(NULL == (*self)) return;
	delete_File_members(*self);
	free(*self);
	(*self) = NULL;
}


/*--------------- internal methods ---------------*/

static long size_get(File* self)
{
	assert(self);
	return self->pro->size;
}
static const char* path_get(File* self)
{
	FilePath* fp;

	assert(self);
	fp = self->pro->filePath;
	return fp->path_get(fp);
}

static const char* dir_get(File* self)
{
	FilePath* fp;

	assert(self);
	fp = self->pro->filePath;
	return fp->dir_get(fp);
}

static const char* name_get(File* self)
{
	FilePath* fp;

	assert(self);
	fp = self->pro->filePath;
	return fp->name_get(fp);
}

static const char* ext_get(File* self)
{
	FilePath* fp;

	assert(self);
	fp = self->pro->filePath;
	return fp->ext_get(fp);
}

static void Close(File* self)
{
	assert(self);
	if(NULL != self->pro->fp)
	{
		fclose(self->pro->fp);
		self->pro->fp = NULL;
	}
}

static long getfilesize(FilePath* fp)
{
#if defined(WIN32) || defined(_WIN32)
	HANDLE hFile;
	DWORD dwSize;

	hFile = CreateFile(fp->path_get(fp),
		GENERIC_READ, FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(INVALID_HANDLE_VALUE == hFile) return 0;

	dwSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return (long)dwSize;
#else
	struct stat st;
	if(0 == stat(fp->path_get(fp), &st))
	{
		return st.st_size;
	}
	return 0;
#endif
}

static E_FileOpen Open(File* self)
{
	File_protected* f;
	FilePath* fpath;

	assert(self);
	f = self->pro;
	if(NULL != f->fp) return FileOpen_AlreadyOpen;
	if(NULL == f->mode) return FileOpen_NoMode;

	fpath = f->filePath;
	f->size = getfilesize(f->filePath);

	f->fp = fopen(fpath->path_get(f->filePath), f->mode);
	if(NULL == f->fp) return FileOpen_CantAccess;

	return FileOpen_NoError;
}

