/**
 * DisAsm.c
 */
#include "common/types.h"
#include <assert.h>
#include <stdarg.h>
#include "common/puts.h"
#include "common/Str.h"
#include "common/List.h"
#include "common/Option.h"
#include "common/ReadWrite.h"
#include "file/FilePath.h"
#include "file/File.h"
#include "file/TextFile.h"
#include "file/RomFile.h"
#include "sdachi/DisAsm.h"

typedef struct _SnesRegisters {
	uint8		db;
	uint16		d;
	uint32		pc;
	uint16		a;
	uint16		x;
	uint16		y;
	uint16		psw;
	uint16		sp;

	/* stack info */
	uint32		callFrom;
	int		depth;
} SnesRegisters;
static void SnesRegistersCleaner(void* ptr)
{
	free(ptr);
}

typedef union _UniAdr {
	uint32		abl;
	uint16		abs;
	int16		rell;
	int8		rel;
}UniAdr;

/* for pass1 */
typedef struct _OpStruct {
	uint8		op;
	int		arglen;
	uint8		arg[4];
	uint32		snesadr;
	uint32		pcadr;
	SnesRegisters	*regs;
} OpStruct;
static void OpStructCleaner(void* ptr)
{
	free(((OpStruct*)ptr)->regs);
	free(ptr);
}
typedef enum {
	Pass1_NoError,
	Pass1_InvalidPointer,
} Pass1Result;

typedef enum {
	AddOpcode_NoError,
	AddOpcode_Exists,
} AddOpcodeResult;


typedef enum _AdrMode {
	Adr_imm,
	Adr_immM,
	Adr_immX,
	Adr_sr,
	Adr_dp,
	Adr_dpx,
	Adr_dpy,
	Adr_idp,
	Adr_idx,
	Adr_idy,
	Adr_idl,
	Adr_idly,
	Adr_isy,
	Adr_abs,
	Adr_abx,
	Adr_aby,
	Adr_abl,
	Adr_alx,
	Adr_ind,
	Adr_iax,
	Adr_ial,
	Adr_rel,
	Adr_rell,
	Adr_bm,
	Adr_none
} AdrMode;

static const int argLength[] = {
	1,  /* Adr_imm */
	-1, /* Adr_immM */
	-1, /* Adr_immX */
	1,  /* Adr_sr */
	1,  /* Adr_dp */
	1,  /* Adr_dpx */
	1,  /* Adr_dpy */
	1,  /* Adr_idp */
	1,  /* Adr_idx */
	1,  /* Adr_idy */
	1,  /* Adr_idl */
	1,  /* Adr_idly */
	1,  /* Adr_isy */
	2,  /* Adr_abs */
	2,  /* Adr_abx */
	2,  /* Adr_aby */
	3,  /* Adr_abl */
	3,  /* Adr_alx */
	2,  /* Adr_ind */
	2,  /* Adr_iax */
	3,  /* Adr_ial */
	1,  /* Adr_rel */
	2,  /* Adr_rell */
	2,  /* Adr_bm */
	0,  /* Adr_none */
};


