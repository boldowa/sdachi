/**
 * TextFile.c
 */
#include "common/types.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include "common/Str.h"
#include "file/FilePath.h"
#include "file/File.h"
#include "File.protected.h" /* inherit */
#include "file/TextFile.h"

/* this header isn't read from anything other */
/* than inherited object.                     */ 
#include "TextFile.protected.h"

#define BuffLen 256

/* prototypes */
/* overrides */
static E_FileOpen Open(TextFile*);
static E_FileOpen Open2(TextFile*, const char*);

static uint row_get(TextFile*);
static const char* GetLine(TextFile*);
static void Printf(TextFile*, const char*, ...);


/*--------------- Constructor / Destructor ---------------*/

/**
 * @brief Create TextFile object
 *
 * @return the pointer of object
 */
TextFile* new_TextFile(const char* path)
{
	File* super;
	TextFile* self;
	TextFile_protected* pro;

	/* make objects */
	super = new_File(path);
	pro = malloc(sizeof(TextFile_protected));
	self = malloc(sizeof(TextFile));

	/* check whether object creatin succeeded */
	assert(pro);
	assert(self);

	/* inherit */
	memcpy(&self->super, super, sizeof(File));
	free(super);

	/*--- set protected member ---*/
	pro->line = 0;
	pro->lineBuffer = NULL;

	/*--- set public member ---*/
	self->Open = Open;
	self->Open2 = Open2;
	self->row_get = row_get;
	self->GetLine = GetLine;
	self->Printf = Printf;

	/* init TextFile object */
	self->pro = pro;
	return self;
}

/**
 * @brief delete own member variables
 *
 * @param the pointer of object
 */
void delete_TextFile_members(TextFile* self)
{
	/* delete super members */
	delete_File_members(&self->super);

	/* delete protected members */
	free(self->pro->lineBuffer);
}

/**
 * @brief Delete TextFile object
 *
 * @param the pointer of object
 */
void delete_TextFile(TextFile** self)
{
	/* This is the template that default destractor. */
	assert(self);
	if(NULL == (*self)) return;
	delete_TextFile_members(*self);
	free((*self)->pro);
	free(*self);
	(*self) = NULL;
}


/*--------------- internal methods ---------------*/

static uint row_get(TextFile* self)
{
	assert(self);
	return self->pro->line;
}

static const char* GetLine(TextFile* self)
{
	char buf[BuffLen];
	char* result;
	char* tmp;
	char* p;
	size_t chompInx;
	TextFile_protected* txt;
	File_protected* filep;

	assert(self);
	txt = self->pro;
	filep = self->super.pro;

	result = calloc(1, sizeof(char));
	assert(result);

	while(true)
	{
		p = fgets(buf, BuffLen, filep->fp);
		if(NULL == p)
		{
			if(0 == strlen(result))
			{
				free(result);
				free(txt->lineBuffer);
				txt->lineBuffer = NULL;
				return NULL;
			}
			break;
		}

		tmp = Str_concat(result, buf);
		assert(tmp);
		free(result);
		result = tmp;
		tmp = NULL;

		if('\n' == buf[strlen(buf)-1])
		{
			break;
		}
	}

	/* chomp */
	chompInx = strlen(result) -1;
	while( ('\n' == result[chompInx]) || ('\r' == result[chompInx]))
	{
		result[chompInx--] = '\0';
	}

	free(txt->lineBuffer);
	txt->lineBuffer = result;
	txt->line++;
	return result;
}

static void Printf(TextFile* self, const char* fmt, ...)
{
	FILE* fp;
	va_list vl;

	assert(self);
	fp = self->super.pro->fp;
	va_start(vl, fmt);
	vfprintf(fp, fmt, vl);
	va_end(vl);
}

static E_FileOpen Open2(TextFile* self, const char* mode)
{
	File_protected* filep;
	TextFile_protected* textp;

	assert(self);

	filep = self->super.pro;
	textp = self->pro;
	if(NULL != mode)
	{
		if(NULL != filep->mode)
		{
			free(filep->mode);
		}
		filep->mode = Str_copy(mode);
		assert(filep->mode);
	}

	textp->line = 0;
	return self->super.Open(&self->super);
}

static E_FileOpen Open(TextFile* self)
{
	return Open2(self, "r");
}

