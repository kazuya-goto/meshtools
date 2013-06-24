/*
 * rf341to342.c
 *   Refine FrontSTR-format 341 mesh data into 342 mesh data
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last modified on Dec 15, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "util.h"
#include "meshio.h"
#include "nodedata.h"
#include "edgedata.h"
#include "refine.h"

void usage(void)
{
  fprintf(stderr,
	  "Usage: %s [OPTION] [SOURCE [DEST]]\n"
	  "Refine FrontSTR-format 341 mesh file SOURCE, "
	  "or standard input, into "
	  "342 mesh file DEST, "
	  "or standard output.\n"
	  "  -v   verbose mode\n"
	  "  -h   display help\n",
	  progname());
  exit(1);
}

static void print_header(FILE *to_file, const char *from_file_name)
{
  time_t t;
  time(&t);
  fprintf(to_file,
	  "##############################################################\n"
	  "# FrontSTR 342 mesh file refined by %s\n"
	  "# Date: %s"
	  "# Original 341 mesh: %s\n"
	  "# CAUTION: BCs are not applied on middle nodes.\n"
	  "##############################################################\n",
	  progname(), ctime(&t), from_file_name);
}

static void proceed_node_data(const char *line, NodeDB *ndb, FILE *to_file)
{
  int node_id;
  double x, y, z;

  if (sscanf(line, "%d,%lf,%lf,%lf", &node_id, &x, &y, &z) != 4) {
    fprintf(stderr, "Error: reading node data failed\n");
    exit(1);
  }
  new_node(ndb, node_id, x, y, z);

  /* fprintf(to_file, "%s", line); */
  fprintf(to_file, "%d,%f,%f,%f\n", node_id, x, y, z);
}

const int mnid[6][3] = {
  {1, 2, 4},
  {0, 2, 5},
  {0, 1, 6},
  {0, 3, 7},
  {1, 3, 8},
  {2, 3, 9}
};

static void proceed_elem_data(const char *line, NodeDB *ndb, EdgeDB *edb,
			      FILE *node_file, FILE *elem_file)
{
  int elem_id, n[10], dummy, i;

  if (sscanf(line,
	     "%d,%d,%d,%d,%d,%d",
	     &elem_id, &n[0], &n[1], &n[2], &n[3], &dummy)
      != 5) {
    fprintf(stderr, "Error: reading element data failed\n");
    exit(1);
  }

  for (i = 0; i < 6; i++)
    if (middle_node(edb, n[mnid[i][0]], n[mnid[i][1]], &n[mnid[i][2]]))
      print_last_middle_node(ndb, node_file);

  fprintf(elem_file,"%d", elem_id);
  for (i = 0; i < 10; i++)
    fprintf(elem_file, ",%d", n[i]);
  fprintf(elem_file, "\n");
}

void refine(FILE *from_file, const char *from_file_name,
	    FILE *to_file, int verbose)
{
  char *line;
  int mode;
  int header, header_prev = NONE;
  MeshIO mio;
  NodeDB *nodeDB;
  EdgeDB *edgeDB;
  FILE *tmp_file;

  if (verbose)
    print_log(stderr, "Starting mesh-type conversion...");

  print_header(to_file, from_file_name);

  meshio_init(&mio, from_file);
  node_init(&nodeDB);
  tmp_file = etmpfile();

  while ((line = meshio_readline(&mio, &mode, &header)) != NULL) {

    if (mode == COMMENT) {
      fprintf(to_file, "%s", line);
      continue;
    }

    if (mode == HEADER) {
      /* check the previous header */
      if (header_prev == NODE && header != NODE) {
	if (verbose)
	  print_log(stderr, "reading NODE-part completed.");
	reduce_node_data(nodeDB);
	edge_init(&edgeDB, nodeDB);

      } else if (header_prev == ELEMENT && header != ELEMENT) {
	if (verbose) {
	  print_log(stderr, "reading ELEMENT-part completed.");
	  print_edge_stat(edgeDB, stderr);
	  print_log(stderr, "Copying element data...");
	}
	rewind(tmp_file);
	file_copy(tmp_file, to_file);
	if (verbose)
	  print_log(stderr, "done.");
      }

      /* check the current header */
      if (header == NODE) {
	if (verbose && header_prev != NODE)
	  print_log(stderr, "Start reading NODE-part...");
	fprintf(to_file, "%s", line);

      } else if (header == ELEMENT) {
	char *p_elem_type;
	if (verbose && header_prev != ELEMENT)
	  print_log(stderr, "Start reading ELEMENT-part...");
	p_elem_type = strstr(line, "341");
	if (p_elem_type == NULL) {
	  fprintf(stderr, "Error: element type is not \"341\"?\n");
	  exit(1);
	}
	p_elem_type[2] = '2';
	fprintf(tmp_file, "%s", line);

      } else {
	fprintf(to_file, "%s", line);
      }
      header_prev = header;
      continue;
    }

    assert(mode == DATA);

    if (header == NODE) {
      proceed_node_data(line, nodeDB, to_file);

    } else if (header == ELEMENT) {
      proceed_elem_data(line, nodeDB, edgeDB, to_file, tmp_file);

    } else {
      fprintf(to_file, "%s", line);
    }
  }

  edge_finalize(edgeDB);
  node_finalize(nodeDB);
  meshio_finalize(&mio);
  fclose(tmp_file);

  if (verbose)
    print_log(stderr, "mesh-type conversion completed.");
}
