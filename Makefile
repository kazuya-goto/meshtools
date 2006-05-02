PREFIX = $(HOME)/hecmw
BINDIR = $(PREFIX)/bin

CC = gcc
CFLAGS = -g -DDEBUG -Wall
#CFLAGS = -O -DDEBUG -Wall
#CFLAGS = -O -Wall
LDFLAGS = -lm

PROGS = rf341to342 sd342to341 meshcount fstr2adv

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PROGS)

rf341to342: nodedata.o edgedata.o rf341to342.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o rf341to342 nodedata.o edgedata.o rf341to342.o

sd342to341: nodedata.o sd342to341.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o sd342to341 nodedata.o sd342to341.o

meshcount: meshcount.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o meshcount meshcount.o

fstr2adv: nodedata.o elemdata.o fstr2adv.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o fstr2adv nodedata.o elemdata.o fstr2adv.o

clean:
	rm -f $(PROGS) *.o *~ *.log *.tmp *.out *.exe

install: all
	cp $(PROGS) $(BINDIR)
