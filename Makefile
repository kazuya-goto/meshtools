PREFIX = $(HOME)/hecmw
BINDIR = $(PREFIX)/bin

CC = gcc

WARNFLAGS = -Wall -W -pedantic
DEBUGFLAGS = -g
#OPTFLAGS = -O3 -fomit-frame-pointer -ffast-math -funroll-loops

CFLAGS = $(WARNFLAGS) $(DEBUGFLAGS) $(OPTFLAGS)
LDFLAGS = -lm

PROGS = rf341to342 sd342to341 meshcount fstr2adv

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PROGS)

rf341to342: nodedata.o edgedata.o rf341to342.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

sd342to341: nodedata.o sd342to341.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

meshcount: meshcount.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

fstr2adv: nodedata.o elemdata.o fstr2adv.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) *.o *~ *.log *.tmp *.out *.exe

install: all
	cp $(PROGS) $(BINDIR)
