typedef struct LGc	LGc;
typedef struct LEnvHash	LEnvHash;
typedef struct LEnv	LEnv;
typedef struct LVal	LVal;
typedef struct LuftVM	LuftVM;

#pragma incomplete LEnvHash
#pragma incomplete LEnv

/* Gc types */
enum
{
	LGCVAL,
	LGCENV,
};

/* LVal types */
enum
{
	TSYMBOL,
	TNUMBER,
	TLIST,
	TPROC,
	TLAMBDA,
};

struct LGc
{
	char	gctype;
	char	gcmark;
	LGc*	gclink;
};

struct LVal
{
	LGc;

	int type;
	LEnv *env;

	/* TNUMBER */
	vlong i;

	/* TSYMBOL */
	char *s;

	/* TPROC */
	LVal *(*proc)(LuftVM *, LVal *);

	/* TLIST */
	int len;
	LVal **list;
};

struct LuftVM
{
	jmp_buf errjmp;
	char errstr[ERRMAX];
	LGc *gcl;

	/* input state */
	char *input;
	int inlen;
	int inrp;
	int inln;
	LVal *inval;

	LEnv *env;
};

#pragma	varargck	type "V"	LVal*
#pragma	varargck	type "E"	LVal*

/* luft public api */
LuftVM *luftvm(void);
void luftproc(LuftVM *L, char *sym, LVal *(*proc)(LuftVM*, LVal*));
int luftdo(LuftVM *L, char *code, int len);
void lufterr(LuftVM *L, char *fmt, ...);
char *lufterrstr(LuftVM *L);

