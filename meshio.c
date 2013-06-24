/*
 * meshio.c
 *
 * Author: Kazuya Goto <goto@multi.k.u-tokyo.ac.jp>
 * Created on May 16, 2006
 * Last modified: Jun 24, 2013
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "meshio.h"
#include "util.h"

struct MeshIO {
  char *line;
  size_t maxlen;
  FILE *meshfile;
  int header_mode;
};

enum {INIT_MAXLEN = 1024};

void meshio_init(MeshIO **mio_p, FILE *fp)
{
  MeshIO *mio;

  *mio_p = (MeshIO *) emalloc(sizeof(MeshIO));

  mio = *mio_p;

  if (fp == NULL) {
    fprintf(stderr, "Error: meshio_init(NULL)\n");
    exit(2);
  }
  mio->meshfile = fp;

  mio->line = (char *) emalloc(INIT_MAXLEN * sizeof(char));
  mio->maxlen = INIT_MAXLEN;

  mio->header_mode = NONE;
}

void meshio_finalize(MeshIO *mio)
{
  mio->meshfile = NULL;
  free(mio->line);
  mio->header_mode = NONE;

  free(mio);
}

/* read a line from mesh file.
   return value is the pointer to the line.
   either COMMENT, HEADER or DATA is set to *mode, and
   either NODE, ELEMENT, NGROUP, EGROUP or OTHER is set to *header */
char *meshio_readline(MeshIO *mio, int *mode, int *header)
{
  if (!fgets(mio->line, mio->maxlen, mio->meshfile))
    return NULL;

  if (strlen(mio->line) == mio->maxlen - 1 &&
      mio->line[mio->maxlen - 2] != '\n') {
    fprintf(stderr, "too long line (longer than %lu)\n", mio->maxlen);
    exit(2);
  }

  if (mio->line[0] == '#' ||
      (mio->line[0] == '!' && mio->line[1] == '!'))
    *mode = COMMENT;
  else if (mio->line[0] == '!') {
    char *p;

    *mode = HEADER;

    p = mio->line + 1;
    while (isspace(*p))
      p++;
    if (strncmp(p, "NODE", 4) == 0)
      *header = NODE;
    else if (strncmp(p, "ELEMENT", 7) == 0)
      *header = ELEMENT;
    else if (strncmp(p, "NGROUP", 6) == 0)
      *header = NGROUP;
    else if (strncmp(p, "EGROUP", 6) == 0)
      *header = EGROUP;
    else
      *header = OTHER;
    mio->header_mode = *header;
  } else {
    *mode = DATA;
    if (mio->header_mode == NONE) {
      fprintf(stderr,
              "Error: unknown file format (no header before data line)\n");
      exit(1);
    }
    *header = mio->header_mode;
  }

  return mio->line;
}
