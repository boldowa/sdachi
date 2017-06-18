/**
 * RomFile.c
 */
#include "common/types.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "common/Str.h"
#include "common/ReadWrite.h"
#include "file/FilePath.h"
#include "file/File.h"
#include "File.protected.h"
#include "file/RomFile.h"

/* this header isn't read from anything other */
/* than inherited object.                     */ 
#include "RomFile.protected.h"

/* define fill-byte */
#define FILL 0x00

typedef struct _RomTypeScore {
	bool hasHeader;
	bool detected;
	bool isSA1;
	bool isSPC7110;
} RomTypeScore;
typedef struct _RomDetectScore {
	bool header;
	RomTypeScore LoRom;
	RomTypeScore HiRom;
	RomTypeScore ExLoRom;
	RomTypeScore ExHiRom;
} RomDetectScore;


/* prototypes */
static long size_get(RomFile*);
static RomType type_get(RomFile*);
static MapMode mapmode_get(RomFile*);
static uint16 sum_get(RomFile*);
static E_FileOpen Open(RomFile*);
static bool Reload(RomFile*);
static void Close(RomFile*);
static bool Write(RomFile*);
static uint32 NullSnesAdr(RomFile*, const uint32);
static uint8* GetPcPtr(RomFile*, const uint32);
static uint8* GetSnesPtr(RomFile*, const uint32);
static uint32 RatsSearchFail(RomFile*, const uint32, RatsSearcher_t);
static bool RatsCleanFalse(RomFile*, const uint32);
static uint32 RatsSearch(RomFile*, const uint32, RatsSearcher_t);
static bool RatsClean(RomFile*, const uint32);
static bool IsValidSum(RomFile*);
static void UseHiRomMapSA1(RomFile*, bool);


/*--------------- Constructor / Destructor ---------------*/

/**
 * @brief Create RomFile object
 *
 * @return the pointer of object
 */
RomFile* new_RomFile(const char* path)
{
	File* super;
	RomFile* self;
	RomFile_protected* pro;

	/* make objects */
	super = new_File(path);
	self = malloc(sizeof(RomFile));
	pro = malloc(sizeof(RomFile_protected));

	/* check whether object creatin succeeded */
	assert(pro);
	assert(self);

	/* inherit */
	memcpy(&self->super, super, sizeof(File));
	free(super);
	self->pro = pro;

	/*--- set protected member ---*/
	self->pro->hasHeader = false;
	self->pro->hcsum = self->pro->hcsumc = 0;
	self->pro->csum = 0;
	self->pro->map = MapMode_Unknown;
	self->pro->type = RomType_Unknown;
	self->pro->size = 0;
	self->pro->rom = NULL;
	self->pro->raw = NULL;
	self->pro->sa1adrinf.useHiRomMap = false;
	self->pro->sa1adrinf.slots[0] = 0;
	self->pro->sa1adrinf.slots[1] = 0;
	self->pro->sa1adrinf.slots[2] = 0;
	self->pro->sa1adrinf.slots[3] = 0;
	/* set default rom file mode */
	self->super.pro->mode = Str_copy("rb+");

	/*--- set public member ---*/
	self->Open = Open;
	self->Reload = Reload;
	self->Close = Close;
	self->Write = Write;
	self->size_get = size_get;
	self->type_get = type_get;
	self->mapmode_get = mapmode_get;
	self->sum_get = sum_get;
	self->IsValidSum = IsValidSum;
	self->Snes2PcAdr = NullSnesAdr;
	self->Pc2SnesAdr = NullSnesAdr;
	self->GetPcPtr = GetPcPtr;
	self->GetSnesPtr = GetSnesPtr;
	self->RatsSearch = RatsSearchFail;
	self->RatsClean = RatsCleanFalse;
	self->UseHiRomMapSA1 = UseHiRomMapSA1;

	/* init RomFile object */
	return self;
}

/**
 * @brief delete own member variables
 *
 * @param the pointer of object
 */
void delete_RomFile_members(RomFile* self)
{
	/* delete super members */
	self->Close(self);
	delete_File_members(&self->super);

	/* delete protected members */
	free(self->pro);
}

/**
 * @brief Delete RomFile object
 *
 * @param the pointer of object
 */
void delete_RomFile(RomFile** self)
{
	/* This is the template that default destractor. */
	assert(self);
	if(NULL == (*self)) return;
	delete_RomFile_members(*self);
	free(*self);
	(*self) = NULL;
}


