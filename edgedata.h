#ifndef EDGEDATA_H
#define EDGEDATA_H

#include <stdio.h>

struct EdgeDB;
typedef struct EdgeDB EdgeDB;

extern void edge_init(EdgeDB **edb_p, NodeDB *ndb);
extern void edge_finalize(EdgeDB *edb);
extern int middle_node(EdgeDB *edb, int i1, int i2, int *mnidp);
extern void print_edge_stat(const EdgeDB *edb, FILE *log_file);

#endif /* EDGEDATA_H */
