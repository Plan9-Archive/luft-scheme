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
	lenventer(L->env, "+", lproc(L, lprocadd));
	lenventer(L->env, "-", lproc(L, lprocsub));
	lenventer(L->env, "*", lproc(L, lprocmul));
	lenventer(L->env, "/", lproc(L, lprocdiv));

	lenventer(L->env, "typename", lproc(L, lproctypename));
	lenventer(L->env, "graphdump", lproc(L, lprocgraphdump));
}

LuftVM*
luftvm(void)
{
	static int luftinit = 0;
	LuftVM *vm;

	if(!luftinit){
		luftinit = 1;
		fmtinstall('V', Vfmt);
	}

	vm = mallocz(sizeof(*vm), 1);

	luftvminit(vm);

	return vm;
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
	print(" = %V\n", rv);

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

