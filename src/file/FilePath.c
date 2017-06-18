/**
 * FilePath.c
 */
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <setjmp.h>
#include "common/types.h"
#include "file/FilePath.h"

/* Private members */
struct _FilePath_private {
	char*		path;
	char*		dir;
	char*		name;
	char*		ext;
};

/* prototypes */
static FilePath* Clone(FilePath*);

static bool SetPath(FilePath*, const char*);
static bool SetDir(FilePath*, const char*);
static bool SetName(FilePath*, const char*);
static bool SetExt(FilePath*, const char*);

static const char* GetPath(FilePath*);
static const char* GetDir(FilePath*);
static const char* GetName(FilePath*);
static const char* GetExt(FilePath*);


/*--------------- Constructor / Destructor ---------------*/

/**
 * @brief Create FilePath object
 *
 * @return the pointer of object
 */
FilePath* new_FilePath(const char* path)
{
	FilePath* self;
	FilePath_private* pri;

	/* make objects */
	pri = malloc(sizeof(FilePath_private));
	self = malloc(sizeof(FilePath));

	/* check whether object creatin succeeded */
	assert(pri);
	assert(self);

	/*--- set private member ---*/
	pri->path = pri->dir = pri->name = pri->ext = NULL;

	/*--- set public member ---*/
	self->Clone = Clone;
	self->path_get = GetPath; self->path_set = SetPath;
	self->dir_get  = GetDir;  self->dir_set  = SetDir;
	self->name_get = GetName; self->name_set = SetName;
	self->ext_get  = GetExt;  self->ext_set  = SetExt;

	/* init CClass object */
	self->pri = pri;

	/* Set initial path */
	if(NULL != path)
	{
		self->path_set(self, path);
	}

	return self;
}

/**
 * @brief delete own member variables
 *
 * @param the pointer of object
 */
void delete_FilePath_members(FilePath* self)
{
	/* delete private members */
	free(self->pri->path);
	free(self->pri->dir);
	free(self->pri->name);
	free(self->pri->ext);
}

/**
 * @brief Delete FilePath object
 *
 * @param the pointer of object
 */
void delete_FilePath(FilePath** self)
{
	/* This is the template that default destractor. */
	assert(self);
	if(NULL == (*self)) return;
	delete_FilePath_members(*self);
	free((*self)->pri);
	free(*self);
	(*self) = NULL;
}


/*--------------- internal methods ---------------*/


/*--- sub methods prototypes ---*/
static void path_strcat(jmp_buf *e, char* s1, const char* s2, const size_t max);
static bool _SetPath(FilePath_private* fp, const char* path);
static bool _Set3(FilePath_private* fp, const char* dir, const char* name, const char* ext);

static FilePath* Clone(FilePath* src)
{
	assert(src);
	return new_FilePath(src->path_get(src));
}

static bool SetPath(FilePath* self, const char* path)
{
	const char* dot_pos = NULL;
	const char* slash_pos = NULL;
	size_t len;
	bool isNoDir = false;
	FilePath_private* fp;

	assert(self);
	fp = self->pri;

	if(MAX_PATH < strlen(path))
	{
		return false;
	}

	/* search slash */
	slash_pos = strrchr(path, '/');
#if isWindows
	{
		char* bslash_pos = NULL;
		bslash_pos = strrchr(path, '\\');
		if((NULL != bslash_pos) && (slash_pos < bslash_pos))
		{
			slash_pos = bslash_pos;
		}
	}
#endif /* isWindows */
	/* release directory */
	if(NULL != fp->dir)
	{
		free(fp->dir);
		fp->dir = NULL;
	}

	/* set directory */
	len = 0;
	if(NULL != slash_pos)
	{
		len = (size_t)(slash_pos - path)+1;
	}
	fp->dir = calloc(len+1, sizeof(char));
	assert(fp->dir);
	strncpy_s(fp->dir, len+1, path, len);

	if(NULL == slash_pos)
	{
		isNoDir = true;
		slash_pos = path;
	}

	/* search dot pos */
	dot_pos = strrchr(path, '.');
	if(NULL == dot_pos || dot_pos < slash_pos)  /* no dot or "./aaa" */
	{
		dot_pos = path + strlen(path);
	}

	/* release name */
	if(NULL != fp->name)
	{
		free(fp->name);
		fp->name = NULL;
	}

	/* set filename */
	len = (size_t)(dot_pos - slash_pos);
	if(true == isNoDir)
	{
		if(path != dot_pos)
		{
			len++;
		}
	}
	if(slash_pos != dot_pos)
	{
		len--;
	}
	fp->name = calloc(len+1, sizeof(char));
	assert(fp->name);
	{
		const char* x = slash_pos;
		if(false == isNoDir)
		{
			x++;
		}
		strncpy_s(fp->name, len+1, x, len);
	}

	/* release extension */
	if(NULL != fp->ext)
	{
		free(fp->ext);
		fp->ext = NULL;
	}

	/* set extension */
	len = strlen(path) - (size_t)(dot_pos - path);
	if(slash_pos == dot_pos)
	{
		/* Non-extention */
		len = 0;
	}
	if(path == dot_pos)
	{
		/* path = ".xxx" */
		len = strlen(path);
	}
	fp->ext = calloc(len+1, sizeof(char));
	assert(fp->ext);
	strncpy_s(fp->ext, len+1, dot_pos, len);

	return _SetPath(fp, path);
}

