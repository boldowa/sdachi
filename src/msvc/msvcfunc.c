/**
 * msvcfunc.c
 *   this source will compile only visual studio.
 */
#if defined(_MSC_VER)

#include <stdio.h>
#include <errno.h>

static const char* foperrstr(errno_t err)
{
	switch(err)
	{
		case EPERM:
			return "permission denied.";

		case ENOENT:
			return "file not exists.";

		default:
			break;
	}
	return "";
}
FILE* _fopen_b(const char* path, const char* mode, const char* func)
{
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, path, mode);
	if(0 != err)
	{
		fprintf(stderr, "file open error in %s(): %s (errno=%d)\n", func, foperrstr(err), err);
		return NULL;
	}
	return fp;
}

#endif
