#include <u.h>
#include <libc.h>
#include <bio.h>
#include <luft.h>

Biobuf bin;
LuftVM *L;

int
del(void*, char *msg)
{
	if(strcmp(msg, "interrupt") == 0)
		return 1;
	return 0;
}

char prog[65535];

void
main(int argc, char *argv[])
{
	int iflag, n, len, rv;
	char *line, *new, *old;

	iflag = 0;

	ARGBEGIN{
	case 'i':
		iflag = 1;
		break;
	}ARGEND

	atnotify(del, 1);


	L = luftvm();

	if(!iflag){
		len = 0;
		for(;;){
			if(len >= sizeof(prog)-1)
				break;
			n = read(0, prog+len, sizeof(prog)-1-len);
			if(n < 0)
				sysfatal("read: %r");
			if(n == 0)
				break;
			len += n;
		}

		prog[len] = '\0';

		if(luftdo(L, prog, len) < 0)
			sysfatal("luftdo: %s", lufterrstr(L));

		exits(nil);
	}

	Binit(&bin, 0, OREAD);
	for(;;){
		print("> ");
		line = Brdstr(&bin, '\n', 1);
		if(line == nil)
			break;

		if(strlen(line) == 0)
			continue;

		for(;;){
			rv = luftdo(L, line, strlen(line));
			if(rv == 0)
				break;
			else if(rv == -1){
				if(iflag)
					print(">> ");

				old = line;
				new = Brdstr(&bin, '\n', 1);
				if(new == nil)
					break;

				line = smprint("%s\n%s", old, new);
				free(old);
				free(new);
			} else if(rv == -2){
				fprint(2, "luftdo: %s\n", lufterrstr(L));
				break;
			}
		}

		free(line);
	}

	exits(nil);
}

