/*
 * nodedata.c
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last Modified: Mar 15, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nodedata.h"

typedef struct _NodeData {
  int id;
  double x;
  double y;
  double z;
  int new_id;
} NodeData;

typedef struct _RenumData {
  int id;
  int key;
} RenumData;

enum { MAX_NODE_INIT = 1024, MAX_NODE_GROW = 2 };

static int n_node = 0;
static int n_mnode = 0;

static NodeData *node_data;
static RenumData *renum_data;
static int max_node;

void node_init(void)
{
  node_data = (NodeData *) malloc(MAX_NODE_INIT * sizeof(NodeData));
  if (node_data == NULL) {
    perror("node_init");
    exit(1);
  }

  renum_data = (RenumData *) malloc(MAX_NODE_INIT * sizeof(RenumData));
  if (renum_data == NULL) {
    perror("node_init");
    exit(1);
  }

  max_node = MAX_NODE_INIT;
}

void node_finalize(void)
{
  free(node_data);
  free(renum_data);
}

static void new_node_with_key(int id,
			      double x,
			      double y,
			      double z,
			      int key)
{
  if (n_node == max_node) {
    NodeData *ndp;
    RenumData *rdp;

    ndp = (NodeData *) realloc(node_data, MAX_NODE_GROW * max_node * sizeof(NodeData));
    if (ndp == NULL) {
      perror("new_node");
      exit(1);
    }

    rdp = (RenumData *) realloc(renum_data, MAX_NODE_GROW * max_node * sizeof(RenumData));
    if (renum_data == NULL) {
      perror("new_node");
      exit(1);
    }

    max_node *= MAX_NODE_GROW;
    node_data = ndp;
    renum_data = rdp;
  }

  if (n_node > 0 && node_data[n_node-1].id >= id) {
    fprintf(stderr, "Error: node id is not sorted\n");
    exit(1);
  }

  node_data[n_node].id = id;
  node_data[n_node].x = x;
  node_data[n_node].y = y;
  node_data[n_node].z = z;

  renum_data[n_node].id = id;
  renum_data[n_node].key = key;

  n_node++;
}

void new_node(int id, double x, double y, double z)
{
  new_node_with_key(id, x, y, z, id);
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
#ifdef DEBUG
  static int call = 0;
#endif
  NodeData node1, *n1p;

#ifdef DEBUG
  if (call == 0) {
    fprintf(stderr, "start sorting node data... ");
    qsort(node_data, n_node, sizeof(NodeData), node_compar);
    fprintf(stderr, "done.\n");
    call++;
  }
#endif
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
  NodeData *n1p;
  int li1;

  n1p = search_node(i1);
  li1 = n1p - node_data;
#ifdef DEBUG
  if (n1p->id != node_data[li1].id) {
    fprintf(stderr, "Error: (n1p->id = %d) != (node_data[%d].id = %d)\n",
	    n1p->id, li1, node_data[li1].id);
    exit(1);
  }
#endif
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

  new_node_with_key(mnid,
		    0.5 * (n1p->x + n2p->x),
		    0.5 * (n1p->y + n2p->y),
		    0.5 * (n1p->z + n2p->z),
		    (i1 + i2) / 2);
  n_mnode++;
  return mnid;
}

char *node_data_line(char *line, int maxlen, int i1)
{
  NodeData *n1p;
  int rv;

  n1p = search_node(i1);
  rv = snprintf(line, maxlen, " %d, %f, %f, %f",
		n1p->id, n1p->x, n1p->y, n1p->z);
  if (rv >= maxlen || rv <  0) {
    fprintf(stderr, "Error: too short buffer; please use longer one.\n");
    exit(1);
  }
  return line;
}

char *last_node_data_line(char *line, int maxlen)
{
  NodeData *n1p;
  int rv;

  n1p = &(node_data[n_node-1]);
  rv = snprintf(line, maxlen, " %d, %f, %f, %f",
		n1p->id, n1p->x, n1p->y, n1p->z);
  if (rv >= maxlen || rv <  0) {
    fprintf(stderr, "Error: too short buffer; please use longer one.\n");
    exit(1);
  }
  return line;
}

static int renum_compar(const void *vr1, const void *vr2)
{
  const RenumData *r1, *r2;

  r1 = (RenumData *) vr1;
  r2 = (RenumData *) vr2;

  if (r1->key < r2->key) return -1;
  else if (r1->key > r2->key) return 1;
  else { /* r1->key == r2->key */
    if (r1->id < r2->id) return -1;
    else if (r1->id > r2->id) return 1;
    else { /* r1->id == r2->id */
      fprintf(stderr, "Error: identical nodes in node_data\n");
      exit(1);
    }
  }
}

void node_renum(void)
{
  int i;
  NodeData *n1p;

  qsort(renum_data, n_node, sizeof(RenumData), renum_compar);
  for (i = 0; i < n_node; i++) {
    n1p = search_node(renum_data[i].id);
    n1p->new_id = i+1;
  }
}

void print_node_renum(FILE *fp)
{
  int i;
  NodeData *n1p;

  for (i = 0; i < n_node; i++) {
    n1p = search_node(renum_data[i].id);
    fprintf(fp, " %d, %f, %f, %f\n",
	    n1p->new_id, n1p->x, n1p->y, n1p->z);
  }
}

int get_new_node_id(int old_id)
{
  NodeData *n1p;
  n1p = search_node(old_id);
  return n1p->new_id;
}

void print_node_adv(FILE *fp)
{
  int i;
  for (i = 0; i < n_node; i++) {
    fprintf(fp, "%f %f %f\n", node_data[i].x, node_data[i].y, node_data[i].z);
  }
}