typedef struct _Opcode {
	char	*op;
	AdrMode	mode;
} Opcode;
static Opcode opcodes [] = {
	/* 0x00 */
	{ "brk",	Adr_none },	/* 0x00 */
	{ "ora",	Adr_idx  },	/* 0x01 */
	{ "cop",	Adr_imm  },	/* 0x02 */
	{ "ora",	Adr_sr   },	/* 0x03 */
	{ "tsb",	Adr_dp   },	/* 0x04 */
	{ "ora",	Adr_dp   },	/* 0x05 */
	{ "asl",	Adr_dp   },	/* 0x06 */
	{ "ora",	Adr_idl  },	/* 0x07 */
	{ "php",	Adr_none },	/* 0x08 */
	{ "ora",	Adr_immM },	/* 0x09 */
	{ "asl",	Adr_none },	/* 0x0A */
	{ "phd",	Adr_none },	/* 0x0B */
	{ "tsb",	Adr_abs  },	/* 0x0C */
	{ "ora",	Adr_abs  },	/* 0x0D */
	{ "asl",	Adr_abs  },	/* 0x0E */
	{ "ora",	Adr_abl  },	/* 0x0F */
	/* 0x10 */
	{ "bpl",	Adr_rel  },	/* 0x10 */
	{ "ora",	Adr_idy  },	/* 0x11 */
	{ "ora",	Adr_idp  },	/* 0x12 */
	{ "ora",	Adr_isy  },	/* 0x13 */
	{ "trb",	Adr_dp   },	/* 0x14 */
	{ "ora",	Adr_dpx  },	/* 0x15 */
	{ "asl",	Adr_dpx  },	/* 0x16 */
	{ "ora",	Adr_idly },	/* 0x17 */
	{ "clc",	Adr_none },	/* 0x18 */
	{ "ora",	Adr_aby  },	/* 0x19 */
	{ "inc",	Adr_none },	/* 0x1A */
	{ "tcs",	Adr_none },	/* 0x1B */
	{ "trb",	Adr_abs  },	/* 0x1C */
	{ "ora",	Adr_abx  },	/* 0x1D */
	{ "asl",	Adr_abx  },	/* 0x1E */
	{ "ora",	Adr_alx  },	/* 0x1F */
	/* 0x20 */
	{ "jsr",	Adr_abs  },	/* 0x20 */
	{ "and",	Adr_idx  },	/* 0x21 */
	{ "jsl",	Adr_abl  },	/* 0x22 */
	{ "and",	Adr_sr   },	/* 0x23 */
	{ "bit",	Adr_dp   },	/* 0x24 */
	{ "and",	Adr_dp   },	/* 0x25 */
	{ "rol",	Adr_dp   },	/* 0x26 */
	{ "and",	Adr_idl  },	/* 0x27 */
	{ "plp",	Adr_none },	/* 0x28 */
	{ "and",	Adr_immM },	/* 0x29 */
	{ "rol",	Adr_none },	/* 0x2A */
	{ "pld",	Adr_none },	/* 0x2B */
	{ "bit",	Adr_abs  },	/* 0x2C */
	{ "and",	Adr_abs  },	/* 0x2D */
	{ "rol",	Adr_abs  },	/* 0x2E */
	{ "and",	Adr_abl  },	/* 0x2F */
	/* 0x30 */
	{ "bmi",	Adr_rel  },	/* 0x30 */
	{ "and",	Adr_idy  },	/* 0x31 */
	{ "and",	Adr_idp  },	/* 0x32 */
	{ "and",	Adr_isy  },	/* 0x33 */
	{ "bit",	Adr_dpx  },	/* 0x34 */
	{ "and",	Adr_dpx  },	/* 0x35 */
	{ "rol",	Adr_dpx  },	/* 0x36 */
	{ "and",	Adr_idly },	/* 0x37 */
	{ "sec",	Adr_none },	/* 0x38 */
	{ "and",	Adr_aby  },	/* 0x39 */
	{ "dec",	Adr_none },	/* 0x3A */
	{ "tsc",	Adr_none },	/* 0x3B */
	{ "bit",	Adr_abx  },	/* 0x3C */
	{ "and",	Adr_abx  },	/* 0x3D */
	{ "rol",	Adr_abx  },	/* 0x3E */
	{ "and",	Adr_alx  },	/* 0x3F */
	/* 0x40 */
	{ "rti",	Adr_none },	/* 0x40 */
	{ "eor",	Adr_idx  },	/* 0x41 */
	{ "wdm",	Adr_none },	/* 0x42 */
	{ "eor",	Adr_sr   },	/* 0x43 */
	{ "mvp",	Adr_bm   },	/* 0x44 */
	{ "eor",	Adr_dp   },	/* 0x45 */
	{ "lsr",	Adr_dp   },	/* 0x46 */
	{ "eor",	Adr_idl  },	/* 0x47 */
	{ "pha",	Adr_none },	/* 0x48 */
	{ "eor",	Adr_immM },	/* 0x49 */
	{ "lsr",	Adr_none },	/* 0x4A */
	{ "phk",	Adr_none },	/* 0x4B */
	{ "jmp",	Adr_abs  },	/* 0x4C */
	{ "eor",	Adr_abs  },	/* 0x4D */
	{ "lsr",	Adr_abs  },	/* 0x4E */
	{ "eor",	Adr_abl  },	/* 0x4F */
	/* 0x50 */
	{ "bvc",	Adr_rel  },	/* 0x50 */
	{ "eor",	Adr_idy  },	/* 0x51 */
	{ "eor",	Adr_idp  },	/* 0x52 */
	{ "eor",	Adr_isy  },	/* 0x53 */
	{ "mvn",	Adr_bm   },	/* 0x54 */
	{ "eor",	Adr_dpx  },	/* 0x55 */
	{ "lsr",	Adr_dpx  },	/* 0x56 */
	{ "eor",	Adr_idly },	/* 0x57 */
	{ "cli",	Adr_none },	/* 0x58 */
	{ "eor",	Adr_aby  },	/* 0x59 */
	{ "phy",	Adr_none },	/* 0x5A */
	{ "tcd",	Adr_none },	/* 0x5B */
	{ "jml",	Adr_abl  },	/* 0x5C */
	{ "eor",	Adr_abx  },	/* 0x5D */
	{ "lsr",	Adr_abx  },	/* 0x5E */
	{ "eor",	Adr_alx  },	/* 0x5F */
	/* 0x60 */
	{ "rts",	Adr_none },	/* 0x60 */
	{ "adc",	Adr_idx  },	/* 0x61 */
	{ "per",	Adr_rell },	/* 0x62 */
	{ "adc",	Adr_sr   },	/* 0x63 */
	{ "stz",	Adr_dp   },	/* 0x64 */
	{ "adc",	Adr_dp   },	/* 0x65 */
	{ "ror",	Adr_dp   },	/* 0x66 */
	{ "adc",	Adr_idl  },	/* 0x67 */
	{ "pla",	Adr_none },	/* 0x68 */
	{ "adc",	Adr_immM },	/* 0x69 */
	{ "ror",	Adr_none },	/* 0x6A */
	{ "rtl",	Adr_none },	/* 0x6B */
	{ "jmp",	Adr_ind  },	/* 0x6C */
	{ "adc",	Adr_abs  },	/* 0x6D */
	{ "ror",	Adr_abs  },	/* 0x6E */
	{ "adc",	Adr_abl  },	/* 0x6F */
	/* 0x70 */
	{ "bvs",	Adr_rel  },	/* 0x70 */
	{ "adc",	Adr_idy  },	/* 0x71 */
	{ "adc",	Adr_idp  },	/* 0x72 */
	{ "adc",	Adr_isy  },	/* 0x73 */
	{ "stz",	Adr_dpx  },	/* 0x74 */
	{ "adc",	Adr_dpx  },	/* 0x75 */
	{ "ror",	Adr_dpx  },	/* 0x76 */
	{ "adc",	Adr_idly },	/* 0x77 */
	{ "sei",	Adr_none },	/* 0x78 */
	{ "adc",	Adr_immM },	/* 0x79 */
	{ "ply",	Adr_none },	/* 0x7A */
	{ "tdc",	Adr_none },	/* 0x7B */
	{ "jmp",	Adr_ial  },	/* 0x7C */
	{ "adc",	Adr_abx  },	/* 0x7D */
	{ "ror",	Adr_abx  },	/* 0x7E */
	{ "adc",	Adr_alx  },	/* 0x7F */
	/* 0x80 */
	{ "bra",	Adr_rel  },	/* 0x80 */
	{ "sta",	Adr_idx  },	/* 0x81 */
	{ "brl",	Adr_rell },	/* 0x82 */
	{ "sta",	Adr_sr   },	/* 0x83 */
	{ "sty",	Adr_dp   },	/* 0x84 */
	{ "sta",	Adr_dp   },	/* 0x85 */
	{ "stx",	Adr_dp   },	/* 0x86 */
	{ "sta",	Adr_idl  },	/* 0x87 */
	{ "dey",	Adr_none },	/* 0x88 */
	{ "bit",	Adr_immM },	/* 0x89 */
	{ "txa",	Adr_none },	/* 0x8A */
	{ "phb",	Adr_none },	/* 0x8B */
	{ "sty",	Adr_abs  },	/* 0x8C */
	{ "sta",	Adr_abs  },	/* 0x8D */
	{ "stx",	Adr_abs  },	/* 0x8E */
	{ "sta",	Adr_abl  },	/* 0x8F */
	/* 0x90 */
	{ "bcc",	Adr_rel  },	/* 0x90 */
	{ "sta",	Adr_idy  },	/* 0x91 */
	{ "sta",	Adr_idp  },	/* 0x92 */
	{ "sta",	Adr_isy  },	/* 0x93 */
	{ "sty",	Adr_dpx  },	/* 0x94 */
	{ "sta",	Adr_dpx  },	/* 0x95 */
	{ "stx",	Adr_dpy  },	/* 0x96 */
	{ "sta",	Adr_idly },	/* 0x97 */
	{ "tya",	Adr_none },	/* 0x98 */
	{ "sta",	Adr_aby  },	/* 0x99 */
	{ "txs",	Adr_none },	/* 0x9A */
	{ "txy",	Adr_none },	/* 0x9B */
	{ "stz",	Adr_abs  },	/* 0x9C */
	{ "sta",	Adr_abx  },	/* 0x9D */
	{ "stz",	Adr_abx  },	/* 0x9E */
	{ "sta",	Adr_alx  },	/* 0x9F */
	/* 0xA0 */
	{ "ldy",	Adr_immX  },	/* 0xA0 */
	{ "lda",	Adr_idx  },	/* 0xA1 */
	{ "ldx",	Adr_immX  },	/* 0xA2 */
	{ "lda",	Adr_sr   },	/* 0xA3 */
	{ "ldy",	Adr_dp   },	/* 0xA4 */
	{ "lda",	Adr_dp   },	/* 0xA5 */
	{ "ldx",	Adr_dp   },	/* 0xA6 */
	{ "lda",	Adr_idl  },	/* 0xA7 */
	{ "tay",	Adr_none },	/* 0xA8 */
	{ "lda",	Adr_immM  },	/* 0xA9 */
	{ "tax",	Adr_none },	/* 0xAA */
	{ "plb",	Adr_none },	/* 0xAB */
	{ "ldy",	Adr_abs  },	/* 0xAC */
	{ "lda",	Adr_abs  },	/* 0xAD */
	{ "ldx",	Adr_abs  },	/* 0xAE */
	{ "lda",	Adr_abl  },	/* 0xAF */
	/* 0xB0 */
	{ "bcs",	Adr_rel  },	/* 0xB0 */
	{ "lda",	Adr_idy  },	/* 0xB1 */
	{ "lda",	Adr_idp  },	/* 0xB2 */
	{ "lda",	Adr_isy  },	/* 0xB3 */
	{ "ldy",	Adr_dpx  },	/* 0xB4 */
	{ "lda",	Adr_dpx  },	/* 0xB5 */
	{ "ldx",	Adr_dpy  },	/* 0xB6 */
	{ "lda",	Adr_idly },	/* 0xB7 */
	{ "clv",	Adr_none },	/* 0xB8 */
	{ "lda",	Adr_aby  },	/* 0xB9 */
	{ "tsx",	Adr_none },	/* 0xBA */
	{ "tyx",	Adr_none },	/* 0xBB */
	{ "ldy",	Adr_abx  },	/* 0xBC */
	{ "lda",	Adr_abx  },	/* 0xBD */
	{ "ldx",	Adr_aby  },	/* 0xBE */
	{ "lda",	Adr_alx  },	/* 0xBF */
	/* 0xC0 */
	{ "cpy",	Adr_immX  },	/* 0xC0 */
	{ "cmp",	Adr_idx  },	/* 0xC1 */
	{ "rep",	Adr_imm  },	/* 0xC2 */
	{ "cmp",	Adr_sr   },	/* 0xC3 */
	{ "cpy",	Adr_dp   },	/* 0xC4 */
	{ "cmp",	Adr_dp   },	/* 0xC5 */
	{ "dec",	Adr_dp   },	/* 0xC6 */
	{ "cmp",	Adr_idl  },	/* 0xC7 */
	{ "iny",	Adr_none },	/* 0xC8 */
	{ "cmp",	Adr_immM },	/* 0xC9 */
	{ "dex",	Adr_none },	/* 0xCA */
	{ "wai",	Adr_none },	/* 0xCB */
	{ "cpy",	Adr_abs  },	/* 0xCC */
	{ "cmp",	Adr_abs  },	/* 0xCD */
	{ "dec",	Adr_abs  },	/* 0xCE */
	{ "cmp",	Adr_abl  },	/* 0xCF */
	/* 0xD0 */
	{ "bne",	Adr_rel  },	/* 0xD0 */
	{ "cmp",	Adr_idy  },	/* 0xD1 */
	{ "cmp",	Adr_idp  },	/* 0xD2 */
	{ "cmp",	Adr_isy  },	/* 0xD3 */
	{ "pei",	Adr_idp  },	/* 0xD4 */
	{ "cmp",	Adr_dpx  },	/* 0xD5 */
	{ "dec",	Adr_dpx  },	/* 0xD6 */
	{ "cmp",	Adr_idly },	/* 0xD7 */
	{ "cld",	Adr_none },	/* 0xD8 */
	{ "cmp",	Adr_aby  },	/* 0xD9 */
	{ "phx",	Adr_none },	/* 0xDA */
	{ "stp",	Adr_none },	/* 0xDB */
	{ "jmp",	Adr_iax  },	/* 0xDC */
	{ "cmp",	Adr_abx  },	/* 0xDD */
	{ "dec",	Adr_abx  },	/* 0xDE */
	{ "cmp",	Adr_alx  },	/* 0xDF */
	/* 0xE0 */
	{ "cpx",	Adr_immX  },	/* 0xE0 */
	{ "sbc",	Adr_idx  },	/* 0xE1 */
	{ "sep",	Adr_imm  },	/* 0xE2 */
	{ "sbc",	Adr_sr   },	/* 0xE3 */
	{ "cpx",	Adr_dp   },	/* 0xE4 */
	{ "sbc",	Adr_dp   },	/* 0xE5 */
	{ "inc",	Adr_dp   },	/* 0xE6 */
	{ "sbc",	Adr_idl  },	/* 0xE7 */
	{ "inx",	Adr_none },	/* 0xE8 */
	{ "sbc",	Adr_immM },	/* 0xE9 */
	{ "nop",	Adr_none },	/* 0xEA */
	{ "xba",	Adr_none },	/* 0xEB */
	{ "cpx",	Adr_abs  },	/* 0xEC */
	{ "sbc",	Adr_abs  },	/* 0xED */
	{ "inc",	Adr_abs  },	/* 0xEE */
	{ "sbc",	Adr_abl  },	/* 0xEF */
	/* 0xF0 */
	{ "beq",	Adr_rel  },	/* 0xF0 */
	{ "sbc",	Adr_idy  },	/* 0xF1 */
	{ "sbc",	Adr_idp  },	/* 0xF2 */
	{ "sbc",	Adr_isy  },	/* 0xF3 */
	{ "pea",	Adr_abs  },	/* 0xF4 */
	{ "sbc",	Adr_dpx  },	/* 0xF5 */
	{ "inc",	Adr_dpx  },	/* 0xF6 */
	{ "sbc",	Adr_idly },	/* 0xF7 */
	{ "sed",	Adr_none },	/* 0xF8 */
	{ "sbc",	Adr_aby  },	/* 0xF9 */
	{ "plx",	Adr_none },	/* 0xFA */
	{ "xce",	Adr_none },	/* 0xFB */
	{ "jsr",	Adr_iax  },	/* 0xFC */
	{ "sbc",	Adr_abx  },	/* 0xFD */
	{ "inc",	Adr_abx  },	/* 0xFE */
	{ "sbc",	Adr_alx  },	/* 0xFF */
};

