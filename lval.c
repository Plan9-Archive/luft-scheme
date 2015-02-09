#include <u.h>
#include <libc.h>
#include <luft.h>
#include "impl.h"

void
ldel(LuftVM*, LVal *v)
{
	switch(v->type){
	case TNUMBER:
	case TPROC:
		break;
	case TLAMBDA:
	case TLIST:
		free(v->list);
		v->len = 0;
		v->list = nil;
		break;
	case TSYMBOL:
		free(v->s);
		v->s = nil;
		break;
	}

	free(v);
}

LVal*
lval(LuftVM *L, int type)
{
	LVal *v;

	v = mallocz(sizeof(*v), 1);
	v->gctype = LGCVAL;
	v->gclink = L->gcl;
	L->gcl = v;
	v->type = type;
	return v;
}

LVal *lnum(LuftVM *L, vlong num)
{
	LVal *v;

	v = lval(L, TNUMBER);
	v->i = num;
	return v;
}

LVal *lsym(LuftVM *L, char *s)
{
	LVal *v;

	v = lval(L, TSYMBOL);
	v->s = strdup(s);
	return v;
}

LVal*
llist(LuftVM *L)
{
	LVal *v;

	v = lval(L, TLIST);
	v->len = 0;
	v->list = nil;
	return v;
}

LVal*
lappend(LuftVM*, LVal *v, LVal *x)
{
	v->len++;
	v->list = realloc(v->list, sizeof(LVal*) * v->len);
	v->list[v->len-1] = x;
	return v;
}

LVal*
llistcopy(LuftVM *L, LVal *v)
{
	int i;
	LVal *rv;

	if(v->type != TLIST)
		lufterr(L, "llistcopy: not a list: %V", v);

	rv = llist(L);
	for(i = 0; i < v->len; i++){
		lappend(L, rv, v->list[i]);
	}

	return rv;
}

LVal*
lproc(LuftVM *L, LVal *(*proc)(LuftVM *, LVal *))
{
	LVal *v;

	v = lval(L, TPROC);
	v->proc = proc;
	return v;
}

