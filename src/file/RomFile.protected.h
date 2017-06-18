#pragma once
/**
 * RomFile.protected.h
 */

typedef struct _SA1AdrInfo {
	bool		useHiRomMap;
	uint8		slots[4];
} SA1AdrInfo;

/**
 * RomFile main instance
 */
struct _RomFile_protected {
	bool		hasHeader;
	uint8*		raw;
	uint8*		rom;
	long		size;
	RomType		type;
	MapMode		map;
	uint16		csum;
	uint16		hcsum;
	uint16		hcsumc;
	SA1AdrInfo	sa1adrinf;	/* It simulates the SuperMMC. */
};

/**
 * detete own members method prototype
 * I recommend it that you move this method in "RomFile.c"
 * if you plan to make the final class.
 */
void delete_RomFile_members(RomFile*);

