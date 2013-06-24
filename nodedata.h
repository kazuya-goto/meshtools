#ifndef NODEDATA_H
#define NODEDATA_H

#include <stdio.h>

#include "precision.h"

struct NodeDB;
typedef struct NodeDB NodeDB;

extern void node_init(NodeDB **ndb_p);
extern void node_finalize(NodeDB *ndb);
extern void new_node(NodeDB *ndb, index_t id, coord_t x, coord_t y, coord_t z);
extern void reduce_node_data(NodeDB *ndb);
extern coord_t node_dist2(NodeDB *ndb, index_t i1, index_t i2);
extern coord_t penta_vol(NodeDB *ndb, index_t i0, index_t i1, index_t i2, index_t i3);
extern void node_coord(NodeDB *ndb, index_t id, coord_t *x, coord_t *y, coord_t *z);
extern index_t number_of_nodes(const NodeDB *ndb);
extern index_t get_local_node_id(NodeDB *ndb, index_t i1);
extern index_t get_global_node_id(const NodeDB *ndb, index_t li1);
extern index_t number_of_middle_nodes(const NodeDB *ndb);
extern index_t new_middle_node(NodeDB *ndb, index_t i1, index_t i2);
extern void print_last_middle_node(const NodeDB *ndb, FILE *fp);
extern void print_node_adv(const NodeDB *ndb, FILE *fp);

#endif /* NODEDATA_H */