/*--------------- internal methods ---------------*/

static void DetectRomType(RomFile*);
static void CalcSum(RomFile*);

static long size_get(RomFile* self)
{
	assert(self);
	return self->pro->size;
}
static RomType type_get(RomFile* self)
{
	assert(self);
	return self->pro->type;
}
static MapMode mapmode_get(RomFile* self)
{
	assert(self);
	return self->pro->map;
}
static uint16 sum_get(RomFile* self)
{
	assert(self);
	return self->pro->csum;
}

static E_FileOpen Open(RomFile* self)
{
	uint8* raw;
	E_FileOpen result;
#ifndef NDEBUG
	size_t rlen;
#endif

	assert(self);

	result = self->super.Open(&self->super);
	if(FileOpen_NoError != result)
	{
		return result;
	}

	raw = (uint8*)malloc((size_t)self->super.pro->size * sizeof(uint8));
	assert(raw);
#ifndef NDEBUG
	rlen =  fread(raw, sizeof(uint8), (size_t)self->super.pro->size, self->super.pro->fp);
#else
	fread(raw, sizeof(uint8), (size_t)self->super.pro->size, self->super.pro->fp);
#endif
	assert(self->super.pro->size == rlen);
	self->pro->raw = raw;

	DetectRomType(self);
	CalcSum(self);
	if(MapMode_Unknown != self->pro->map)
	{
		self->RatsSearch = RatsSearch;
		self->RatsClean = RatsClean;
	}

	return FileOpen_NoError;
}

static void Close(RomFile* self)
{
	assert(self);
	free(self->pro->raw);
	self->pro->raw = NULL;
	self->pro->rom = NULL;
	self->super.Close(&self->super);

	self->Snes2PcAdr = NullSnesAdr;
	self->Pc2SnesAdr = NullSnesAdr;
	self->RatsSearch = RatsSearchFail;
	self->RatsClean = RatsCleanFalse;
}

static bool Reload(RomFile* self)
{
	assert(self);

	rewind(self->super.pro->fp);
	fseek(self->super.pro->fp, 0, SEEK_SET);
	return (self->super.pro->size == fread(self->pro->raw, sizeof(uint8), (size_t)self->super.pro->size, self->super.pro->fp));
}

static bool Write(RomFile* self)
{
	uint32 sumadr;
	uint16 csumc;

	assert(self);

	sumadr = self->Snes2PcAdr(self, 0xffdc);
	if(ROMADDRESS_NULL == sumadr) return false;

	/* re-calculate checksum */
	CalcSum(self);
	csumc = self->pro->csum ^ 0xffff;
	write16(&self->pro->rom[sumadr+0], csumc);
	write16(&self->pro->rom[sumadr+2], self->pro->csum);
	self->pro->hcsum = self->pro->csum;
	self->pro->hcsumc = csumc;
	rewind(self->super.pro->fp);
	fseek(self->super.pro->fp, 0, SEEK_SET);
	if(self->pro->hasHeader)
	{
		fseek(self->super.pro->fp, 0x200, SEEK_SET);
	}
	return (self->pro->size == fwrite(self->pro->rom, sizeof(uint8), (size_t)self->pro->size, self->super.pro->fp));
}

/*=== Checksum calculate methods =========================*/
static uint16 byteSum(const uint8 *data, long len)
{
	uint16 sum = 0;
	long i;

	for(i=0; i<len; i++)
	{
		sum = (uint16)(sum + data[i]);
	}

	return sum;
}
static void CalcSum(RomFile* self)
{
	long mask = 0x1000000;
	uint32 sumadr;

	assert(self);
	sumadr = self->Snes2PcAdr(self, 0xffdc);
	/*assert(ROMADDRESS_NULL != sumadr);*/
	if(ROMADDRESS_NULL == sumadr) return;

	/* get header sum */
	self->pro->hcsumc = read16(&self->pro->rom[sumadr+0]);
	self->pro->hcsum  = read16(&self->pro->rom[sumadr+2]);

	/* get first bit */
	while(0 == (mask & self->pro->size)) mask >>= 1;
	/* calc first bit sum */
	self->pro->csum = byteSum(&self->pro->rom[0], mask);
	/* calc other sums */
	self->pro->csum = (uint16)(self->pro->csum + (byteSum(&self->pro->rom[mask], self->pro->size-mask) << 1));
}

