/**
 * RomFileTest.cpp
 */
#include <assert.h>
#include <unistd.h>
extern "C"
{
#include "common/types.h"
#include "common/Str.h"
#include "common/ReadWrite.h"
#include "file/File.h"
#include "file/RomFile.h"
}

#define FILL 0

#include "CppUTest/TestHarness.h"

#define TestRoot "testdata/file/"
#define TestFile "test.smc"

TEST_GROUP(RomFile)
{
	/* test target */
	RomFile* target;

	void setup()
	{
		FILE *f;
		uint16 uitmp = 0xffff;
		uint8 map = 0x20;

		/* create LoRom file */
		f = fopen(TestRoot TestFile, "wb");
		fseek(f, 0x801ff, SEEK_SET);
		fwrite(&map, 1, 1, f);		/* write dummy data */
		fseek(f, 0x81d5, SEEK_SET);
		fwrite(&map, 1, 1, f);		/* write mapmode */
		fseek(f, 0x81dc, SEEK_SET);
		fwrite(&uitmp, 2, 1, f);	/* write dummy sum */
		fclose(f);

		/* create check object */
		target = new_RomFile(TestRoot TestFile);
	}

	void teardown()
	{
		delete_RomFile(&target);
		remove(TestRoot TestFile);
	}
};

/**
 * Check object create
 */
TEST(RomFile, new)
{
	CHECK(NULL != target);

	/* check initial value */
	LONGS_EQUAL(0, target->size_get(target));
	LONGS_EQUAL(RomType_Unknown, target->type_get(target));
	LONGS_EQUAL(MapMode_Unknown, target->mapmode_get(target));
	LONGS_EQUAL(0, target->sum_get(target));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x8000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x0000));
	POINTERS_EQUAL(NULL, target->GetPcPtr(target, 0))
	POINTERS_EQUAL(NULL, target->GetSnesPtr(target, 0))
	LONGS_EQUAL(ROMADDRESS_NULL, target->RatsSearch(target, 0x8000, NULL));
	CHECK_FALSE(target->RatsClean(target, 0x8000));
}

/**
 * Check object delete
 */
TEST(RomFile, delete)
{
	delete_RomFile(&target);

	/* check delete */
	POINTERS_EQUAL(NULL, target);
}

/**
 * Check size_get function
 */
TEST(RomFile, size_get)
{
	/* Nothing to do here */
}

/**
 * Check type_get function
 */
TEST(RomFile, type_get)
{
	/* Nothing to do here */
}
/**
 * Check mapmode_get function
 */
TEST(RomFile, mapmode_get)
{
	/* Nothing to do here */
}
/**
 * Check sum_get function
 */
TEST(RomFile, sum_get)
{
	/* Nothing to do here */
}
/**
 * Check Open function
 */
TEST(RomFile, Open)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check rom info */
	LONGS_EQUAL(MapMode_20, target->mapmode_get(target));
	LONGS_EQUAL(RomType_LoRom, target->type_get(target));
	LONGS_EQUAL(0x80000, target->size_get(target));
	LONGS_EQUAL(0x023e, target->sum_get(target));

	/* close */
	target->Close(target);
}

/**
 * check Reload method
 */
TEST(RomFile, Reload)
{
	uint8* ptr;

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* write data */
	ptr = target->GetSnesPtr(target, 0x8000);
	write16(ptr, 0x1234);
	LONGS_EQUAL(0x1234, read16(ptr));

	/* reload */
	CHECK(target->Reload(target));

	/* check data */
	CHECK(0x1234 != read16(ptr));

	target->Close(target);
}

/**
 * check Close method
 */
TEST(RomFile, Close)
{
	/* Nothing to do here */
}

/**
 * check Write method
 */
TEST(RomFile, Write)
{
	uint8* ptr;

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* write data */
	ptr = target->GetSnesPtr(target, 0x8000);
	write16(ptr, 0x1234);
	LONGS_EQUAL(0x1234, read16(ptr));

	/* write */
	CHECK(target->Write(target));

	/* reload */
	CHECK(target->Reload(target));

	/* check data */
	LONGS_EQUAL(0x1234, read16(ptr));
}