typedef struct {
	int    inx;
	char   buffer[256];
}SprintfBuffer;
static void CleanSprintfBuffer(SprintfBuffer *b)
{
	b->inx = 0;
	b->buffer[0] = '\0';	/* quick clean */
}
static void bufSprintf(SprintfBuffer *b, const char* fmt, ...)
{
	va_list vl;
	int len;

	va_start(vl, fmt);
	len = vsprintf_s(&b->buffer[b->inx], (size_t)(256-b->inx), fmt, vl);
	b->inx += len;
	va_end(vl);
}

static const char * GetMapModeString(RomFile *from)
{
	switch(from->mapmode_get(from))
	{
		case MapMode_20:
		case MapMode_20H:
			return "LoRom";

		case MapMode_21:
		case MapMode_21H:
			return "HiRom";

		case MapMode_SA1:
			return "SA-1";

		case MapMode_SPC7110:
			return "SPC7110";

		case MapMode_25:
		case MapMode_25H:
			switch(from->type_get(from))
			{
				case RomType_ExLoRom:
					return "ExLoRom";
				case RomType_ExHiRom:
					return "ExHiRom";
				default:
					break;
			}
			break;

		default:
			break;
	}

	return "Unknown";
}

static void AddAnalysysTarget(SnesRegisters* base, UniAdr adr, AdrMode mode, List* snesRegsList)
{
	SnesRegisters* regs;

	regs = malloc(sizeof(SnesRegisters));
	assert(regs);
	memcpy(regs, base, sizeof(SnesRegisters));
	regs->callFrom = base->callFrom;

	switch(mode)
	{
		case Adr_rel:
			regs->pc = (uint32)((int32)regs->pc + adr.rel);
			break;
		case Adr_rell:
			regs->pc = (uint32)((int32)regs->pc + adr.rell);
			break;
		case Adr_abs:
			regs->pc = (regs->pc & 0xff0000) + adr.abs;
			break;
		case Adr_abl:
			regs->pc = adr.abl;
			break;

		default:
			break;
	}

	snesRegsList->enqueue(snesRegsList, regs);
}

