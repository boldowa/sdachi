#pragma once

/**********************************************************
 *
 * Str is responsible for ...
 *
 **********************************************************/

/**
 * Copy string
 *   args: Str_concat(const char* s)
 *     s - src string
 *   return:
 *     If copy succeeded, return copyed string pointer.
 *     If copy failed, return NULL.
 */
char* Str_copy(const char* s);

/**
 * Concat string
 *   args: Str_concat(const char* s1, const char* s2)
 *     s1 - string1
 *     s1 - string2
 *   return:
 *     If concat succeeded, return concated string.
 *     If concat failed, return NULL.
 */
char* Str_concat(const char*, const char*);

/**
 * lower-case string
 *   args: Str_tolower(const char* s)
 *     s - string
 */
void Str_tolower(char* s);

/**
 * upper-case string
 *   args: Str_toupper(const char* s)
 *     s - string
 */
void Str_toupper(char* s);

/**
 * Replace string
 *   args: Str_replace(const char* search, const char* replace, const char* target)
 *     search  - string before replacement
 *     replace - replacement string
 *     target  - replacement target
 *   return:
 *     If replace succeeded, return replaced string.
 *     If replace failed, return NULL.
 */
char* Str_replace(const char*, const char*, const char*);