static bool IsValidSum(RomFile* self)
{
	assert(self);
	if(self->pro->hcsum == self->pro->csum)
	{
		if(self->pro->hcsum == (self->pro->hcsumc^0xffff))
		{
			return true;
		}
	}
	return false;
}

/*=== RomAddress convert methods =========================*/
/*===== LoRom =====*/
static uint32 LoRom_Snes2Pc(RomFile* self, const uint32 sna)
{
	uint32 pca;
	if(0x1000000 <= sna) return ROMADDRESS_NULL;
	if(0 == (sna & 0x8000)) return ROMADDRESS_NULL;

	pca = (((sna&0x7f0000)>>1) + (sna&0x7fff));
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	return pca;
}
static uint32 LoRom_Pc2Snes(RomFile* self, const uint32 pca)
{
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	if(0x400000 <= pca) return ROMADDRESS_NULL;

	return ((((pca&0x7f8000)<<1)|0x800000) + ((pca&0x7fff)|0x8000));
}

/*===== SA-1(LoRom) =====*/
static uint32 SA1_Snes2Pc(RomFile* self, const uint32 sna)
{
	uint32 pca;
	int slot;
	uint8 bnk = (uint8)(sna >> 16);
	bool useHiRomMap;

	if((0x00 <= bnk) && (0x20 > bnk))
	{
		slot = 0;
		useHiRomMap = false;
	}
	else if((0x20 <= bnk) && (0x40 > bnk))
	{
		slot = 1;
		useHiRomMap = false;
	}
	else if((0x80 <= bnk) && (0xa0 > bnk))
	{
		slot = 2;
		useHiRomMap = false;
	}
	else if((0xa0 <= bnk) && (0xc0 > bnk))
	{
		slot = 3;
		useHiRomMap = false;
	}
	else if((0xc0 <= bnk) && (0xcf >= bnk))
	{
		slot = 0;
		useHiRomMap = true;
	}
	else if((0xd0 <= bnk) && (0xdf >= bnk))
	{
		slot = 1;
		useHiRomMap = true;
	}
	else if((0xe0 <= bnk) && (0xef >= bnk))
	{
		slot = 2;
		useHiRomMap = true;
	}
	else if((0xf0 <= bnk) && (0xff >= bnk))
	{
		slot = 3;
		useHiRomMap = true;
	}
	else
	{
		return ROMADDRESS_NULL;
	}

	if(useHiRomMap)
	{
		pca = (uint32)((((self->pro->sa1adrinf.slots[slot])+((uint32)bnk&0x0f)) << 16) + ((uint32)sna & 0xffff));
	}
	else
	{
		if(0 == (sna & 0x8000)) return ROMADDRESS_NULL;
		pca = (uint32)(((uint32)self->pro->sa1adrinf.slots[slot] << 16) + (((uint32)bnk&0x1f) << 15) + ((uint32)sna & 0x7fff));
	}

	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	return pca;
}
static uint32 SA1_Pc2Snes(RomFile* self, const uint32 pca)
{
	int i;
	uint8 bnk = (uint8)(pca>>16);
	uint32 add = 0;

	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	if(0x800000 <= pca) return ROMADDRESS_NULL;

	/* HiRom map */
	if(self->pro->sa1adrinf.useHiRomMap)
	{
		for(i=0; i<4; i++)
		{
			if(self->pro->sa1adrinf.slots[i] <= bnk
					&& self->pro->sa1adrinf.slots[i] + 16 > bnk)
			{
				break;
			}
		}
		if(4 <= i) return ROMADDRESS_NULL;
		return (((((self->pro->sa1adrinf.slots[i] & 0x7f) + ((uint32)bnk & 0x0f)) << 16) + (((uint32)pca&0xffff))) | 0xc00000);
	}

	/* LoRom map */
	for(i=0; i<4; i++)
	{
		if(self->pro->sa1adrinf.slots[i] <= bnk
		&& self->pro->sa1adrinf.slots[i] + 16 > bnk)
		{
			break;
		}
	}
	switch(i) /* switch slot */
	{
		case 0:
			add = 0x00;
			break;
		case 1:
			add = 0x20;
			break;
		case 2:
			add = 0x80;
			break;
		case 3:
			add = 0xa0;
			break;
		default:
			return ROMADDRESS_NULL;
	}
	return (((add + ((uint32)bnk & 0x0f)) << 16) + (((uint32)pca&0x17fff)|0x8000));
}

