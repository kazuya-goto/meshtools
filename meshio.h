#ifndef MESHIO_H
#define MESHIO_H

#include <stdio.h>

enum {COMMENT, HEADER, DATA};
enum {NONE, NODE, ELEMENT, NGROUP, EGROUP, OTHER};

extern void meshio_init(FILE *fp);
extern void meshio_finalize(void);
extern char *meshio_readline(int *mode, int *header);

#endif /* MESHIO_H */
