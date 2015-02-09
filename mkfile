</$objtype/mkfile

LIB=/$objtype/lib/libluft.a

OFILES=\
	luft.$O\
	lapi.$O\
	lenv.$O\
	lproc.$O\
	lval.$O\

HFILES=\
	/sys/include/luft.h\

CLEANFILES=\
	$HFILES\
	$O.test\

</sys/src/cmd/mklib

/sys/include/luft.h:	luft.h
	cp $prereq $target

luft.$O:	luft.leg.c
	$CC -FTVp luft.c

luft.leg.c:	luft.leg
	leg -o $target < $prereq

$O.test:	test.$O $LIB
	$LD -o $target $prereq

uninstall:V:
	rm -f $HFILES $LIB

