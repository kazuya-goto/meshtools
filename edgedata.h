#ifndef EDGEDATA_H
#define EDGEDATA_H

#include <stdio.h>

extern void edge_init(void);
extern void edge_finalize(void);
extern int middle_node(int i1, int i2, int *mnidp);
extern void print_edge_stat(FILE *log_file);

#endif /* EDGEDATA_H */