static bool SearchOpStruct(const void* a, const void* b)
{
	const OpStruct *opsA = (OpStruct*)a;
	const OpStruct *opsB = (OpStruct*)b;

	return (opsA->pcadr == opsB->pcadr);
}

static AddOpcodeResult AddOpcode(OpStruct *ops, List* opStructList)
{
	Iterator* it;
	OpStruct* o;
	uint32 begdif = 0;
	uint32 enddif = 0;

	if(opStructList->length(opStructList) != 0)
	{
		it = opStructList->begin(opStructList);
		o = (OpStruct*)it->data(it);
		begdif = (uint32)abs((int)(ops->pcadr - o->pcadr));
		if(o->pcadr > ops->pcadr)
		{
			opStructList->pushFront(opStructList,ops);
			return AddOpcode_NoError;
		}

		it = opStructList->end(opStructList);
		o = (OpStruct*)it->data(it);
		enddif = (uint32)abs((int)(ops->pcadr - o->pcadr));
		if(o->pcadr < ops->pcadr)
		{
			opStructList->enqueue(opStructList,ops);
			return AddOpcode_NoError;
		}
	}

	/* dup check */
	if(begdif < enddif)
	{
		/* search from top */
		if(NULL != opStructList->searchex(opStructList, ops, SearchOpStruct, NULL, false))
		return AddOpcode_Exists;
	}
	else
	{
		/* search from tail */
		if(NULL != opStructList->searchex(opStructList, ops, SearchOpStruct, NULL, true))
		return AddOpcode_Exists;
	}

	/* insert */
	for(it = opStructList->begin(opStructList); it != NULL; it = it->next(it))
	{
		o = it->data(it);
		if(ops->pcadr < o->pcadr) break;
	}
	if(NULL != it)
	{
		it->insert(it, ops);
	}
	else
	{
		opStructList->enqueue(opStructList,ops);
	}
	return AddOpcode_NoError;
}

