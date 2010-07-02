LIB=.
#CFLAGS=-O -pg -I$(LIB) -DFAST
CFLAGS=-g -I$(LIB)
LIBOBJS= $(LIB)/obstack.o $(LIB)/regex.o
OBJS = awk1.o awk2.o awk3.o awk.tab.o debug.o
gawk: $(OBJS) $(LIBOBJS)
	$(CC) -o gawk $(CFLAGS) $(OBJS) $(LIBOBJS) -lm
$(OBJS): awk.h
awk.tab.c: awk.y
	bison -v awk.y
#	yacc -v awk.y
#	mv y.tab.c awk.tab.c
lint:
	lint -I$(LIB) awk.tab.c awk1.c awk2.c awk3.c
clean:
	rm -f gawk *.o core awk.output awk.tab.c

# We don't distribute shar files, but they're useful for mailing.
SHARS = COPYING Makefile README awk.h awk.tab.c awk.y awk1.c awk2.c awk3.c\
 debug.c obstack.h obstack.c regex.h regex.c

awk.shar: $(SHARS)
	shar -f awk -c $(SHARS)

awk.tar: $(SHARS)
	tar cvf awk.tar $(SHARS)

awk.tar.Z:	awk.tar
	-@mv awk.tar.Z awk.tar.old.Z
	compress < awk.tar > awk.tar.Z

# This command probably won't be useful to the rest of the world, but makes
# life much easier for me.
dist:	awk.tar awk.tar.Z
	rcp awk.tar prep:/u2/emacs/awk.tar
	rcp awk.tar.Z prep:/u2/emacs/awk.tar.Z
