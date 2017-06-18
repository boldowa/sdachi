#pragma once
/**
 * TextFile.protected.h
 */

/**
 * TextFile main instance
 */
struct _TextFile_protected {
	/* member */
	char* lineBuffer;
	uint line;
};

/**
 * detete own members method prototype
 */
void delete_TextFile_members(TextFile*);

