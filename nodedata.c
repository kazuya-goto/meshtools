/*
 * nodedata.c
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last Modified: Feb 5, 2007
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "nodedata.h"
#include "util.h"

enum { MAX_NODE_INIT = 1024, MAX_NODE_GROW = 2 };

/* initialize node_data */
void node_init(NodeDB *ndb)
{
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
}

/* resize node_data */
static void resize_node_data(NodeDB *ndb, int len)
{
  ndb->node_data = (NodeData *) erealloc(ndb->node_data, len * sizeof(NodeData));
  ndb->max_node = len;
}

/* register a new node in node_data */
void new_node(NodeDB *ndb, int id, double x, double y, double z)
{
  if (ndb->n_node == ndb->max_node)
    resize_node_data(ndb, ndb->max_node * MAX_NODE_GROW);

  if (ndb->n_node > 0 && ndb->node_data[ndb->n_node-1].id >= id) {
    fprintf(stderr, "Warning: node id is not sorted\n");
    ndb->issorted = 0;
  }

  ndb->node_data[ndb->n_node].id = id;
  ndb->node_data[ndb->n_node].x = (coord_t) x;
  ndb->node_data[ndb->n_node].y = (coord_t) y;
  ndb->node_data[ndb->n_node].z = (coord_t) z;

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
static NodeData *search_node(NodeDB *ndb, int i1)
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
	    " could not find data for node %d\n", i1);
    exit(1);
  }
  return n1p;
}

/* return a square of distance between two nodes */
double node_dist2(NodeDB *ndb, int i1, int i2)
{
  NodeData *n1p, *n2p;

  n1p = search_node(ndb, i1);
  n2p = search_node(ndb, i2);
  return (double) (n1p->x - n2p->x)*(n1p->x - n2p->x) +
    (n1p->y - n2p->y)*(n1p->y - n2p->y) +
    (n1p->z - n2p->z)*(n1p->z - n2p->z);
}

double penta_vol(NodeDB *ndb, int i0, int i1, int i2, int i3)
{
  NodeData *np[4];
  double v1[3], v2[3], v3[3];

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

/* return the number of nodes */
int number_of_nodes(const NodeDB *ndb)
{
  return ndb->n_node;
}

/* return the local nodeID of node i1 (globalID) */
int get_local_node_id(NodeDB *ndb, int i1)
{
  int li1;
  li1 = search_node(ndb, i1) - ndb->node_data;
  if (ndb->node_data[li1].id != i1) {
    fprintf(stderr, "Error: failed get_local_node_id; "
	    "(node_data[%d].id = %d) != (i1 = %d)\n",
	    li1, ndb->node_data[li1].id, i1);
    exit(1);
  }
  return li1;
}

/* return the global nodeID of node li1 (localID) */
int get_global_node_id(const NodeDB *ndb, int li1)
{
  return ndb->node_data[li1].id;
}


/* return the number of middle-nodes */
int number_of_middle_nodes(const NodeDB *ndb)
{
  return ndb->n_mnode;
}

/* register a middle node between i1 and i2 as a new node */
int new_middle_node(NodeDB *ndb, int i1, int i2)
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
  fprintf(fp, "%d,%f,%f,%f\n",
	  ndb->middle_node.id,
	  ndb->middle_node.x,
	  ndb->middle_node.y,
	  ndb->middle_node.z);
}

/* print node data in Adventure .msh format */
void print_node_adv(const NodeDB *ndb, FILE *fp)
{
  int i;
  for (i = 0; i < ndb->n_node; i++)
    fprintf(fp, "%f %f %f\n",
	    ndb->node_data[i].x,
	    ndb->node_data[i].y,
	    ndb->node_data[i].z);
}
