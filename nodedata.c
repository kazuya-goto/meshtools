/*
 * nodedata.c
 *
 * Author: Kazuya Goto <goto@multi.k.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last Modified: Jun 24, 2013
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "nodedata.h"
#include "util.h"

struct NodeData {
  index_t id;
  coord_t x;
  coord_t y;
  coord_t z;
};

typedef struct NodeData NodeData;

struct NodeDB {
  index_t n_node;
  NodeData *node_data;
  index_t max_node;
  int issorted;
  index_t n_mnode;
  NodeData middle_node; /* last added middle node */
};

enum { MAX_NODE_INIT = 1024, MAX_NODE_GROW = 2 };

/* initialize node_data */
void node_init(NodeDB **ndb_p)
{
  NodeDB *ndb;

  *ndb_p = (NodeDB *) emalloc(sizeof(NodeDB));

  ndb = *ndb_p;

  ndb->n_node = 0;
  ndb->node_data = (NodeData *) emalloc(MAX_NODE_INIT * sizeof(NodeData));
  ndb->max_node = MAX_NODE_INIT;
  ndb->issorted = 1;
  ndb->n_mnode = 0;
}

/* finalize node_data */
void node_finalize(NodeDB *ndb)
{
  ndb->n_node = 0;
  free(ndb->node_data);
  ndb->node_data = NULL;
  ndb->max_node = 0;
  ndb->issorted = 0;
  ndb->n_mnode = 0;

  free(ndb);
}

/* resize node_data */
static void resize_node_data(NodeDB *ndb, size_t len)
{
  ndb->node_data = (NodeData *) erealloc(ndb->node_data, len * sizeof(NodeData));
  ndb->max_node = len;
}

/* register a new node in node_data */
void new_node(NodeDB *ndb, index_t id, coord_t x, coord_t y, coord_t z)
{
  if (ndb->n_node == ndb->max_node)
    resize_node_data(ndb, ndb->max_node * MAX_NODE_GROW);

  if (ndb->n_node > 0 && ndb->node_data[ndb->n_node-1].id >= id) {
    fprintf(stderr, "Warning: node id is not sorted\n");
    ndb->issorted = 0;
  }

  ndb->node_data[ndb->n_node].id = id;
  ndb->node_data[ndb->n_node].x = x;
  ndb->node_data[ndb->n_node].y = y;
  ndb->node_data[ndb->n_node].z = z;

  ndb->n_node++;
}

/* reduce the size of node_data */
void reduce_node_data(NodeDB *ndb)
{
  resize_node_data(ndb, ndb->n_node);
}

/* node comparison, to be used by bsearch and qsort */
static int node_compar(const void *vn1, const void *vn2)
{
  const NodeData *n1, *n2;

  n1 = (NodeData *) vn1;
  n2 = (NodeData *) vn2;

  if (n1->id < n2->id) return -1;
  else if (n1->id == n2->id) return 0;
  else return 1;
}

/* find a node having globalID i1 */
static NodeData *search_node(NodeDB *ndb, index_t i1)
{
  NodeData node1, *n1p;

  if (!ndb->issorted) {
    fprintf(stderr, "start sorting node data... ");
    qsort(ndb->node_data, ndb->n_node, sizeof(NodeData), node_compar);
    fprintf(stderr, "done.\n");
    ndb->issorted = 1;
  }

  node1.id = i1;
  n1p = bsearch(&node1, ndb->node_data, ndb->n_node, sizeof(NodeData), node_compar);
  if (n1p == NULL) {
    fprintf(stderr,
            "Error: searching node id failed (node_id may not be sorted)\n"
	    " could not find data for node %lld\n", (long long) i1);
    exit(1);
  }
  return n1p;
}

/* return a square of distance between two nodes */
coord_t node_dist2(NodeDB *ndb, index_t i1, index_t i2)
{
  NodeData *n1p, *n2p;
  coord_t dx, dy, dz;

  n1p = search_node(ndb, i1);
  n2p = search_node(ndb, i2);
  dx = n1p->x - n2p->x;
  dy = n1p->y - n2p->y;
  dz = n1p->z - n2p->z;
  return dx*dx + dy*dy + dz*dz;
}

