/*
 * edgedata.c
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last modified: Dec 13, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "nodedata.h"
#include "edgedata.h"
#include "util.h"

struct Edge {
  int onid; /* node id of the other end of the edge */
  int mnid; /* node id of the middle node */
};

typedef struct Edge Edge;

struct EdgeData {
  int nid;
  int n_edge;  /* number of edges to nodes with greater node-ID *
		* these nodes are recorded in edge[].           */
  int n_edge_s; /* number of edges to nodes with smaller node-ID *
		 * these nodes are not recorded.                 */
  Edge *edge; /* edge detail */
  int max_edge; /* allocated length of edge[] */

};

typedef struct EdgeData EdgeData;

struct EdgeDB {
  int n_node_init;
  EdgeData *edge_data;
  NodeDB *ndb;
};

enum { MAX_EDGE_GROW_LEN = 4 };

/* initialize edge_data */
void edge_init(EdgeDB **edb_p, NodeDB *ndb)
{
  int i;
  EdgeDB *edb;

  *edb_p = (EdgeDB *) emalloc(sizeof(EdgeDB));

  edb = *edb_p;

  edb->n_node_init = number_of_nodes(ndb);
  if (edb->n_node_init == 0) {
    fprintf(stderr, "Error: node data not set\n");
    exit(1);
  }

  edb->edge_data = (EdgeData *) emalloc(edb->n_node_init * sizeof(EdgeData));

  for (i = 0; i < edb->n_node_init; i++) {
    edb->edge_data[i].nid = get_global_node_id(ndb, i);
    edb->edge_data[i].n_edge = 0;
    edb->edge_data[i].n_edge_s = 0;
    edb->edge_data[i].edge = NULL;
    edb->edge_data[i].max_edge = 0;
  }

  edb->ndb = ndb;
}

/* finalize edge_data */
void edge_finalize(EdgeDB *edb)
{
  int i;

  for (i = 0; i < edb->n_node_init; i++)
    free(edb->edge_data[i].edge);
  free(edb->edge_data);
  edb->n_node_init = 0;
  edb->ndb = NULL;

  free(edb);
}

/* resize edge_data */
static void resize_edge(EdgeData *edp, int len)
{
  edp->edge = (Edge *) erealloc(edp->edge, len * sizeof(Edge));
  edp->max_edge = len;
}

/* Global node-ID of the middle node between nodes i1 and i2 (global
   IDs) are set in *mnidp.
   Return value is 1 if the middle node is newly created, or 0 if the
   middle node already exists. */
int middle_node(EdgeDB *edb, int i1, int i2, int *mnidp)
{
  int li1, li2; /* local IDs of i1 and i2 */
  EdgeData *edp;
  Edge *ep;
  int j;

  /* make i1 smaller than  i2 */
  if (i1 > i2) {
    int tmp = i1;
    i1 = i2;
    i2 = tmp;
  } else if (i1 == i2) {
    fprintf(stderr, "no edge between identical node\n");
    exit(1);
  }

  li1 = get_local_node_id(edb->ndb, i1);
  edp = &(edb->edge_data[li1]);

  /* if the edge i1-i2 is registered, return the middle-node */
  for (j = 0; j < edp->n_edge; j++) {
    if (edp->edge[j].onid == i2) {
      *mnidp = edp->edge[j].mnid;
      return 0; /* not created */
    }
  }

  /* not found: register as a new edge */
  if (edp->n_edge == edp->max_edge)
    resize_edge(edp, edp->max_edge + MAX_EDGE_GROW_LEN);

  j = edp->n_edge;
  ep = &(edp->edge[j]);

  ep->onid = i2;
  ep->mnid = new_middle_node(edb->ndb, i1, i2);
  edp->n_edge++;

  li2 = get_local_node_id(edb->ndb, i2);
  edb->edge_data[li2].n_edge_s++;

  *mnidp = ep->mnid;
  return 1; /* newly created */
}

/* print statistic data, just for interest. */
void print_edge_stat(const EdgeDB *edb, FILE *log_file)
{
  int neg, ne;
  int min = 10000, max = 0;
  int sum = 0, sumg = 0;
  int n_node_actv = 0;
  float avr;
  int sum_max = 0;
  int i;

  for (i = 0; i < edb->n_node_init; i++) {
    neg = edb->edge_data[i].n_edge;
    ne = neg + edb->edge_data[i].n_edge_s;

    /* for debugging...
    fprintf(log_file, "%d: %d ( %d / %d )\n",
	    edge_data[i].nid, ne, neg, edge_data[i].max_edge);
    */
    sum_max += edb->edge_data[i].max_edge;

    if (ne == 0) continue;

    if (sum == 0) {
      min = max = ne;
    } else {
      if (ne < min) min = ne;
      if (ne > max) max = ne;
    }
    sum += ne;
    sumg += neg;
    n_node_actv++;
  }

  avr = (float) sum / (float) n_node_actv;

  fprintf(log_file,
	  "             initial number of nodes : %d\n"
	  "              number of nodes in use : %d\n"
	  "number of added nodes (middle nodes) : %d\n"
	  "             minimum number of edges : %d\n"
	  "             maximum number of edges : %d\n"
	  "             average number of edges : %f\n"
	  "                used / allocated (%%) : %d / %d (%f%%)\n",
	  edb->n_node_init,
	  n_node_actv,
	  number_of_middle_nodes(edb->ndb),
	  min, max, avr,
	  sumg, sum_max, 100.0*(float)sumg/(float)sum_max);
}
