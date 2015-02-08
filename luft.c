#include <u.h>
#include <libc.h>
#include <stdio.h>
#include <luft.h>
#include "impl.h"

#ifdef DEBUG
#define YY_DEBUG 1
#endif

static int
getch(LuftVM *L)
{
	int c;

	/* handle eof */
	if(L->inrp >= L->inlen)
		return -1;
	
	c = (int) (L->input[L->inrp++] & 0xFF);
	switch(c){
	case '\n':
		L->inln++;
		break;
	}
	return c;
}

static int
getr(LuftVM *L)
{
	int c, i;
	char str[UTFmax+1];
	Rune r;

	c = getch(L);
	if(c < Runeself)
		return c;

	i = 0;
	str[i++] = c;

	do {
		c = getch(L);
		str[i++] = c;
	} while(!fullrune(str, i));

	c = chartorune(&r, str);
	if(r == Runeerror && c == 1){
		print("illegal utf-8 sequence\t");
		for(c = 0; c < i; c++)
			print("%s%.2x", c > 0 ? " " : "", *(uchar*)(str+c));
		print("\n");
	}

	return r;
}

static int
lexc(LuftVM *L, char *buf, int sz)
{
	int c, res;
	Rune r;

	if(sz < UTFmax)
		sysfatal("out of room");

	c = getr(L);

	if(c < 0){
		buf[0] = '\0';
		return 0;
	} else if(c >= Runeself){
		r = c;
		res = runetochar(buf, &r);
		buf[res] = '\0';
		return res;
	} else {
		buf[0] = c;
		buf[1] = '\0';
		return 1;
	}
}

#define YYSTYPE LVal*

#define YY_CTX_LOCAL
#define YY_CTX_MEMBERS LuftVM *L;

#define YY_INPUT(ctx, buf, result, max)				\
{								\
	result = lexc(ctx->L, buf, max);			\
}

#include "luft.leg.c"

static char *typenames[] = {
[TSYMBOL]	"symbol",
[TNUMBER]	"number",
[TLIST]		"list",
[TPROC]		"proc",
[TLAMBDA]	"lambda",
};

char*
ltypename(int ty)
{
	if(ty >= TSYMBOL && ty <= TLAMBDA)
		return typenames[ty];
	return "<missing type name>";
}

/* format LVal */
int
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
	}

	return fmtprint(f, "%s (%s)", ltypename(v->type), extra);
}

LVal*
luftparse(LuftVM *L, char *code, int len)
{
	yycontext ctx;
	memset(&ctx, 0, sizeof(ctx));

	if(L->input != nil)
		free(L->input);

	L->input = strdup(code);
	L->inlen = len;
	L->inrp = 0;
	L->inln = 1;

	if(L->inval != nil){
		L->inval = nil;
	}

	if(setjmp(L->errjmp)){
		/* a parse error occurred, so clear the
		 * jmp_buf pc to prevent an accident */
		L->errjmp[JMPBUFPC] = 0;
	} else {
		ctx.L = L;
		yyparse(&ctx);
	}

	yyrelease(&ctx);

	return L->inval;
}

LVal*
lufteval(LuftVM *L, LVal *val, LEnv *env)
{
	int i;
	LVal *rv, *sym, *list, *proc;
	LEnv *e;

	rv = nil;

	switch(val->type){
	case TNUMBER:
		rv = val;
		break;
	case TSYMBOL:
		e = lenvfind(env, val->s);
		if(e == nil)
			lufterr(L, "unbound symbol '%s'", val->s);

		rv = lenvlookup(e, val->s);
		break;
	case TLIST:
		if(val->len == 0){
			rv = lenvlookup(env, "nil");
			break;
		}

		sym = val->list[0];

		if(sym->type == TSYMBOL && sym->s != nil){
			/* builtins */
			if(strcmp(sym->s, "quote") == 0){
				if(val->len < 2)
					rv = lenvlookup(env, "nil");
				else
					rv = val->list[1];
			} else if(strcmp(sym->s, "if") == 0){
				/* condition */
				rv = lufteval(L, val->list[1], env);
				if(rv->type == TSYMBOL && strcmp(rv->s, "#f") == 0){
					/* was false */
					if(val->len < 4)
						rv = lenvlookup(e, "nil");
					else
						rv = val->list[3];
				} else {
					/* was true */
					rv = val->list[2];
				}

				rv = lufteval(L, rv, env);
			} else if(strcmp(sym->s, "define") == 0){
				assert(val->len == 3);
				rv = val->list[1];
				if(rv->type != TSYMBOL)
					sysfatal("define: not a symbol");
				rv = lufteval(L, val->list[2], env);
				lenventer(env, val->list[1]->s, rv);
			} else if(strcmp(sym->s, "lambda") == 0){
				assert(val->len == 3);
				val->type = TLAMBDA;
				val->env = env;
				rv = val;
			}

			if(rv != nil)
				goto rv;
		}

		/* proc/lambda */
		proc = lufteval(L, val->list[0], env);
		list = llist(L);

		for(i = 1; i < val->len; i++){
			lappend(L, list, lufteval(L, val->list[i], env));
		}

		switch(proc->type){
		case TPROC:
			rv = proc->proc(L, list);
			break;
		case TLAMBDA:
			rv = lufteval(L, proc->list[2], proc->env);
			break;
		default:
			rv = lenvlookup(env, "nil");
		}

		break;
	}

rv:
	return rv;
}

