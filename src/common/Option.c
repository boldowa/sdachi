/**
 * Option.c
 */
#include "common/types.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common/Str.h"
#include "common/Funex.h"
#include "common/puts.h"
#include "common/Option.h"

typedef bool (*SetOptionFunc_t)(void* param, const char* arg);

/* private ====================================*/
static char* type2str_tbl[] = {
	NULL,		/* OptionType_Unknown */
	" bool ",	/* OptionType_Bool */
	" int  ",	/* OptionType_Int */
	"float ",	/* OptionType_Float */
	"string",	/* OptionType_String */
	"string",	/* OptionType_FunctionString */

	/*==============================================*/
	NULL,		/* OptionType_Term */
};

static void RemoveArg(int* argcp, char*** argvp, const int inx)
{
	int i,j;
	int argc = *argcp;
	char** argv = *argvp;

	for(i = inx, j = inx+1; j<argc; i++,j++)
	{
		argv[i] = argv[j];
	}

	(*argcp)--;
}

static bool isValidHex(const char* s)
{
	int i;

	for(i=0; '\0' != s[i]; i++)
	{
		if(!ishex(s[i])) return false;
	}
	return true;
}

static bool isValidNumber(const char* s)
{
	int i;
	bool dot = false;

	for(i=0; '\0' != s[i]; i++)
	{
		if(!isdigit(s[i]))
		{
			if('.' != s[i])
			{
				return false;
			}
			if(true == dot)
			{
				return false;
			}

			dot = true;
		}
	}
	return true;
}
static bool SetOpt_Bool(void* param, const char* arg)
{
	bool* b = (bool*)param;

	(*b) = !(*b);
	return true;
}
static bool SetOpt_Int(void* param, const char* arg)
{
	int* i = (int*)param;
	if(0 == strncmp("0x", arg, 2))
	{
		if(true == isValidHex(&arg[2]))
		{
			(*i) = atoh(&arg[2]);
			return true;
		}
	}

	if(false == isValidNumber(arg))
	{
		return false;
	}
	(*i) = (int)atoi(arg);
	return true;
}
static bool SetOpt_Float(void* param, const char* arg)
{
	float* f = (float*)param;
	if(false == isValidNumber(arg))
	{
		return false;
	}

	(*f) = (float)atof(arg);
	return true;
}
static bool SetOpt_String(void* param, const char* arg)
{
	const char** s = (const char**)param;

	(*s) = arg;
	return true;
}
static bool SetOpt_FunctionString(void* param, const char* arg)
{
	SetOptStruct* st;
	st = (SetOptStruct*)param;
	return st->func(st->dest, arg);
}

static SetOptionFunc_t SetOption[] = {
	NULL, /* OptionType_Unknown */
	SetOpt_Bool, /* OptionType_Bool */
	SetOpt_Int, /* OptionType_Int */
	SetOpt_Float, /* OptionType_Float */
	SetOpt_String, /* OptionType_String */
	SetOpt_FunctionString, /* OptionType_String */

	/*==============================================*/
	NULL, /* OptionType_Term */
};
/* public =====================================*/
bool Option_Parse(int* argcp, char*** argvp, OptionStruct* opt)
{
	int	inx_opt = 1;

	int	argc = *argcp;
	char**	argv = *argvp;
	OptionStruct *o;

	char*  cmd;
	bool detected;

	if((OptionType_Term + 1) > (sizeof(SetOption) / sizeof(SetOptionFunc_t)))
	{
		putfatal("%s: Program error: Option parsers are few.", __func__);
		return false;
	}

	for(inx_opt = 1; inx_opt < argc; )
	{
		cmd = argv[inx_opt];
		detected = false;
		if('-' == cmd[0])
		{
			for(o=opt; o->type!=OptionType_Term; o++)
			{
				/* option check */
				if('-' == cmd[1])
				{
					/* Long option */
					if(0 == strcmp(&cmd[2], o->name))
					{
						detected = true;
					}
				}
				else
				{
					/* Short option */
					if(o->sname == cmd[1] && '\0' == cmd[2])
					{
						detected = true;
					}
				}

				/* Apply option */
				if(true == detected)
				{
					if(NULL == SetOption[o->type])
					{
						putfatal("Invalid option type");
						return false;
					}

					RemoveArg(&argc, &argv, inx_opt);
					if(OptionType_Bool == o->type)
					{
						SetOption[o->type](o->val, NULL);
						break;
					}

					if(inx_opt >= argc)
					{
						puterror("Argument is missing for \"%s\" option", cmd);
						return false;
					}
					if(false == SetOption[o->type](o->val, argv[inx_opt]))
					{
						puterror("Invalid parameter for \"%s\" option: %s", cmd, argv[inx_opt]);
						return false;
					}
					RemoveArg(&argc, &argv, inx_opt);
					break;
				} /* detected */
			} /* OptionStruct loop */
			if(false == detected)
			{
				puterror("Unknown option: \"%s\"", cmd);
				return false;
			}
		} /* Option check */
		else
		{
			/* Not option */
			inx_opt++;
		}
	}

	*argcp = argc;
	return true;
}

void Option_Usage(const OptionStruct* opt)
{
	printf("Options:\n");
	for(; OptionType_Term != opt->type; opt++)
	{
		printf("  -%c, --%-10s [%s] %.50s\n", opt->sname, opt->name, type2str_tbl[opt->type], opt->desc);
	}
}

