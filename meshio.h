#ifndef MESHIO_H
#define MESHIO_H

#include <stdio.h>

enum {COMMENT, HEADER, DATA};
enum {NONE, NODE, ELEMENT, NGROUP, EGROUP, OTHER};

typedef struct MeshIO {
  char *line;
  unsigned maxlen;
  FILE *meshfile;
  int header_mode;
} MeshIO;

extern void meshio_init(MeshIO *mio, FILE *fp);
extern void meshio_finalize(MeshIO *mio);
extern char *meshio_readline(MeshIO *mio, int *mode, int *header);

#endif /* MESHIO_H */