coord_t penta_vol(NodeDB *ndb, index_t i0, index_t i1, index_t i2, index_t i3)
{
  NodeData *np[4];
  coord_t v1[3], v2[3], v3[3];

  np[0] = search_node(ndb, i0);
  np[1] = search_node(ndb, i1);
  np[2] = search_node(ndb, i2);
  np[3] = search_node(ndb, i3);

  v1[0] = np[1]->x - np[0]->x;
  v1[1] = np[1]->y - np[0]->y;
  v1[2] = np[1]->z - np[0]->z;

  v2[0] = np[2]->x - np[0]->x;
  v2[1] = np[2]->y - np[0]->y;
  v2[2] = np[2]->z - np[0]->z;

  v3[0] = np[3]->x - np[0]->x;
  v3[1] = np[3]->y - np[0]->y;
  v3[2] = np[3]->z - np[0]->z;

  return ((v1[1] * v2[2] - v1[2] * v2[1]) * v3[0] +
	  (v1[2] * v2[0] - v1[0] * v2[2]) * v3[1] +
	  (v1[0] * v2[1] - v1[1] * v2[0]) * v3[2]) / 6.0;
}

void node_coord(NodeDB *ndb, index_t id, coord_t *x, coord_t *y, coord_t *z)
{
  NodeData *np;

  np = search_node(ndb, id);
  *x = np->x;
  *y = np->y;
  *z = np->z;
}

/* return the number of nodes */
index_t number_of_nodes(const NodeDB *ndb)
{
  return ndb->n_node;
}

/* return the local nodeID of node i1 (globalID) */
index_t get_local_node_id(NodeDB *ndb, index_t i1)
{
  index_t li1;
  li1 = search_node(ndb, i1) - ndb->node_data;
  if (ndb->node_data[li1].id != i1) {
    fprintf(stderr, "Error: failed get_local_node_id; "
	    "(node_data[%lld].id = %lld) != (i1 = %lld)\n",
	    (long long) li1, (long long) ndb->node_data[li1].id, (long long) i1);
    exit(1);
  }
  return li1;
}

/* return the global nodeID of node li1 (localID) */
index_t get_global_node_id(const NodeDB *ndb, index_t li1)
{
  return ndb->node_data[li1].id;
}


/* return the number of middle-nodes */
index_t number_of_middle_nodes(const NodeDB *ndb)
{
  return ndb->n_mnode;
}

/* register a middle node between i1 and i2 as a new node */
index_t new_middle_node(NodeDB *ndb, index_t i1, index_t i2)
{
  NodeData *n1p, *n2p;

  if (ndb->n_mnode == 0)
    ndb->middle_node.id = ndb->node_data[ndb->n_node-1].id + 1;
  else
    ndb->middle_node.id++;

  n1p = search_node(ndb, i1);
  n2p = search_node(ndb, i2);

  ndb->middle_node.x = 0.5 * (n1p->x + n2p->x);
  ndb->middle_node.y = 0.5 * (n1p->y + n2p->y);
  ndb->middle_node.z = 0.5 * (n1p->z + n2p->z);
  ndb->n_mnode++;

  return ndb->middle_node.id;
}

/* print node data of the last middle node */
void print_last_middle_node(const NodeDB *ndb, FILE *fp)
{
  fprintf(fp, "%lld,%f,%f,%f\n",
	  (long long) ndb->middle_node.id,
	  ndb->middle_node.x,
	  ndb->middle_node.y,
	  ndb->middle_node.z);
}

/* print node data in Adventure .msh format */
void print_node_adv(const NodeDB *ndb, FILE *fp)
{
  index_t i;
  for (i = 0; i < ndb->n_node; i++)
    fprintf(fp, "%f %f %f\n",
	    ndb->node_data[i].x,
	    ndb->node_data[i].y,
	    ndb->node_data[i].z);
}