/*===== HiRom =====*/
static uint32 HiRom_Snes2Pc(RomFile* self, const uint32 sna)
{
	uint32 pca;

	if(0x1000000 <= sna) return ROMADDRESS_NULL;

	pca = sna & 0x3fffff;
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	return pca;
}
static uint32 HiRom_Pc2Snes(RomFile* self, const uint32 pca)
{
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	if(0x400000 <= pca) return ROMADDRESS_NULL;

	return pca | 0xc00000;
}

/*===== SPC7110(HiRom) =====*/
static uint32 SPC7110_Snes2Pc(RomFile* self, const uint32 sna)
{
	/* TODO: implement */
	return ROMADDRESS_NULL;
}
static uint32 SPC7110_Pc2Snes(RomFile* self, const uint32 pca)
{
	/* TODO: implement */
	return ROMADDRESS_NULL;
}

/*===== ExLoRom =====*/
static uint32 ExLoRom_Snes2Pc(RomFile* self, const uint32 sna)
{
	uint32 pca;

	if(0x1000000 <= sna) return ROMADDRESS_NULL;
	if(0 == (sna & 0x8000)) return ROMADDRESS_NULL;

	if(0 != (0x800000 & sna))
	{
		pca = (((sna&0x7f0000)>>1) + (sna&0x7fff));
	}
	else
	{
		pca = (((sna&0x7f0000)>>1) + (sna&0x7fff) + 0x400000);
	}
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	return pca;
}
static uint32 ExLoRom_Pc2Snes(RomFile* self, const uint32 pca)
{
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	if(0x7f0000 <= pca) return ROMADDRESS_NULL;

	if(0x400000 <= pca)
	{
		return ((((pca&0x7f8000)<<1)&0x7fffff) + ((pca&0x7fff)|0x8000));
	}
	return ((((pca&0x7f8000)<<1)|0x800000) + ((pca&0x7fff)|0x8000));
}

/*===== ExHiRom =====*/
static uint32 ExHiRom_Snes2Pc(RomFile* self, const uint32 sna)
{
	uint32 pca;

	if(0x1000000 <= sna) return ROMADDRESS_NULL;
	if(0 == (sna & 0x400000)) return ROMADDRESS_NULL;

	if(0xc00000 == (0xc00000 & sna))
	{
		pca = (sna & 0x3fffff);
	}
	else
	{
		if(0x3e0000 <= (sna&0x3fffff)) return ROMADDRESS_NULL;
		pca = (sna & 0x3fffff)+0x400000;
	}
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	return pca;
}
static uint32 ExHiRom_Pc2Snes(RomFile* self, const uint32 pca)
{
	if(self->pro->size <= pca) return ROMADDRESS_NULL;
	if(0x7e0000 <= pca) return ROMADDRESS_NULL;

	if(0x400000 > pca)
	{
		return pca | 0xc00000;
	}
	return ((pca & 0x3fffff) + 0x400000);
}


/*=== RomType detect methods =============================*/

