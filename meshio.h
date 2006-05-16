#ifndef _MESHIO_H_
#define _MESHIO_H_

enum {COMMENT, HEADER, DATA};
enum {NONE, NODE, ELEMENT, NGROUP, EGROUP, OTHER};

extern void meshio_init(FILE *fp);
extern void meshio_finalize(void);
extern char *meshio_readline(int *mode, int *header);

#endif /* _MESHIO_H_ */
