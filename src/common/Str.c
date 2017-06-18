#include "common/types.h"
#include <assert.h>
#include <ctype.h>
#include "common/Str.h"

char* Str_copy(const char* s)
{
	char* cp;
	size_t len;

	if(NULL==s) return NULL;
	len = strlen(s);
	cp = calloc(len+1, sizeof(char));
	assert(cp);
	strcpy_s(cp, len+1, s);

	return cp;
}

void Str_tolower(char* s)
{
	if(NULL==s) return;
	for(; '\0' != *s; s++)
	{
		*s = (char)tolower(*s);
	}
}

void Str_toupper(char* s)
{
	if(NULL==s) return;
	for(; '\0' != *s; s++)
	{
		*s = (char)toupper(*s);
	}
}

char* Str_concat(const char* s1, const char* s2)
{
	char* con;
	char* top;
	size_t len;
	size_t s1len;

	if(NULL == s1 || NULL == s2) return NULL;

	/* Memory allocation */
	len = strlen(s1) + strlen(s2);
	con = calloc(len+1, sizeof(char));
	assert(con);
	top = con;
	
	/* Connect string */
	strcpy_s(con, len+1, s1);
	s1len = strlen(s1);
	con += s1len;
	len -= s1len;
	strcpy_s(con, len+1, s2);

	return top;
}

char* Str_replace(const char* search, const char* replace, const char* target)
{
	char* work = NULL;
	char* top = NULL;
	char* tmp = NULL;
	char* tmp2 = NULL;
	char* find = NULL;
	char* result;
	size_t len;

	if(NULL == search || NULL == replace || NULL == target) return NULL;

	/* Empty values fail */
	if(0 == strlen(search)) return NULL;

	/* Init result value(default is "") */
	result = calloc(1, sizeof(char));
	assert(result);

	/* Copy for work */
	len = strlen(target);
	work = calloc(len+1, sizeof(char));
	assert(work);
	strcpy_s(work, len+1, target);
	top = work;
	
	/* Replacement loop */
	while(NULL != (find = strstr(work, search)))
	{
		/* Insert termination code on find point */
		*find = '\0';

		/* Replace 1 : connect with first half */
		tmp = Str_concat(work, replace);
		work = find;
		work += strlen(search);

		/* Replace 2 : connect with second half */
		tmp2 = Str_concat(result, tmp);
		free(tmp);
		free(result);
		result = tmp2;
		tmp2 = NULL;
	}

	/* connect with remainder */
	tmp = Str_concat(result, work);

	/* clean */
	free(result);
	free(top);

	/* return result */
	return tmp;
}