/**
 * check IsValidSum method
 */
TEST(RomFile, IsValidSum)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));
	CHECK_FALSE(target->IsValidSum(target));

	/* fix checksum */
	CHECK(target->Write(target));
	/* check */
	CHECK(target->IsValidSum(target));
}

/**
 * check GetSnesPtr method
 */
TEST(RomFile, GetSnesPtr)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	LONGS_EQUAL(NULL, target->GetSnesPtr(target, 0x108000));
	LONGS_EQUAL(NULL, target->GetSnesPtr(target, 0x010000));
	LONGS_EQUAL(NULL, target->GetSnesPtr(target, 0x007fff));
	LONGS_EQUAL(NULL, target->GetSnesPtr(target, 0x000000));
	CHECK_FALSE(NULL == target->GetSnesPtr(target, 0xffff));
}

/**
 * check GetPcPtr method
 */
TEST(RomFile, GetPcPtr)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	LONGS_EQUAL(NULL, target->GetPcPtr(target, 0x80000));
	CHECK_FALSE(NULL == target->GetPcPtr(target, 0x0000));
	CHECK_FALSE(NULL == target->GetPcPtr(target, 0x7fff));
}

/**
 * check Snes2PcAdr method
 */
TEST(RomFile, Snes2PcAdr)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x108000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x010000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x007fff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x000000));
	LONGS_EQUAL(0x040000, target->Snes2PcAdr(target, 0x888000));
	LONGS_EQUAL(0x007fff, target->Snes2PcAdr(target, 0x00ffff));
	LONGS_EQUAL(0x007fff, target->Snes2PcAdr(target, 0x80ffff));
}

/**
 * check Pc2SnesAdr method
 */
TEST(RomFile, Pc2SnesAdr)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x80000));
	LONGS_EQUAL(0x888000, target->Pc2SnesAdr(target, 0x040000));
	LONGS_EQUAL(0x808000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0x80ffff, target->Pc2SnesAdr(target, 0x007fff));
}

/**
 * check RatsSearch method
 */
static bool m1(const uint8* ptr, const uint32 len)
{
	if(0x4533 == read16(ptr)) return true;
	return false;
}
TEST(RomFile, RatsSearch)
{
	const uint8* RatsTag1 = (uint8*)"STAR\x00\x01\xff\xfe\x11\x22";
	const uint32 RatsTag1Ptr = 0x818300;
	const uint8* RatsTag2 = (uint8*)"STAR\x00\x01\xff\xfe\x33\x45";
	const uint32 RatsTag2Ptr = 0x85a123;
	uint8* ptr;

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* empty */
	LONGS_EQUAL(ROMADDRESS_NULL, target->RatsSearch(target, 0x8000, NULL));

	/* genetate rats tag */
	ptr = target->GetSnesPtr(target, 0x8010);
	memcpy(ptr, "STR", 3);
	LONGS_EQUAL(0, memcmp("STR", ptr, 3));

	ptr = target->GetSnesPtr(target, RatsTag1Ptr);
	memcpy(ptr, RatsTag1, 10);
	LONGS_EQUAL(0, memcmp(RatsTag1, ptr, 10));

	ptr = target->GetSnesPtr(target, RatsTag2Ptr);
	memcpy(ptr, RatsTag2, 10);
	LONGS_EQUAL(0, memcmp(RatsTag2, ptr, 10));

	/* search incremental */
	LONGS_EQUAL(RatsTag1Ptr, target->RatsSearch(target, 0x8000, NULL));
	LONGS_EQUAL(RatsTag2Ptr, target->RatsSearch(target, 0x858000, NULL));
	LONGS_EQUAL(ROMADDRESS_NULL, target->RatsSearch(target, 0x85a200, NULL));

	/* search cond */
	LONGS_EQUAL(RatsTag2Ptr, target->RatsSearch(target, 0x8000, m1));
}

/**
 * check RatsClean method
 */