static bool SetDir(FilePath* self, const char* dir)
{
	char* tmp;
	size_t len;
	FilePath_private* fp;

	assert(self);
	fp = self->pri;

	len = strlen(dir);
	tmp = calloc(len+1, sizeof(char));
	assert(tmp);
	strcpy_s(tmp, len+1, dir);

	if(false == _Set3(fp, tmp, fp->name, fp->ext))
	{
		free(tmp);
		return false;
	}

	if(NULL != fp->dir)
	{
		free(fp->dir);
	}
	fp->dir = tmp;

	return true;
}

static bool SetName(FilePath* self, const char* name)
{
	char* tmp;
	size_t len;
	FilePath_private* fp;

	assert(self);
	fp = self->pri;

	len = strlen(name);
	tmp = calloc(len+1, sizeof(char));
	assert(tmp);
	strcpy_s(tmp, len+1, name);

	if(false == _Set3(fp, fp->dir, tmp, fp->ext))
	{
		free(tmp);
		return false;
	}

	if(NULL != fp->name)
	{
		free(fp->name);
	}
	fp->name = tmp;

	return true;
}

static bool SetExt(FilePath* self, const char* ext)
{
	char* tmp;
	size_t len;
	FilePath_private* fp;

	assert(self);
	fp = self->pri;

	len = strlen(ext);
	tmp = calloc(len+1, sizeof(char));
	assert(tmp);
	strcpy_s(tmp, len+1, ext);

	if(false == _Set3(fp, fp->dir, fp->name, tmp))
	{
		free(tmp);
		return false;
	}

	if(NULL != fp->ext)
	{
		free(fp->ext);
	}
	fp->ext = tmp;

	return true;
}

static const char* GetPath(FilePath* self){ return self->pri->path; }
static const char* GetDir(FilePath* self) { return self->pri->dir; }
static const char* GetName(FilePath* self){ return self->pri->name; }
static const char* GetExt(FilePath* self) { return self->pri->ext; }

/* ----------sub methods ----------*/

static void path_strcat(jmp_buf *e, char* s1, const char* s2, const size_t max)
{
	if(max-1 < (strlen(s1) + strlen(s2)))
	{
		longjmp((*e), 1);
	}
	strcat_s(s1, max, s2);
}

static bool _SetPath(FilePath_private* fp, const char* path)
{
	char* tmp;
	size_t len;

	len = strlen(path);
	tmp = calloc(len+1, sizeof(char));
	assert(tmp);
	strcpy_s(tmp, len+1, path);

	if(NULL != fp->path)
	{
		free(fp->path);
	}
	fp->path = tmp;

	return true;
}

static bool _Set3(FilePath_private* fp, const char* dir, const char* name, const char* ext)
{
	char tmp[MAX_PATH+1];
	jmp_buf e;

	tmp[0] = '\0';

	if(0 == setjmp(e)) /* try */
	{
		path_strcat(&e, tmp, dir, MAX_PATH+1);
		path_strcat(&e, tmp, name, MAX_PATH+1);
		path_strcat(&e, tmp, ext, MAX_PATH+1);
	}
	else /* catch */
	{
		return false;
	}

	return _SetPath(fp, tmp);
}