static Pass1Result DisAsm_Pass1(RomFile* from, SnesRegisters* regs, List* opStructList, const int depth, const int depthMax)
{
	uint8* ptr;
	List* snesRegsList;
	uint16 pcLo = 0;
	uint16 prevPcLo = 0;
	Opcode *op;
	OpStruct *opst;
	int arglen;
	SnesRegisters* subRegs;
	UniAdr adr = {0};
	SnesRegisters *grpSts;

	/* check recursive limit */
	if((depthMax <= depth) && (0 != depthMax)) return Pass1_NoError;

	/* get data pointer */
	ptr = from->GetSnesPtr(from, regs->pc);
	if(NULL == ptr)
	{
		puterror("Invalid pointer : $%06x (call from $%06x)", regs->pc, regs->callFrom);
		return Pass1_InvalidPointer;
	}

	grpSts = malloc(sizeof(SnesRegisters));
	assert(grpSts);
	memcpy(grpSts, regs, sizeof(SnesRegisters));
	grpSts->depth = depth;

	snesRegsList = new_List(NULL, SnesRegistersCleaner);
	assert(snesRegsList);

	pcLo = (uint16)(regs->pc & 0xffff);
	while(prevPcLo <= pcLo)
	{
		opst = calloc(1, sizeof(OpStruct));
		assert(opst);

		opst->regs = grpSts;
		grpSts = NULL;
		opst->op = ptr[0];
		opst->snesadr = regs->pc;
		opst->pcadr = from->Snes2PcAdr(from, regs->pc);

		regs->callFrom = regs->pc;
		op = &opcodes[(ptr++)[0]];
		arglen = 2;	/* default: imm(16bits mode) length */
		switch(op->mode)
		{
			case Adr_immM:
				if(regs->psw & 0x20) arglen = 1;
				break;

			case Adr_immX:
				if(regs->psw & 0x10) arglen = 1;
				break;

			default:
				arglen = argLength[op->mode];
				break;
		}
		opst->arglen = arglen;
		memcpy(opst->arg, ptr, (size_t)arglen);

		/* add disassemble list */
		if(AddOpcode_Exists == AddOpcode(opst, opStructList))
		{
			return Pass1_NoError;
		}

		/* increase program counters */
		prevPcLo = pcLo;
		regs->pc = (uint32)(regs->pc+1+(uint32)arglen);
		pcLo = (uint16)(regs->pc&0xffff);
		ptr += arglen;


		/* analysys the opcode */
		adr.abl = 0;
		switch(opst->op)
		{
			/* return */
			case 0x40:	/* rti */
			case 0x60:	/* rts */
			case 0x6b:	/* rtl */
				goto ReturnRoutine;

			/* indirect / index jump */
			case 0x6c:
			case 0x7c:
			case 0xdc:
				goto ReturnRoutine;

			/* relative branch */
			case 0x10:	/* bpl */
			case 0x30:	/* bmi */
			case 0x50:	/* bvc */
			case 0x70:	/* bvs */
			case 0x90:	/* bcc */
			case 0xb0:	/* bcs */
			case 0xd0:	/* bne */
			case 0xf0:	/* beq */
				adr.rel = (int8)opst->arg[0];
				AddAnalysysTarget(regs, adr, Adr_rel, snesRegsList);
				break;

			case 0x80:	/* bra */
				adr.rel = (int8)opst->arg[0];
				AddAnalysysTarget(regs, adr, Adr_rel, snesRegsList);
				goto ReturnRoutine;

			/* relative long branch */
			case 0x82:	/* brl */
				adr.rell = (int16)read16(&opst->arg[0]);
				AddAnalysysTarget(regs, adr, Adr_rell, snesRegsList);
				goto ReturnRoutine;

			/* jump */
			case 0x4c:	/* jmp */
				adr.abs = read16(&opst->arg[0]);
				AddAnalysysTarget(regs, adr, Adr_abs, snesRegsList);
				goto ReturnRoutine;

			case 0x5c:	/* jml */
				adr.abl = read24(&opst->arg[0]);
				AddAnalysysTarget(regs, adr, Adr_abl, snesRegsList);
				goto ReturnRoutine;

			/* subroutine */
			case 0x20:	/* jsr */
				{
					uint32 pre = regs->pc;
					uint32 precf = regs->callFrom;
					Pass1Result r;
					regs->pc = (regs->pc & 0xff0000) + read16(&opst->arg[0]);
					if(Pass1_NoError != (r = DisAsm_Pass1(from, regs, opStructList, depth+1, depthMax)))
					{
						delete_List(&snesRegsList);
						return r;
					}
					regs->pc = pre;
					regs->callFrom = precf;
				}
				break;

			case 0x22:	/* jsl */
				{
					uint32 pre = regs->pc;
					uint32 precf = regs->callFrom;
					Pass1Result r;
					regs->pc = read24(&opst->arg[0]);
					if(Pass1_NoError != (r = DisAsm_Pass1(from, regs, opStructList, depth+1, depthMax)))
					{
						delete_List(&snesRegsList);
						return r;
					}
					regs->pc = pre;
					regs->callFrom = precf;
				}
				break;

			case 0xc2:	/* rep */
				regs->psw = (uint16)(regs->psw & (opst->arg[0] ^ 0xff));
				break;

			case 0xe2:	/* sep */
				regs->psw = (uint16)(regs->psw | opst->arg[0]);
				break;

			default:
				break;
		}
	}
ReturnRoutine:

	/* analysys subroutines and branches */
	subRegs = snesRegsList->dequeue(snesRegsList);
	while(NULL != subRegs)
	{
		Pass1Result r;
		r = DisAsm_Pass1(from, subRegs, opStructList, depth, depthMax);
		SnesRegistersCleaner(subRegs);

		if(r != Pass1_NoError)
		{
			delete_List(&snesRegsList);
			return r;
		}
		subRegs = snesRegsList->dequeue(snesRegsList);
	}

	/* clean */
	delete_List(&snesRegsList);
	return Pass1_NoError;
}


