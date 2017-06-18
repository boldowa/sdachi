#pragma once
/**
 * DisAsm.c
 */

typedef struct _DisAsmInf {
	bool  accum16bits;
	bool  index16bits;
	int   progCounter;
	int   dataSplits;
	int   dataCount;
	char* dataLabel;
	int   depthMax;
	const char* outputPath;
	bool  enableUpper;
} DisAsmInf;

bool DisAsm(RomFile* from, TextFile* fasm, DisAsmInf* inf);

