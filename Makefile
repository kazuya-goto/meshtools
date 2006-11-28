PREFIX = $(HOME)/hecmw
BINDIR = $(PREFIX)/bin

CC = gcc

WARNFLAGS = -Wall -W -pedantic -ansi
#DEBUGFLAGS = -g
DEBUGFLAGS = -DNDEBUG
OPTFLAGS = -O2

CFLAGS = $(WARNFLAGS) $(DEBUGFLAGS) $(OPTFLAGS) -D_FILE_OFFSET_BITS=64
LDFLAGS =

PROGS = rf341to342 sd342to341 meshcount fstr2adv

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PROGS)

rf341to342: util.o meshio.o nodedata.o edgedata.o rf341to342.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

sd342to341: util.o meshio.o nodedata.o sd342to341.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

meshcount: util.o meshio.o meshcount.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

fstr2adv: util.o meshio.o nodedata.o elemdata.o fstr2adv.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) *.o *~ *.log *.tmp *.out *.exe *.tar.gz

install: all
	cp $(PROGS) $(BINDIR)

dist:
	@ if [ -d meshtools ]; then rm -rf meshtools; fi
	@ mkdir meshtools
	@ cp Makefile *.c *.h README meshtools
	@ DISTNAME=`svn info | grep Revision | perl -ne 'split;print "meshtools-r$$_[1].tar.gz";'`; \
	if [ -f $$DISTNAME ]; then rm -f $$DISTNAME; fi; \
	echo Creating $$DISTNAME; \
	tar zcvf $$DISTNAME meshtools
	@ rm -rf meshtools