bool DisAsm_Pass2(TextFile* fasm, List* opStructList, bool enableUpper)
{
	OpStruct* opst;
	SprintfBuffer buf;

	opst = opStructList->dequeue(opStructList);
	while(NULL != opst)
	{
		/* puts group info */
		if(NULL != opst->regs)
		{
			fasm->Printf(fasm, "\n");
			fasm->Printf(fasm, ";-----------------------------\n");
			fasm->Printf(fasm, ";   call depth   : %d\n", opst->regs->depth);
			fasm->Printf(fasm, ";   call from    : $%06x\n", opst->regs->callFrom);
			fasm->Printf(fasm, ";   A register   : %s\n", (opst->regs->psw & 0x20) ? "8 bit" : "16 bit");
			fasm->Printf(fasm, ";   X/Y register : %s\n", (opst->regs->psw & 0x10) ? "8 bit" : "16 bit");
			fasm->Printf(fasm, ";-----------------------------\n");
		}

		CleanSprintfBuffer(&buf);
		bufSprintf(&buf, "L%06x:\t%s", opst->snesadr, opcodes[opst->op].op);
		switch(opcodes[opst->op].mode)
		{
			case Adr_imm:
				/* "...#$02        " */
				bufSprintf(&buf, "   #$%02x         ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_immM:
			case Adr_immX:
				if(2 == opst->arglen)
				{
					/* ".w.#$1234      " */
					bufSprintf(&buf, ".w #$%04x       ; %02x %02x %02x\n",
							read16(&opst->arg[0]),
							opst->op, opst->arg[0], opst->arg[1]);
					break;
				}
				/* "...#$02        " */
				bufSprintf(&buf, ".b #$%02x         ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_sr:
				/* ".b.$02, s      " */
				bufSprintf(&buf, ".b $%02x, s       ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_dp:
				/* ".b.$02         " */
				bufSprintf(&buf, ".b $%02x          ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_dpx:
				/* ".b.$02, x      " */
				bufSprintf(&buf, ".b $%02x, x       ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_dpy:
				/* ".b.$02, y      " */
				bufSprintf(&buf, ".b $%02x, y       ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_idp:
				/* ".b.($02)       " */
				bufSprintf(&buf, ".b ($%02x)        ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_idx:
				/* ".b.($02, x)    " */
				bufSprintf(&buf, ".b ($%02x, x)     ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_idy:
				/* ".b.($02), y    " */
				bufSprintf(&buf, ".b ($%02x), y     ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_idl:
				/* ".b.[$02]       " */
				bufSprintf(&buf, ".b [$%02x]        ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_idly:
				/* ".b.[$02], y    " */
				bufSprintf(&buf, ".b [$%02x], y     ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_isy:
				/* ".b.($02, s), y " */
				bufSprintf(&buf, ".b ($%02x, s), y  ; %02x %02x\n",
						opst->arg[0],
						opst->op, opst->arg[0]);
				break;

			case Adr_abs:
				/* ".w.$1234       " */
				switch(opst->op)
				{
					case 0x20:	/* jsr */
					case 0x4c:	/* jmp */
						bufSprintf(&buf, "   L%06x      ; %02x %02x %02x\n",
								((uint32)((((int32)opst->snesadr+3) & 0xff0000) + read16(&opst->arg[0]))),
								opst->op, opst->arg[0], opst->arg[1]);
						break;

					default:
						bufSprintf(&buf, ".w $%04x        ; %02x %02x %02x\n",
								read16(&opst->arg[0]),
								opst->op, opst->arg[0], opst->arg[1]);
						break;
				}
				break;

			case Adr_abx:
				/* ".w.$1234, x    " */
				bufSprintf(&buf, ".w $%04x, x     ; %02x %02x %02x\n",
						read16(&opst->arg[0]),
						opst->op, opst->arg[0], opst->arg[1]);
				break;

			case Adr_aby:
				/* ".w.$1234, y    " */
				bufSprintf(&buf, ".w $%04x, y     ; %02x %02x %02x\n",
						read16(&opst->arg[0]),
						opst->op, opst->arg[0], opst->arg[1]);
				break;

			case Adr_abl:
				/* ".l.$123456     " */
				switch(opst->op)
				{
					case 0x22:	/* jsl */
					case 0x5c:	/* jml */
						bufSprintf(&buf, "   L%06x      ; %02x %02x %02x %02x\n",
								read24(&opst->arg[0]),
								opst->op, opst->arg[0], opst->arg[1], opst->arg[2]);
						break;

					default:
						bufSprintf(&buf, ".l $%06x      ; %02x %02x %02x %02x\n",
								read24(&opst->arg[0]),
								opst->op, opst->arg[0], opst->arg[1], opst->arg[2]);
						break;
				}
				break;

			case Adr_alx:
				/* ".l.$123456, x  " */
				bufSprintf(&buf, ".l $%06x, x   ; %02x %02x %02x %02x\n",
						read24(&opst->arg[0]),
						opst->op, opst->arg[0], opst->arg[1], opst->arg[2]);
				break;

			case Adr_ind:
				/* ".w.($1234)     " */
				bufSprintf(&buf, ".w ($%04x)      ; %02x %02x %02x\n",
						read16(&opst->arg[0]),
						opst->op, opst->arg[0], opst->arg[1]);
				break;

			case Adr_iax:
				/* ".w.($1234, x)  " */
				bufSprintf(&buf, ".w ($%04x, x)   ; %02x %02x %02x\n",
						read16(&opst->arg[0]),
						opst->op, opst->arg[0], opst->arg[1]);
				break;

			case Adr_ial:
				/* ".l.[$123456]   " */
				bufSprintf(&buf, ".l [$%06x]    ; %02x %02x %02x %02x\n",
						read24(&opst->arg[0]),
						opst->op, opst->arg[0], opst->arg[1], opst->arg[2]);
				break;

			case Adr_rel:
				/* "...$02         " */
				bufSprintf(&buf, "   L%06x      ; %02x %02x\n",
						((uint32)((int32)opst->snesadr+2 + (int8)opst->arg[0])),
						opst->op, opst->arg[0]);
				break;

			case Adr_rell:
				/* "...$1234       " */
				bufSprintf(&buf, "   L%06x      ; %02x %02x %02x\n",
						((uint32)((int32)opst->snesadr+3 + (int16)read16(&opst->arg[0]))),
						opst->op, opst->arg[0], opst->arg[1]);
				break;

			case Adr_bm:
				/* "...$02, $03    " */
				bufSprintf(&buf, "   $%02x, $%02x     ; %02x %02x %02x\n",
						opst->arg[0], opst->arg[1],
						opst->op, opst->arg[0], opst->arg[1]);
				break;

			case Adr_none:
				bufSprintf(&buf, "%15s ; %02x\n", "", opst->op);
				break;
		}

		if(enableUpper)
		{
			Str_toupper(buf.buffer);
		}
		fasm->Printf(fasm, "%s", buf.buffer);

		OpStructCleaner(opst);
		opst = opStructList->dequeue(opStructList);
	}

	return true;
}


bool DisAsm(RomFile* from, TextFile* fasm, DisAsmInf* inf)
{
	uint8* ptr;
	uint32 address;

	if(RomType_Unknown == from->type_get(from))
	{
		puterror("Unknown rom type.");
		return false;
	}

	if(inf->progCounter == -1)
	{
		uint16 sa;
		ptr = from->GetSnesPtr(from,0xfffc);
		sa = read16(ptr);
		address = sa;
		ptr = from->GetSnesPtr(from,sa);
	}
	else
	{
		address = (uint32)inf->progCounter;
		ptr = from->GetSnesPtr(from, (uint32)inf->progCounter);
	}
	if(NULL == ptr)
	{
		puterror("Invalid snes address.");
		return false;
	}

	if(inf->dataCount != 0)
	{/* data mode */
		int i;
		if(0 != strcmp("", inf->dataLabel))
		{
			fasm->Printf(fasm, "%s:\n", inf->dataLabel);
		}
		for(i=0; i<inf->dataCount; i++)
		{
			int j;
			fasm->Printf(fasm, "\t.db\t");
			for(j=0; j<inf->dataSplits-1; j++)
			{
				fasm->Printf(fasm, "$%02x, ", (ptr++)[0]);
			}
			fasm->Printf(fasm, "$%02x\n", (ptr++)[0]);
		}
		return true;
	}


	{/* disasm mode */
		bool result;
		List* opStructList;
		SnesRegisters regs = {0};
		regs.psw = 0x30;

		if(-1 == inf->progCounter)
		{
			regs.callFrom = 0xfffc;
		}
		else
		{
			regs.callFrom = (uint32)inf->progCounter;
		}

		opStructList = new_List(NULL, OpStructCleaner);
		assert(opStructList);
		
		if(inf->accum16bits) regs.psw = (uint16)(regs.psw & (0x20 ^ 0xff));
		if(inf->index16bits) regs.psw = (uint16)(regs.psw & (0x10 ^ 0xff));

		regs.pc = address;
		regs.db = (uint8)(address >> 16);

		/* output asm header */
		fasm->Printf(fasm, ";-------------------------------------------------\n");
		fasm->Printf(fasm, ";  File : %s\n", fasm->super.path_get(&fasm->super));
		fasm->Printf(fasm, ";  From : %s\n", from->super.path_get(&from->super));
		fasm->Printf(fasm, ";  Map  : %s\n", GetMapModeString(from));
		fasm->Printf(fasm, ";-------------------------------------------------\n");

		/* Pass1 : Generate disassemble list */
		result = true;
		if(Pass1_NoError != DisAsm_Pass1(from, &regs, opStructList, 0, inf->depthMax))
		{
			result = false;
		}

		/* Pass2 : Write to asm file */
		result &= DisAsm_Pass2(fasm, opStructList, inf->enableUpper);

		/* clean */
		delete_List(&opStructList);
		return result;
	}
}