TEST(RomFile, RatsClean)
{
	uint8* ptr;

	const uint8* RatsTag = (uint8*)"STAR\x00\x01\xff\xfe\x11\x22";
	const uint8 Cleaned[10] = { FILL };
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* genetate rats tag */
	ptr = target->GetSnesPtr(target, 0x8025);
	memcpy(ptr, RatsTag, 10);
	LONGS_EQUAL(0, memcmp(RatsTag, ptr, 10));

	/* clean check */
	CHECK(target->RatsClean(target, 0x8025));
	LONGS_EQUAL(0, memcmp(Cleaned, ptr, 10));

	CHECK_FALSE(target->RatsClean(target, 0x8025));

}

/**
 * check UseHiRomMapSA1 method
 */
TEST(RomFile, UseHiRomMapSA1)
{
	/* Nothing to do here */
}

TEST_GROUP(RomFile_SA1)
{
	/* test target */
	RomFile* target;

	void setup()
	{
		FILE *f;
		uint16 uitmp = 0xffff;
		uint8 map = 0x23;

		/* create LoRom file */
		f = fopen(TestRoot TestFile, "wb");
		fseek(f, 0x48ffff, SEEK_SET);
		fwrite(&map, 1, 1, f);		/* write dummy data */
		fseek(f, 0x7fd5, SEEK_SET);
		fwrite(&map, 1, 1, f);		/* write mapmode */
		fseek(f, 0x7fdc, SEEK_SET);
		fwrite(&uitmp, 2, 1, f);	/* write dummy sum */
		fclose(f);

		/* create check object */
		target = new_RomFile(TestRoot TestFile);
	}

	void teardown()
	{
		delete_RomFile(&target);
		remove(TestRoot TestFile);
	}
};

TEST(RomFile_SA1, Snes2PcAdr)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* LoRom Map */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0x8000));
	LONGS_EQUAL(0x100000, target->Snes2PcAdr(target, 0x208000));
	LONGS_EQUAL(0x200000, target->Snes2PcAdr(target, 0x808000));
	LONGS_EQUAL(0x300000, target->Snes2PcAdr(target, 0xa08000));

	/* HiRom Map */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(0x100000, target->Snes2PcAdr(target, 0xd00000));
	LONGS_EQUAL(0x200000, target->Snes2PcAdr(target, 0xe00000));
	LONGS_EQUAL(0x300000, target->Snes2PcAdr(target, 0xf00000));

	/* out of map */
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x408000));
}

TEST(RomFile_SA1, Pc2SnesAdr)
{
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* LoRom Map */
	LONGS_EQUAL(0x008000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0x208000, target->Pc2SnesAdr(target, 0x100000));
	LONGS_EQUAL(0x808000, target->Pc2SnesAdr(target, 0x200000));
	LONGS_EQUAL(0xa08000, target->Pc2SnesAdr(target, 0x300000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x400000));

	/* HiRom Map */
	target->UseHiRomMapSA1(target, true);
	LONGS_EQUAL(0xc00000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0xd00000, target->Pc2SnesAdr(target, 0x100000));
	LONGS_EQUAL(0xe00000, target->Pc2SnesAdr(target, 0x200000));
	LONGS_EQUAL(0xf00000, target->Pc2SnesAdr(target, 0x300000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x400000));
}

TEST_GROUP(RomFile_MemMaps)
{
	/* test target */
	RomFile* target;

	void setup()
	{
		/* create check object */
		target = new_RomFile(TestRoot TestFile);
	}

	void teardown()
	{
		delete_RomFile(&target);
		remove(TestRoot TestFile);
	}
};

TEST(RomFile_MemMaps, HiRomNoHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x21;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0xffff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0xffd5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0xffdc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(0x00ffff, target->Snes2PcAdr(target, 0xc0ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc10000));

	LONGS_EQUAL(0xc00000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0xc0ffff, target->Pc2SnesAdr(target, 0x00ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x010000));
}

TEST(RomFile_MemMaps, HiRomHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x21;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0x101ff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0x101d5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0x101dc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(0x00ffff, target->Snes2PcAdr(target, 0xc0ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc10000));

	LONGS_EQUAL(0xc00000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0xc0ffff, target->Pc2SnesAdr(target, 0x00ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x010000));
}