static void ExRomJudge(RomFile* self, const uint32 pca, RomTypeScore* rts)
{
	uint8 map;

	map = self->pro->raw[pca - 0x40 + 0x15];

	/* check exrom */
	map &= 0x2f;
	if(0x25 == map)
	{
		rts->detected = true;
	}
}
static void HiRomJudge(RomFile* self, const uint32 pca, RomTypeScore* rts)
{
	uint8 map;

	map = self->pro->raw[pca - 0x40 + 0x15];

	/* check spc7110 (Far easter zero) */
	if(0x3a == map)
	{
		rts->detected = true;
		rts->isSPC7110 = true;
		return;
	}

	/* check generic hirom */
	map &= 0x2f;
	if(0x21 == map)
	{
		rts->detected = true;
	}
}
static void LoRomJudge(RomFile* self, const uint32 pca, RomTypeScore* rts)
{
	uint8 map;

	map = self->pro->raw[pca - 0x40 + 0x15];

	/* check sa-1 */
	if(0x23 == map)
	{
		rts->detected = true;
		rts->isSA1 = true;
		return;
	}

	/* check generic lorom */
	map &= 0x2f;
	if(0x20 == map)
	{
		rts->detected = true;
	}
}
static bool RomType_ValidSum(RomFile* self, const uint32 pca)
{
	uint16 hsum;
	uint16 hsumc;
	uint32 sumadr;

	if(self->super.pro->size < pca)
	{
		return false;
	}

	sumadr = pca - 0x24;
	hsumc = read16(&self->pro->raw[sumadr]);
	hsum  = read16(&self->pro->raw[sumadr+2]);

	if(0xffff != (hsumc + hsum))
	{
		return false;
	}

	return true;
}
static void DetectRomType(RomFile* self)
{
	RomDetectScore rds = {0};

	/* Check has header */
	if(0x200 == (self->super.pro->size % 0x8000))
	{
		rds.header = true;
	}

	/* Check LoRom / SA1Rom */
	if(RomType_ValidSum(self, 0x8000))
	{
		LoRomJudge(self, 0x8000, &rds.LoRom);
	}
	if(true == rds.header)
	{
		if(RomType_ValidSum(self, 0x8200))
		{
			rds.LoRom.hasHeader = true;
			LoRomJudge(self, 0x8200, &rds.LoRom);
		}
	}

	/* Check HiRom / SPC7110Rom */
	if(RomType_ValidSum(self, 0x10000))
	{
		HiRomJudge(self, 0x10000, &rds.HiRom);
	}
	if(true == rds.header)
	{
		if(RomType_ValidSum(self, 0x10200))
		{
			rds.HiRom.hasHeader = true;
			HiRomJudge(self, 0x10200, &rds.HiRom);
		}
	}

	/* Check ExLoRom */
	if(RomType_ValidSum(self, 0x408000))
	{
		ExRomJudge(self, 0x408000, &rds.ExLoRom);
	}
	if(true == rds.header)
	{
		if(RomType_ValidSum(self, 0x408200))
		{
			rds.ExLoRom.hasHeader = true;
			ExRomJudge(self, 0x408200, &rds.ExLoRom);
		}
	}

	/* Check ExHiRom */
	if(RomType_ValidSum(self, 0x410000))
	{
		ExRomJudge(self, 0x410000, &rds.ExHiRom);
	}
	if(true == rds.header)
	{
		if(RomType_ValidSum(self, 0x410200))
		{
			rds.ExHiRom.hasHeader = true;
			ExRomJudge(self, 0x410200, &rds.ExHiRom);
		}
	}

	/* Init rom type */
	self->pro->type = RomType_Unknown;
	self->pro->hasHeader = false;
	self->pro->rom = &self->pro->raw[0];
	self->pro->size = self->super.pro->size;

	/* Jundge RomType */
	if(true == rds.ExHiRom.detected)
	{
		self->pro->map = self->pro->raw[0x40ffd5];
		self->pro->type = RomType_ExHiRom;
		self->Snes2PcAdr = ExHiRom_Snes2Pc;
		self->Pc2SnesAdr = ExHiRom_Pc2Snes;
		if(true == rds.ExHiRom.hasHeader)
		{
			self->pro->hasHeader = true;
			self->pro->map = self->pro->raw[0x4101d5];
			self->pro->rom = &self->pro->raw[0x200];
			self->pro->size -= 0x200;
		}
		return;
	}
	if(true == rds.ExLoRom.detected)
	{
		self->pro->map = self->pro->raw[0x407fd5];
		self->pro->type = RomType_ExLoRom;
		self->Snes2PcAdr = ExLoRom_Snes2Pc;
		self->Pc2SnesAdr = ExLoRom_Pc2Snes;
		if(true == rds.ExLoRom.hasHeader)
		{
			self->pro->hasHeader = true;
			self->pro->map = self->pro->raw[0x4081d5];
			self->pro->rom = &self->pro->raw[0x200];
			self->pro->size -= 0x200;
		}
		return;
	}
	if(true == rds.HiRom.detected)
	{
		self->pro->map = self->pro->raw[0xffd5];
		self->pro->type = RomType_HiRom;
		self->Snes2PcAdr = HiRom_Snes2Pc;
		self->Pc2SnesAdr = HiRom_Pc2Snes;
		if(true == rds.HiRom.hasHeader)
		{
			self->pro->hasHeader = true;
			self->pro->map = self->pro->raw[0x101d5];
			self->pro->rom = &self->pro->raw[0x200];
			self->pro->size -= 0x200;
		}
		if(true == rds.HiRom.isSPC7110)
		{
			self->pro->map = MapMode_SPC7110;
			self->Snes2PcAdr = SPC7110_Snes2Pc;
			self->Pc2SnesAdr = SPC7110_Pc2Snes;
		}
		return;
	}
	if(true == rds.LoRom.detected)
	{
		self->pro->map = self->pro->raw[0x7fd5];
		self->pro->type = RomType_LoRom;
		self->Snes2PcAdr = LoRom_Snes2Pc;
		self->Pc2SnesAdr = LoRom_Pc2Snes;
		if(true == rds.LoRom.hasHeader)
		{
			self->pro->hasHeader = true;
			self->pro->map = self->pro->raw[0x81d5];
			self->pro->rom = &self->pro->raw[0x200];
			self->pro->size -= 0x200;
		}
		if(true == rds.LoRom.isSA1)
		{
			self->Snes2PcAdr = SA1_Snes2Pc;
			self->Pc2SnesAdr = SA1_Pc2Snes;
			/* init sa-1 bankmap */
			self->pro->sa1adrinf.useHiRomMap = false;
			self->pro->sa1adrinf.slots[0] = 0x00;
			self->pro->sa1adrinf.slots[1] = 0x10;
			self->pro->sa1adrinf.slots[2] = 0x20;
			self->pro->sa1adrinf.slots[3] = 0x30;
		}
		return;
	}

	self->pro->rom = NULL;
}

