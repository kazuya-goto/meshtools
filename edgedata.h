#ifndef EDGEDATA_H
#define EDGEDATA_H

#include <stdio.h>

typedef struct Edge {
  int onid; /* node id of the other end of the edge */
  int mnid; /* node id of the middle node */
} Edge;

typedef struct EdgeData {
  int nid;
  int n_edge;  /* number of edges to nodes with greater node-ID *
		* these nodes are recorded in edge[].           */
  int n_edge_s; /* number of edges to nodes with smaller node-ID *
		 * these nodes are not recorded.                 */
  Edge *edge; /* edge detail */
  int max_edge; /* allocated length of edge[] */

} EdgeData;

typedef struct EdgeDB {
  int n_node_init;
  EdgeData *edge_data;
  NodeDB *ndb;
} EdgeDB;

extern void edge_init(EdgeDB *edb, NodeDB *ndb);
extern void edge_finalize(EdgeDB *edb);
extern int middle_node(EdgeDB *edb, int i1, int i2, int *mnidp);
extern void print_edge_stat(const EdgeDB *edb, FILE *log_file);

#endif /* EDGEDATA_H */
