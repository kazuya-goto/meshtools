PREFIX = $(HOME)/hecmw
BINDIR = $(PREFIX)/bin

CC = gcc

#WARNFLAGS = -Wall -W -pedantic -ansi
#WARNFLAGS = -Wall -W -pedantic -std=c99
WARNFLAGS =
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

rf341to342: util.o meshio.o nodedata.o edgedata.o rf341to342.o main.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

sd342to341: util.o meshio.o nodedata.o sd342to341.o main.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

meshcount: util.o meshio.o meshcount.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

fstr2adv: util.o meshio.o nodedata.o elemdata.o fstr2adv.o main.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) *.o *~ *.log *.tmp *.out *.exe *.tar.gz

install: all
	cp $(PROGS) $(BINDIR)

dist:
	@ DISTNAME=`svn info | grep Revision | perl -ne 'split;print "meshtools-r$$_[1]";'`; \
	if [ -d $$DISTNAME ]; then rm -rf $$DISTNAME; fi; \
	svn export . $$DISTNAME; \
	if [ -f $$DISTNAME.tar.gz ]; then rm -f $$DISTNAME.tar.gz; fi; \
	echo Creating $$DISTNAME.tar.gz; \
	tar zcvf $$DISTNAME.tar.gz $$DISTNAME; \
	rm -rf $$DISTNAME
