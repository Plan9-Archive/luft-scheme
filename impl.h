/* LEnv table size */
enum
{
	LUFTHASHSZ	= 128,
};

struct LEnvHash
{
	char name[64];
	LVal *val;
	LEnvHash *next;
};

struct LEnv
{
	LGc;
	LuftVM *L;
	LEnvHash *hash[LUFTHASHSZ];
	LEnv *parent;
};

/* parse.c */
char *ltypename(int ty);
int Vfmt(Fmt *f);
LVal *luftparse(LuftVM *L, char *code, int len);
LVal *lufteval(LuftVM *L, LVal *val, LEnv *env);

/* lval.c */
void ldel(LuftVM *L, LVal *v);
LVal *lval(LuftVM *L, int type);
LVal *lnum(LuftVM *L, vlong v);
LVal *lsym(LuftVM *L, char *s);
LVal *llist(LuftVM *L);
LVal *lappend(LuftVM *L, LVal*, LVal*);
LVal *lproc(LuftVM *L, LVal *(*proc)(LuftVM *, LVal *));

/* lenv.c */
void lenventer(LEnv *e, char *s, LVal *v);
LEnv *lenvfind(LEnv *e, char *s);
LVal *lenvlookup(LEnv *e, char *s);
LEnv *lenv(LuftVM *L, LEnv *parent);
void lenvgc(LuftVM *L);

/* lproc.c */
LVal *lprocadd(LuftVM *L, LVal *v);
LVal *lprocsub(LuftVM *L, LVal *v);
LVal *lprocmul(LuftVM *L, LVal *v);
LVal *lprocdiv(LuftVM *L, LVal *v);

LVal *lproctypename(LuftVM *L, LVal *v);
LVal *lprocgraphdump(LuftVM *L, LVal *v);

