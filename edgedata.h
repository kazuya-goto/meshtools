#ifndef EDGEDATA_H
#define EDGEDATA_H

#include <stdio.h>
#include "precision.h"

struct EdgeDB;
typedef struct EdgeDB EdgeDB;

extern void edge_init(EdgeDB **edb_p, NodeDB *ndb);
extern void edge_finalize(EdgeDB *edb);
extern index_t middle_node(EdgeDB *edb, index_t i1, index_t i2, index_t *mnidp);
extern void print_edge_stat(const EdgeDB *edb, FILE *log_file);

#endif /* EDGEDATA_H */
