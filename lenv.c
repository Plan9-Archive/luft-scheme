#include <u.h>
#include <libc.h>
#include <luft.h>
#include "impl.h"

void
lenventer(LEnv *e, char *s, LVal *v)
{
	u32int h;
	char *p;
	LEnvHash *hsh;

	h = 0;

	for(p = s; *p; p++)
		h = h*3 + *p;

	h %= LUFTHASHSZ;

	hsh = mallocz(sizeof(*hsh), 1);
	hsh->next = e->hash[h];
	e->hash[h] = hsh;
	snprint(hsh->name, sizeof(hsh->name), "%s", s);
	hsh->val = v;
}

/* find which env a symbol is in */
LEnv*
lenvfind(LEnv *e, char *s)
{
	u32int h;
	char *p;
	LEnv *newe;
	LEnvHash *hsh;

	h = 0;

	for(p = s; *p; p++)
		h = h*3 + *p;

	h %= LUFTHASHSZ;

	for(hsh = e->hash[h]; hsh != nil; hsh = hsh->next){
		/* check symbol exists in this environment */
		if(strcmp(s, hsh->name) == 0)
			return e;
	}

	if(e->parent != nil)
		return lenvfind(e->parent, s);

	return nil;
}

/* return the value of a symbol in an env */
LVal*
lenvlookup(LEnv *e, char *s)
{
	u32int h;
	char *p;
	LEnvHash *hsh;
	LVal *v;

	h = 0;
	v = nil;

	for(p = s; *p; p++)
		h = h*3 + *p;

	h %= LUFTHASHSZ;

	for(hsh = e->hash[h]; hsh != nil; hsh = hsh->next){
		if(strcmp(s, hsh->name) == 0){
			v = hsh->val;
			break;
		}
	}

	return v;
}

LEnv*
lenv(LuftVM *L, LEnv *parent)
{
	LEnv *env;

	env = mallocz(sizeof(*env), 1);
	env->gctype = LGCENV;
	env->gclink = L->gcl;
	L->gcl = env;
	env->L = L;
	env->parent = parent;

	return env;
}

static void
lenvdel(LuftVM *L, LEnv *env)
{
	int i;
	LEnvHash *hsh, *next;

	for(i = 0; i < LUFTHASHSZ; i++){
		for(hsh = env->hash[i]; hsh != nil; hsh = next){
			next = hsh->next;
			free(hsh);
		}
	}
}

static void gcmarkval(LVal *v);

static void
gcmarkenv(LEnv *env)
{
	int i;
	LGc *g;
	LEnvHash *hsh;
	LVal *v;
	LEnv *e;

	if(env->gcmark > 0)
		return;

	env->gcmark = 1;

	for(i = 0; i < LUFTHASHSZ; i++){
		for(hsh = env->hash[i]; hsh != nil; hsh = hsh->next){
			v = hsh->val;
			gcmarkval(v);
		}
	}
}

static void
gcmarkval(LVal *v)
{
	int i;

	if(v->gcmark > 0)
		return;

	v->gcmark = 1;

	switch(v->type){
	case TLIST:
	case TLAMBDA:
		for(i = 0; i < v->len; i++){
			gcmarkval(v->list[i]);
		}
	}

	if(v->env != nil)
		gcmarkenv(v->env);

	return;
}

void
lenvgc(LuftVM *L)
{
	int i, l;
	LGc *m, **p, *next;
	LEnvHash *hsh;
	LVal *v;

	for(m = L->gcl; m != nil; m = m->gclink)
		m->gcmark = 0;

	gcmarkenv(L->env);

	p = &L->gcl;
	for(m = L->gcl; m != nil; m = next){
		next = m->gclink;
		if(m->gcmark == 0){
			*p = next;
			switch(m->gctype){
			case LGCVAL:
				ldel(L, (LVal*)m);
				break;
			case LGCENV:
				lenvdel(L, (LEnv*)m);
				break;
			}
		} else
			p = &m->gclink;
	}
}

