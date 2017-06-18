/**
 * sdachi.c
 */
#include "common/types.h"
#include "common/Option.h"
#include "file/FilePath.h"
#include "file/File.h"
#include "file/TextFile.h"
#include "file/RomFile.h"
#include "sdachi/DisAsm.h"
#include "sdachi/version.h"

static void ShowUsage(const char* pg, const OptionStruct* opt)
{
	printf("Usage: %s [options] <rom>\n", pg);
	Option_Usage(opt);
}

static void ShowVersion()
{
	printf("%s v%.2f \n", AppName, AppVersion);
	printf("  by boldowa\n");
	printf("  since    : May 28 2017\n");
	printf("  compiled : %s\n", __DATE__);
}

static bool DisassembleRom(const char* rompath, DisAsmInf* inf, bool (*dis)(RomFile*, TextFile*, DisAsmInf*))
{
	RomFile* from;
	TextFile* fasm;
	FilePath* fpath;
	bool result;

	from = new_RomFile(rompath);

	if(NULL == inf->outputPath)
	{
		fpath = new_FilePath(rompath);
		fpath->ext_set(fpath, ".asm");
		fasm = new_TextFile(fpath->path_get(fpath));
		delete_FilePath(&fpath);
	}
	else
	{
		fasm = new_TextFile(inf->outputPath);
	}

	if(FileOpen_NoError != from->Open(from)
	|| FileOpen_NoError != fasm->Open2(fasm, "w"))
	{
		delete_RomFile(&from);
		delete_TextFile(&fasm);
		return false;
	}

	result = dis(from, fasm, inf);

	delete_RomFile(&from);
	fasm->super.Close(&fasm->super);
	/*if(!result)
	{
		remove(fasm->super.path_get(&fasm->super));
	}
	else*/
	{
		printf("Output: %s\n", fasm->super.path_get(&fasm->super));
	}
	delete_TextFile(&fasm);

	return result;
}

int main(int argc, char** argv)
{
	/* options */
	DisAsmInf disinf = {
		false, false,
		-1,
		16, 0, "", 3,
		NULL, false
	};
	bool showVersion = false;
	bool showHelp = false;

	bool  result;

	/* command-line options */
	OptionStruct options[] = {
		{ "a", 'a', "16bit accumlator", OptionType_Bool, &disinf.accum16bits },
		{ "x", 'x', "16bit index register", OptionType_Bool, &disinf.index16bits },
		{ "pc", 'p', "Specify program counter(SNES Address)", OptionType_Int, &disinf.progCounter },
		{ "recursive", 'r', "Specify recursive depth max(default: 3)", OptionType_Int, &disinf.depthMax },
		{ "count", 'c', "Data counts(enable data mode / default: 0)", OptionType_Int, &disinf.dataCount },
		{ "split", 's', "Data splits(default: 16)", OptionType_Int, &disinf.dataSplits },
		{ "label", 'l', "Specify data mode label", OptionType_String, &disinf.dataLabel },
		{ "upper", 'u', "Enable upper case", OptionType_Bool, &disinf.enableUpper },
		{ "output", 'o', "Specify output file(default: <rom>.asm)", OptionType_String, &disinf.outputPath },
		{ "version", 'v', "show version", OptionType_Bool, &showVersion },
		{ "help", '?', "show help message", OptionType_Bool, &showHelp },
		/* term */
		{ NULL, '\0', NULL, OptionType_Term, NULL },
	};

	if(!Option_Parse(&argc, &argv, options))
	{
		return -1;
	}

	/* Show help / version */
	if(true == showVersion)
	{
		ShowVersion();
	}
	if(true == showHelp)
	{
		ShowUsage(argv[0], options);
	}
	if((true == showVersion) || (true == showHelp))
	{
		return 0;
	}

	if(argc != 2)
	{
		printf("Usage: %s [options] <rom>\n", argv[0]);
		printf("Please try '-?' or '--help' option, and you can get more information.\n");
		return 0;
	}

	result = DisassembleRom(argv[1], &disinf, DisAsm);

	if(false == result)
	{
		printf("Failed...\n");
		return -1;
	}

	printf("Succeeded.\n");
	return 0;
}
