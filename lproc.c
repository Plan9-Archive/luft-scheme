#include <u.h>
#include <libc.h>
#include <luft.h>
#include "impl.h"

/* builtin procs */

LVal*
lprocadd(LuftVM *L, LVal *v)
{
	int i;
	vlong n;
	LVal *cv;

	if(v->len <= 0)
		return lenvlookup(v->env, "nil");

	cv = v->list[0];

	if(cv->type != TNUMBER)
		return lenvlookup(v->env, "#f");

	n = cv->i;

	for(i = 1; i < v->len; i++){
		cv = v->list[i];
		if(cv->type != TNUMBER)
			return lenvlookup(v->env, "#f");
		n += cv->i;
	}

	return lnum(L, n);
}

LVal*
lprocsub(LuftVM *L, LVal *v)
{
	int i;
	vlong n;
	LVal *cv;

	if(v->len <= 0)
		return lenvlookup(v->env, "nil");

	cv = v->list[0];

	if(cv->type != TNUMBER)
		return lenvlookup(v->env, "#f");

	n = cv->i;

	/* unary minus */
	if(v->len == 1)
		return lnum(L, -n);

	for(i = 1; i < v->len; i++){
		cv = v->list[i];
		if(cv->type != TNUMBER)
			return lenvlookup(v->env, "#f");
		n -= cv->i;
	}

	return lnum(L, n);
}

LVal*
lprocmul(LuftVM *L, LVal *v)
{
	int i;
	vlong n;
	LVal *cv;

	if(v->len <= 0)
		return lenvlookup(v->env, "nil");

	cv = v->list[0];

	if(cv->type != TNUMBER)
		return lenvlookup(v->env, "#f");

	n = cv->i;

	for(i = 1; i < v->len; i++){
		cv = v->list[i];
		if(cv->type != TNUMBER)
			return lenvlookup(v->env, "#f");
		n *= cv->i;
	}

	return lnum(L, n);
}

LVal*
lprocdiv(LuftVM *L, LVal *v)
{
	int i;
	vlong n;
	LVal *cv;

	if(v->len <= 0)
		return lenvlookup(v->env, "nil");

	cv = v->list[0];

	if(cv->type != TNUMBER)
		return lenvlookup(v->env, "#f");

	n = cv->i;

	for(i = 1; i < v->len; i++){
		cv = v->list[i];
		if(cv->type != TNUMBER)
			return lenvlookup(v->env, "#f");
		n /= cv->i;
	}

	return lnum(L, n);
}

LVal*
lproccmp(LuftVM *L, LVal *v)
{
	LVal *rv;

	rv = lenvlookup(L->env, "#f");
	if(v->len < 2)
		return rv;

	if(v->list[0]->type != TNUMBER || v->list[1]->type != TNUMBER)
		return rv;

	if(v->list[0]->i != v->list[1]->i)
		return rv;

	return lenvlookup(L->env, "#t");
}

LVal*
lproclist(LuftVM *L, LVal *v)
{
	return llistcopy(L, v);
}

LVal*
lprocprint(LuftVM*, LVal *v)
{
	print("%E\n", v);
	return v;
}

LVal*
lproctypename(LuftVM *L, LVal *v)
{
	LVal *rv;

	if(v->len < 1)
		return lenvlookup(L->env, "nil");

	rv = lsym(L, ltypename(v->list[0]->type));
	return rv;
}

static void
dump1(LVal *v)
{
	int i;

	print("\t%s_%#p [label=\"%V\"];\n", ltypename(v->type), v, v);

	if(v->type == TLIST || v->type == TLAMBDA){
		for(i = 0; i < v->len; i++){
			print("\t%s_%#p -> %s_%#p;\n", ltypename(v->type), v, ltypename(v->list[i]->type), v->list[i]);
			dump1(v->list[i]);
		}
	}
}

LVal*
lprocgraphdump(LuftVM *L, LVal *v)
{
	print("digraph %s {\n", ltypename(v->type));
	dump1(v);
	print("}\n");
	return lenvlookup(L->env, "nil");
}

