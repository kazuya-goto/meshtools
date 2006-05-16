/*
 * edgedata.c
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last modified: May 16, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "nodedata.h"
#include "edgedata.h"

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

enum { MAX_EDGE_INIT = 8, MAX_EDGE_GROW = 8 };

static int n_node_init;
static EdgeData *edge_data;

void edge_init(void)
{
  int i;

  n_node_init = number_of_nodes();
  if (n_node_init == 0) {
    fprintf(stderr, "Error: node data not set\n");
    exit(1);
  }

  edge_data = (EdgeData *) malloc(n_node_init * sizeof(EdgeData));
  if (edge_data == NULL) {
    perror("in edge_init()");
    exit(2);
  }

  for (i = 0; i < n_node_init; i++) {
    edge_data[i].nid = get_global_node_id(i);
    edge_data[i].n_edge = 0;
    edge_data[i].n_edge_s = 0;
    edge_data[i].edge = (Edge *) malloc(MAX_EDGE_INIT * sizeof(Edge));
    if (edge_data[i].edge == NULL) {
      perror("in edge_init()");
      exit(2);
    }
    edge_data[i].max_edge = MAX_EDGE_INIT;
  }
}

void edge_finalize(void)
{
  int i;

  for (i = 0; i < n_node_init; i++) {
    free(edge_data[i].edge);
  }
  free(edge_data);
}

int middle_node(int i1, int i2, int *mnidp)
{
  int li1, li2;
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

  li1 = get_local_node_id(i1);
  edp = &(edge_data[li1]);

  /* if the edge i1-i2 is registered, return the middle-node */
  for (j = 0; j < edp->n_edge; j++) {
    if (edp->edge[j].onid == i2) {
      *mnidp = edp->edge[j].mnid;
      return 0;
    }
  }

  /* not found: register as a new edge */
  if (edp->n_edge == edp->max_edge) {
    Edge *ep;

    ep = (Edge *) realloc(edp->edge, (edp->max_edge + MAX_EDGE_GROW) * sizeof(Edge));
    if (ep == NULL) {
      perror("in middle_node()");
      exit(2);
    }
    edp->max_edge += MAX_EDGE_GROW;
    edp->edge = ep;
  }

  j = edp->n_edge;
  ep = &(edp->edge[j]);

  ep->onid = i2;
  ep->mnid = new_middle_node(i1, i2);
  edp->n_edge++;

  li2 = get_local_node_id(i2);
  edge_data[li2].n_edge_s++;

  *mnidp = ep->mnid;
  return 1;
}

void print_edge_stat(FILE *log_file)
{
  int neg, ne;
  int min, max;
  int sum = 0, sumg = 0;
  int n_node_actv = 0;
  double avr;
  int sum_max = 0;
  int i;

  for (i = 0; i < n_node_init; i++) {
    neg = edge_data[i].n_edge;
    ne = neg + edge_data[i].n_edge_s;

    /* for debugging...
    fprintf(log_file, "%d: %d ( %d / %d )\n",
	    edge_data[i].nid, ne, neg, edge_data[i].max_edge);
    */
    sum_max += edge_data[i].max_edge;

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

  avr = (double) sum / (double) n_node_actv;

  fprintf(log_file,
	  "             initial number of nodes : %d\n"
	  "              number of nodes in use : %d\n"
	  "number of added nodes (middle nodes) : %d\n"
	  "             minimum number of edges : %d\n"
	  "             maximum number of edges : %d\n"
	  "             average number of edges : %f\n"
	  "                used / allocated (%%) : %d / %d (%f%%)\n",
	  n_node_init, n_node_actv, number_of_middle_nodes(), min, max, avr,
	  sumg, sum_max, 100.0*(double)sumg/(double)sum_max);
}