/*=== DataPtr methods ====================================*/
static uint8* GetSnesPtr(RomFile* self, const uint32 sna)
{
	uint32 pca;

	assert(self);
	pca = self->Snes2PcAdr(self, sna);
	if(ROMADDRESS_NULL == pca) return NULL;

	return &self->pro->rom[pca];
}

static uint8* GetPcPtr(RomFile* self, const uint32 pca)
{
	assert(self);
	if(ROMADDRESS_NULL == self->Pc2SnesAdr(self, pca)) return NULL;

	return &self->pro->rom[pca];
}

/*=== Default methods ====================================*/
static uint32 NullSnesAdr(RomFile* self, const uint32 ad)
{
	return ROMADDRESS_NULL;
}
static uint32 RatsSearchFail(RomFile* self, const uint32 ad, RatsSearcher_t search)
{
	return ROMADDRESS_NULL;
}
static bool RatsCleanFalse(RomFile* self, const uint32 ad)
{
	return false;
}

static uint8* memsearch(const uint8 *p1, const uint8 *p2, const uint32 len, const uint8* end)
{
	int match;
	for(; p1 <= end; p1++)
	{
		if(p1[0] == p2[0])
		{
			match = 1;
			while(p1[match] == p2[match])
			{
				match++;
				if(match == len)
				{
					return (uint8*)p1;
				}
			}
			p1 += match;
		}
	}
	return NULL;
}
static uint32 RatsSearch(RomFile* self, const uint32 sna, RatsSearcher_t search)
{
	uint32 adr;
	uint8* ptr;
	uint16 sz;
	uint16 szc;

	assert(self);

	adr = self->Snes2PcAdr(self, sna);
	if(ROMADDRESS_NULL == adr) return ROMADDRESS_NULL;

	ptr = &self->pro->rom[adr];

	for ( ; ptr != NULL ; )
	{
		/* Search rats tag */
		ptr = memsearch(ptr, (const uint8*)"STAR", 4, &self->pro->rom[self->pro->size]);
		if(NULL != ptr)
		{
			/* check valid rats tag */
			sz =  read16(ptr+4);
			szc = read16(ptr+6);
			if(sz == (szc^0xffff))
			{
				if(NULL == search || search(ptr+8, (uint32)(sz+1)))
				{
					return self->Pc2SnesAdr(self, (uint32)(ptr - self->pro->rom));
				}
			}

			/* unmatch */
			ptr += sz + 9;
		}
	}

	return ROMADDRESS_NULL;
}

static bool RatsClean(RomFile* self, const uint32 sna)
{
	uint32 adr;
	uint8* ptr;
	uint16 sz;
	uint16 szc;

	assert(self);

	/* Get rom address */
	adr = self->Snes2PcAdr(self, sna);
	if(ROMADDRESS_NULL == adr) return false;
	ptr = &self->pro->rom[adr];

	/* Check RATS tag */
	if(0 != memcmp(ptr, (const uint8*)"STAR", 4)) return false;

	sz =  read16(ptr+4);
	szc = read16(ptr+6);
	if(sz != (szc^0xffff)) return false;

	/* Fill data */
	sz = (uint16)(sz + 9);
	memset(ptr, FILL, sz);

	return true;
}

static void UseHiRomMapSA1(RomFile* self, bool m)
{
	assert(self);
	self->pro->sa1adrinf.useHiRomMap = m;
}

