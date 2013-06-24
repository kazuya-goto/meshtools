#ifndef NODEDATA_H
#define NODEDATA_H

#include <stdio.h>

#include "precision.h"

struct NodeDB;
typedef struct NodeDB NodeDB;

extern void node_init(NodeDB **ndb);
extern void node_finalize(NodeDB *ndb);
extern void new_node(NodeDB *ndb, int id, coord_t x, coord_t y, coord_t z);
extern void reduce_node_data(NodeDB *ndb);
extern coord_t node_dist2(NodeDB *ndb, int i1, int i2);
extern coord_t penta_vol(NodeDB *ndb, int i0, int i1, int i2, int i3);
extern void node_coord(NodeDB *ndb, int id, coord_t *x, coord_t *y, coord_t *z);
extern int number_of_nodes(const NodeDB *ndb);
extern int get_local_node_id(NodeDB *ndb, int i1);
extern int get_global_node_id(const NodeDB *ndb, int li1);
extern int number_of_middle_nodes(const NodeDB *ndb);
extern int new_middle_node(NodeDB *ndb, int i1, int i2);
extern void print_last_middle_node(const NodeDB *ndb, FILE *fp);
extern void print_node_adv(const NodeDB *ndb, FILE *fp);

#endif /* NODEDATA_H */
