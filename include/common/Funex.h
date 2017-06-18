#pragma once
/**********************************************************
 *
 * Funex is responsible for ...
 *
 **********************************************************/

typedef bool (*FunexFunc_t)(const char*, void*);

typedef struct FunexStruct {
	FunexFunc_t	func;
	void*		match;
} FunexStruct;

/* Main */
int FunexMatch(const char*, FunexStruct*);

/* Funex libs */
bool ishex(const char);
int atoh(const char*);
bool IsSpace(const char);
void CutOffTailSpaces(char*);
void SkipSpaces(const char*, size_t*, const size_t);
void SkipUntilSpaces(const char*, size_t*, const size_t);
void SkipSpacesRev(const char*, size_t*);
void SkipUntilChar(const char*, size_t*, const char, const size_t);