TEST(RomFile_MemMaps, SPC7110RomNoHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x3a;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0xffff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0xffd5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0xffdc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc0ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc10000));

	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x00ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x010000));
}

TEST(RomFile_MemMaps, SPC7110RomHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x3a;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0x101ff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0x101d5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0x101dc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc0ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0xc10000));

	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x00ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x010000));
}

TEST(RomFile_MemMaps, ExLoRomNoHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x25;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0x407fff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0x407fd5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0x407fdc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0x808000));
	LONGS_EQUAL(0x007fff, target->Snes2PcAdr(target, 0x80ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x810000));
	LONGS_EQUAL(0x400000, target->Snes2PcAdr(target, 0x008000));

	LONGS_EQUAL(0x808000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0x80ffff, target->Pc2SnesAdr(target, 0x007fff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x410000));
	LONGS_EQUAL(0x008000, target->Pc2SnesAdr(target, 0x400000));
}

TEST(RomFile_MemMaps, ExLoRomHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x25;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0x4081ff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0x4081d5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0x4081dc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0x808000));
	LONGS_EQUAL(0x007fff, target->Snes2PcAdr(target, 0x80ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x810000));
	LONGS_EQUAL(0x400000, target->Snes2PcAdr(target, 0x008000));

	LONGS_EQUAL(0x808000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0x80ffff, target->Pc2SnesAdr(target, 0x007fff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x410000));
	LONGS_EQUAL(0x008000, target->Pc2SnesAdr(target, 0x400000));
}

TEST(RomFile_MemMaps, ExHiRomNoHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x25;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0x40ffff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0x40ffd5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0x40ffdc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(0x00ffff, target->Snes2PcAdr(target, 0xc0ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x410000));
	LONGS_EQUAL(0x400000, target->Snes2PcAdr(target, 0x400000));

	LONGS_EQUAL(0xc00000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0xc0ffff, target->Pc2SnesAdr(target, 0x00ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x410000));
	LONGS_EQUAL(0x400000, target->Pc2SnesAdr(target, 0x400000));
}

TEST(RomFile_MemMaps, ExHiRomHeader)
{
	FILE *f;
	uint16 sumtmp = 0xffff;
	uint8 map = 0x25;

	/* create LoRom file */
	f = fopen(TestRoot TestFile, "wb");
	fseek(f, 0x4101ff, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write dummy data */
	fseek(f, 0x4101d5, SEEK_SET);
	fwrite(&map, 1, 1, f);		/* write mapmode */
	fseek(f, 0x4101dc, SEEK_SET);
	fwrite(&sumtmp, 2, 1, f);	/* write dummy sum */
	fclose(f);

	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	/* check memmap method */
	LONGS_EQUAL(0x000000, target->Snes2PcAdr(target, 0xc00000));
	LONGS_EQUAL(0x00ffff, target->Snes2PcAdr(target, 0xc0ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Snes2PcAdr(target, 0x410000));
	LONGS_EQUAL(0x400000, target->Snes2PcAdr(target, 0x400000));

	LONGS_EQUAL(0xc00000, target->Pc2SnesAdr(target, 0x000000));
	LONGS_EQUAL(0xc0ffff, target->Pc2SnesAdr(target, 0x00ffff));
	LONGS_EQUAL(ROMADDRESS_NULL, target->Pc2SnesAdr(target, 0x410000));
	LONGS_EQUAL(0x400000, target->Pc2SnesAdr(target, 0x400000));
}

TEST_GROUP(RomFile_Error)
{
	/* test target */
	RomFile* target;

	void setup()
	{
	}

	void teardown()
	{
		delete_RomFile(&target);
	}
};

TEST(RomFile_Error, NoFile)
{
	/* create check object */
	target = new_RomFile(TestRoot "hoge");
	LONGS_EQUAL(FileOpen_CantAccess, target->Open(target));
}

TEST(RomFile_Error, UnknownFile)
{
	/* create check object */
	target = new_RomFile("/dev/null");
	LONGS_EQUAL(FileOpen_NoError, target->Open(target));

	LONGS_EQUAL(RomType_Unknown, target->type_get(target));
}
