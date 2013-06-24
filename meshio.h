#ifndef MESHIO_H
#define MESHIO_H

#include <stdio.h>

enum {COMMENT, HEADER, DATA};
enum {NONE, NODE, ELEMENT, NGROUP, EGROUP, OTHER};

struct MeshIO;
typedef struct MeshIO MeshIO;

extern void meshio_init(MeshIO **mio_p, FILE *fp);
extern void meshio_finalize(MeshIO *mio);
extern char *meshio_readline(MeshIO *mio, int *mode, int *header);

#endif /* MESHIO_H */
