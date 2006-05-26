PREFIX = $(HOME)/hecmw
BINDIR = $(PREFIX)/bin

CC = gcc

WARNFLAGS = -Wall -W -pedantic -ansi
DEBUGFLAGS = -g
#OPTFLAGS = -O3 -fomit-frame-pointer -ffast-math -funroll-loops

CFLAGS = $(WARNFLAGS) $(DEBUGFLAGS) $(OPTFLAGS)
LDFLAGS =

PROGS = rf341to342 sd342to341 meshcount fstr2adv

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PROGS)

rf341to342: util.o meshio.o nodedata.o edgedata.o elemdata.o rf341to342.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

sd342to341: util.o meshio.o nodedata.o sd342to341.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

meshcount: util.o meshio.o meshcount.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

fstr2adv: util.o meshio.o nodedata.o elemdata.o fstr2adv.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) *.o *~ *.log *.tmp *.out *.exe

install: all
	cp $(PROGS) $(BINDIR)
