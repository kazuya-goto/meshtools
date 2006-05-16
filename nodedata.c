/*
 * nodedata.c
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last Modified: May 16, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nodedata.h"

typedef struct NodeData {
  int id;
  double x;
  double y;
  double z;
  int new_id;
} NodeData;

enum { MAX_NODE_INIT = 1024, MAX_NODE_GROW = 2 };

static int n_node = 0;
static int n_mnode = 0;

static NodeData *node_data;
static int max_node;
static int issorted = 1;

void node_init(void)
{
  node_data = (NodeData *) malloc(MAX_NODE_INIT * sizeof(NodeData));
  if (node_data == NULL) {
    perror("in node_init()");
    exit(2);
  }
  max_node = MAX_NODE_INIT;
}

void node_finalize(void)
{
  free(node_data);
}

void new_node(int id, double x, double y, double z)
{
  if (n_node == max_node) {
    NodeData *ndp;

    ndp = (NodeData *) realloc(node_data, MAX_NODE_GROW * max_node * sizeof(NodeData));
    if (ndp == NULL) {
      perror("new_node()");
      exit(2);
    }

    max_node *= MAX_NODE_GROW;
    node_data = ndp;
  }

  if (n_node > 0 && node_data[n_node-1].id >= id) {
    fprintf(stderr, "Warning: node id is not sorted\n");
    issorted = 0;
  }

  node_data[n_node].id = id;
  node_data[n_node].x = x;
  node_data[n_node].y = y;
  node_data[n_node].z = z;

  n_node++;
}

static int node_compar(const void *vn1, const void *vn2)
{
  const NodeData *n1, *n2;

  n1 = (NodeData *) vn1;
  n2 = (NodeData *) vn2;

  if (n1->id < n2->id) return -1;
  else if (n1->id == n2->id) return 0;
  else return 1;
}

static NodeData *search_node(int i1)
{
  NodeData node1, *n1p;

  if (!issorted) {
    fprintf(stderr, "start sorting node data... ");
    qsort(node_data, n_node, sizeof(NodeData), node_compar);
    fprintf(stderr, "done.\n");
    issorted = 1;
  }

  node1.id = i1;
  n1p = bsearch(&node1, node_data, n_node, sizeof(NodeData), node_compar);
  if (n1p == NULL) {
    fprintf(stderr,
            "Error: searching node id failed (node_id may not be sorted)\n"
	    " could not find data for node %d\n", i1);
    exit(1);
  }
  return n1p;
}

double node_dist(int i1, int i2)
{
  NodeData *n1p, *n2p;

  n1p = search_node(i1);
  n2p = search_node(i2);
  return sqrt((n1p->x - n2p->x)*(n1p->x - n2p->x) +
	      (n1p->y - n2p->y)*(n1p->y - n2p->y) +
	      (n1p->z - n2p->z)*(n1p->z - n2p->z));
}

int number_of_nodes(void)
{
  return n_node;
}

int number_of_middle_nodes(void)
{
  return n_mnode;
}

int get_local_node_id(int i1)
{
  int li1;
  li1 = search_node(i1) - node_data;
  if (node_data[li1].id != i1) {
    fprintf(stderr, "Error: failed get_local_node_id; "
	    "(node_data[%d].id = %d) != (i1 = %d)\n",
	    li1, node_data[li1].id, i1);
    exit(1);
  }
  return li1;
}

int get_global_node_id(int li1)
{
  return node_data[li1].id;
}

int new_middle_node(int i1, int i2)
{
  int mnid;
  NodeData *n1p, *n2p;

  mnid = node_data[n_node-1].id + 1;
  n1p = search_node(i1);
  n2p = search_node(i2);

  new_node(mnid,
	   0.5 * (n1p->x + n2p->x),
	   0.5 * (n1p->y + n2p->y),
	   0.5 * (n1p->z + n2p->z));
  n_mnode++;
  return mnid;
}

void print_last_node_data_line(FILE *fp)
{
  NodeData *n1p;
  int rv;

  n1p = &(node_data[n_node-1]);
  rv = fprintf(fp, " %d, %f, %f, %f\n",
	       n1p->id, n1p->x, n1p->y, n1p->z);
}

void print_node_adv(FILE *fp)
{
  int i;
  for (i = 0; i < n_node; i++) {
    fprintf(fp, "%f %f %f\n", node_data[i].x, node_data[i].y, node_data[i].z);
  }
}
