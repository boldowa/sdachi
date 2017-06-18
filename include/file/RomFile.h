#pragma once
/**
 * RomFile.h
 */

typedef enum RomType{
	RomType_Unknown = 0,
	RomType_LoRom,
	RomType_HiRom,
	RomType_ExLoRom,
	RomType_ExHiRom
}RomType;

typedef enum MapMode{
	MapMode_Unknown = 0,
	/* Normal speed */
	MapMode_20	= 0x20,		/* LoRom */
	MapMode_21,			/* HiRom */
	MapMode_Reserved,
	MapMode_SA1,			/* SA-1 */
	MapMode_25	= 0x25,		/* ExHiRom */
	/* high speed */
	MapMode_20H	= 0x30,		/* LoRom */
	MapMode_21H,			/* HiRom */
	MapMode_25H	= 0x35,		/* ExHiRom */
	MapMode_SPC7110	= 0x3A,		/* SPC7110 */

}MapMode;

typedef bool (*RatsSearcher_t)(const uint8*, const uint32);

#define ROMADDRESS_NULL 0x80000000

/**
 * public accessor
 */
typedef struct _RomFile RomFile;
typedef struct _RomFile_protected RomFile_protected;
typedef struct _RomFile_private RomFile_private;
struct _RomFile {
	/* inherited */
	File super;
	/* methods */
	long (*size_get)(RomFile*);
	RomType (*type_get)(RomFile*);
	MapMode (*mapmode_get)(RomFile*);
	uint16 (*sum_get)(RomFile*);
	E_FileOpen (*Open)(RomFile*);
	bool (*Reload)(RomFile*);
	void (*Close)(RomFile*);
	bool (*Write)(RomFile*);
	bool (*IsValidSum)(RomFile*);
	uint8* (*GetSnesPtr)(RomFile*, const uint32);
	uint8* (*GetPcPtr)(RomFile*, const uint32);
	uint32 (*Pc2SnesAdr)(RomFile*, const uint32);
	uint32 (*Snes2PcAdr)(RomFile*, const uint32);
	uint32 (*RatsSearch)(RomFile*, const uint32, RatsSearcher_t);
	bool (*RatsClean)(RomFile*, const uint32);
	void (*UseHiRomMapSA1)(RomFile*, bool);
	/* protected members */
	RomFile_protected* pro;
};

/**
 * Constructor
 */
RomFile* new_RomFile(const char*);

/**
 * Destractor
 */
void delete_RomFile(RomFile**);

