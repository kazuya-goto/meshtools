PREFIX = $(HOME)/hecmw
BINDIR = $(PREFIX)/bin

CC = gcc
#CFLAGS = -g -DDEBUG -Wall
#CFLAGS = -O -DDEBUG -Wall
CFLAGS = -O -Wall
LDFLAGS = -lm

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: rf341to342 sd342to341 meshcount

rf341to342: nodedata.o edgedata.o rf341to342.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o rf341to342 nodedata.o edgedata.o rf341to342.o

sd342to341: nodedata.o sd342to341.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o sd342to341 nodedata.o sd342to341.o

meshcount: meshcount.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o meshcount meshcount.o

clean:
	rm -f rf341to342 sd342to341 *.o *~ *.log *.tmp *.out *.exe

install: all
	cp rf341to342 sd342to341 meshcount $(BINDIR)
