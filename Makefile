-include Makefile.inc

PROGS = rf341to342 sd342to341 meshcount fstr2adv

SRCS = \
	edgedata.c \
	elemdata.c \
	fstr2adv.c \
	main.c \
	meshcount.c \
	meshio.c \
	nodedata.c \
	rf341to342.c \
	sd342to341.c \
	util.c

HEADERS = \
	edgedata.h \
	elemdata.h \
	meshio.h \
	nodedata.h \
	precision.h \
	refine.h \
	util.h

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

check:
	@(cd sample-mesh; ./test.sh)

clean:
	rm -f $(PROGS) *.o *~ *.log *.tmp *.out *.exe *.tar.gz

install: all
	cp $(PROGS) $(BINDIR)

depend: $(SRCS)
	$(CC) -MM -MG $(SRCS) $(CFLAGS) > Makefile.dep

-include Makefile.dep
