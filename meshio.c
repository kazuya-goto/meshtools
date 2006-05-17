#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "meshio.h"

enum {INIT_MAXLEN = 1024};

static char *line;
static unsigned int maxlen;
static FILE *meshfile;
static int header_mode;

void meshio_init(FILE *fp)
{
  if (fp == NULL) {
    fprintf(stderr, "Error: meshio_init(NULL)\n");
    exit(2);
  }
  meshfile = fp;

  line = (char *) malloc(INIT_MAXLEN * sizeof(char));
  if (line == NULL) {
    perror("in meshio_init()");
    exit(2);
  }
  maxlen = INIT_MAXLEN;

  header_mode = NONE;
}

void meshio_finalize(void)
{
  meshfile = NULL;
  free(line);
  header_mode = NONE;
}

char *meshio_readline(int *mode, int *header)
{
  if (!fgets(line, maxlen, meshfile))
    return NULL;

  if (strlen(line) == maxlen-1 &&
      line[maxlen-2] != '\n') {
    fprintf(stderr, "too long line (longer than %u)\n", maxlen);
    exit(2);
  }

  if (line[0] == '#' ||
      (line[0] == '!' && line[1] == '!'))
    *mode = COMMENT;
  else if (line[0] == '!') {
    char *p;

    *mode = HEADER;

    p = line + 1;
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
    header_mode = *header;
  } else {
    *mode = DATA;
    if (header_mode == NONE) {
      fprintf(stderr,
              "Error: unknown file format (no header before data line)\n");
      exit(1);
    }
    *header = header_mode;
  }

  return line;
}
