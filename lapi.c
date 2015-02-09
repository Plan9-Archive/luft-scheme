#include <u.h>
#include <libc.h>
#include <luft.h>
#include "impl.h"

static void
luftvminit(LuftVM *L)
{
	/* global symbols */
	L->env = lenv(L, nil);

	lenventer(L->env, "nil", lsym(L, "nil"));
	lenventer(L->env, "#t", lsym(L, "#t"));
	lenventer(L->env, "#f", lsym(L, "#f"));

	/* math */
	luftproc(L, "+", lprocadd);
	luftproc(L, "-", lprocsub);
	luftproc(L, "*", lprocmul);
	luftproc(L, "/", lprocdiv);
	luftproc(L, "=", lproccmp);

	/* list manipulation */
	luftproc(L, "list", lproclist);

	luftproc(L, "print", lprocprint);
	luftproc(L, "typename", lproctypename);
	luftproc(L, "graphdump", lprocgraphdump);
}

static int
Vfmt(Fmt *f)
{
	char extra[32];
	LVal *v;

	extra[0] = '\0';
	v = va_arg(f->args, LVal*);

	if(v == nil)
		return fmtprint(f, "<nil>");

	switch(v->type){
	case TSYMBOL:
		snprint(extra, sizeof(extra), "%s", v->s);
		break;
	case TNUMBER:
		snprint(extra, sizeof(extra), "%lld", v->i);
		break;
	case TLIST:
		snprint(extra, sizeof(extra), "%d", v->len);
		break;
	case TPROC:
		snprint(extra, sizeof(extra), "%#p", v->proc);
		break;
	case TLAMBDA:
		snprint(extra, sizeof(extra), "env %#p", v->env);
		break;
	}

	return fmtprint(f, "%s (%s)", ltypename(v->type), extra);
}

static int
Efmt(Fmt *f)
{
	int n, i;
	LVal *v;

	n = 0;
	v = va_arg(f->args, LVal*);

	switch(v->type){
	case TSYMBOL:
		n += fmtprint(f, "%s", v->s);
		break;
	case TNUMBER:
		n += fmtprint(f, "%lld", v->i);
		break;
	case TLIST:
		n += fmtstrcpy(f, "(");
		for(i = 0; i < v->len; i++){
			if(i > 0)
				n += fmtstrcpy(f, " ");
			n += fmtprint(f, "%E", v->list[i]);
		}
		n += fmtstrcpy(f, ")");
		break;
	case TPROC:
		n += fmtstrcpy(f, "<Proc>");
		break;
	case TLAMBDA:
		n += fmtstrcpy(f, "<Lambda>");
		break;
	}

	return n;
}

LuftVM*
luftvm(void)
{
	static int luftinit = 0;
	LuftVM *vm;

	if(!luftinit){
		luftinit = 1;
		fmtinstall('V', Vfmt);
		fmtinstall('E', Efmt);
	}

	vm = mallocz(sizeof(*vm), 1);

	luftvminit(vm);

	return vm;
}

void
luftproc(LuftVM *L, char *sym, LVal *(*proc)(LuftVM*, LVal*))
{
	lenventer(L->env, sym, lproc(L, proc));
}

int
luftdo(LuftVM *L, char *code, int len)
{
	LVal *v, *rv;

	v = luftparse(L, code, len);
	if(v == nil)
		return -1;

	if(setjmp(L->errjmp) > 0){
		L->errjmp[JMPBUFPC] = 0;
		return -2;
	}

	rv = lufteval(L, v, L->env);
	print("%E\n", rv);

	lenvgc(L);
	return 0;
}

void
lufterr(LuftVM *L, char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vseprint(L->errstr, L->errstr+sizeof(L->errstr), fmt, arg);
	va_end(arg);

	assert(L->errjmp[JMPBUFPC] != 0);
	longjmp(L->errjmp, 1);
}

char*
lufterrstr(LuftVM *L)
{
	return L->errstr;
}

