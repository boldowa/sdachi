/**
 * types.h
 */
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define isWindows \
		   defined(WIN32) \
		|| defined(_WIN32) \
		|| defined(WINDOWS) \
		|| defined(_WINDOWS) \
		|| defined(Windows) \
		|| defined(_Windows) \
	

#define isUnix \
		defined(UNIX) \
		defined(_UNIX) \
		defined(Linux) \
		defined(_Linux) \


#if defined(_MSC_VER)
#  if (_MSC_VER < 1600)
     /* It isn't supported stdint.h and stdbool.h in versions prior to Visual Studio 2010. */
#    ifndef __cplusplus
     /*--- C ---*/
     /* boolean */
#      ifdef _Bool
        typedef _Bool bool;
#      else
        typedef char bool;
#      endif
#      ifndef false
#        define false 0
#      endif
#      ifndef true
#        define true !false
#      endif
#    endif
     /* MSVC funcs */
#    define strncasecmp _strnicmp
#    define strcasecmp _stricmp
#    define fopen(fn,m) _fopen_b(fn,m,__func__)
     FILE* _fopen_b(const char*, const char*, const char*);
#  endif

   /* type stdint */
   typedef unsigned char		uint8_t;
   typedef unsigned short	uint16_t;
   typedef unsigned int		uint32_t;
   typedef unsigned long		uint64_t;
   typedef char			int8_t;
   typedef short			int16_t;
   typedef int			int32_t;
   typedef long			int64_t;

#else
#  ifndef __cplusplus
     /*--- C ---*/
#    include <stdbool.h>
#    include <stdint.h>
#  else
     /*--- C++ ---*/
#    include <cstdint>
#  endif
#endif

/* define type */
typedef unsigned int	uint;
typedef unsigned long	ulong;
typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;
typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

/**
 * define maxpath
 */
#ifndef MAX_PATH
  #define MAX_PATH 260
#endif

/**
 * define __func__ for old Visual Studio
 */
#ifndef __func__
#  ifdef __FUNCTION__
#    define __func__ __FUNCTION__
#  endif
#endif

/**
 * string secure function overwrap for except visual studio
 */
#ifndef _MSC_VER
#  define strcpy_s(dst, len, src) strcpy(dst, src)
#  define strncpy_s(dst, len, src, cnt) strncpy(dst, src, cnt)
#  define strcat_s(s1, len, s2) strcat(s1, s2)
#  define strncat_s(s1, len, s2, cnt) strncat(s1, s2, cnt)
#  define sprintf_s(buf, len, fmt, ...)  sprintf(buf, fmt, ##__VA_ARGS__)
#  define vsprintf_s(buf, len, fmt, ...)  vsprintf(buf, fmt, ##__VA_ARGS__)
#endif
