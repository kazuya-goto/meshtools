/*
 * fstr2adv.c
 *   Convert FrontSTR-format mesh file into Adventure mesh file
 *
 * Author: Kazuya Goto <goto@multi.k.u-tokyo.ac.jp>
 * Created on Apr 7, 2006
 * Last modified on Jun 24, 2013
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>      /* for time(), clock() and ctime() */
#include <assert.h>
#include "util.h"
#include "meshio.h"
#include "nodedata.h"
#include "elemdata.h"
#include "refine.h"

void usage(void)
{
  fprintf(stderr,
	  "Usage: %s [OPTION] [SOURCE [DEST]]\n"
	  "Convert FrontSTR-format mesh file SOURCE, "
	  "or standard input, into "
	  "Adventure-format mesh file DEST, "
	  "or standard output.\n"
	  "  -v   verbose mode\n"
	  "  -h   display help\n",
	  progname());
  exit(1);
}

static void proceed_node_data(const char *line, NodeDB *ndb)
{
  long long node_id;
  float x, y, z;

  if (sscanf(line, "%lld,%f,%f,%f", &node_id, &x, &y, &z) != 4) {
    fprintf(stderr, "Error: reading node data failed\n");
    exit(1);
  }
  new_node(ndb, node_id, x, y, z);
}

static void proceed_elem_data(const char *line,
			      ElemDB *eldb)
{
  int nret, i;
  long long elem_id, nl[10], dummy;
  index_t n[10];

  nret = sscanf(line,
                "%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld",
		&elem_id, &nl[0], &nl[1], &nl[2], &nl[3], &nl[4],
		&nl[5], &nl[6], &nl[7], &nl[8], &nl[9], &dummy);
  if (nret != 5 && nret != 11) {
    fprintf(stderr, "Error: reading element data failed\n");
    exit(1);
  }
  for (i = 0; i < 10; i++) n[i] = nl[i];
  new_elem(eldb, elem_id, n);
}

void refine(FILE *from_file, const char *from_file_name,
	    FILE *to_file, int verbose)
{
  char *line;
  int mode;
  int header, header_prev = NONE;
  MeshIO *mio;
  NodeDB *nodeDB;
  ElemDB *elemDB;

  if (verbose)
    print_log(stderr, "Starting mesh-type conversion...");

  meshio_init(&mio, from_file);

  while ((line = meshio_readline(mio, &mode, &header)) != NULL) {

    if (mode == COMMENT) continue;

    if (mode == HEADER) {
      /* check the previous header mode */
      if (verbose) {
	if (header_prev == NODE && header != NODE)
	  print_log(stderr, "reading NODE-part completed.");
	else if (header_prev == ELEMENT && header != ELEMENT)
	  print_log(stderr, "reading ELEMENT-part completed.");
      }

      /* check the current header */
      if (header == NODE) {
	if (header_prev != NODE) {
	  if (verbose)
	    print_log(stderr, "Start reading NODE-part...");
	  node_init(&nodeDB);
	}

      } else if (header == ELEMENT) {
	if (header_prev != ELEMENT) {
	  if (verbose)
	    print_log(stderr, "Start reading ELEMENT-part...");
	  elem_init(&elemDB, line);
	}
      }
      header_prev = header;
      continue;
    }

    assert(mode == DATA);

    if (header == NODE) {
      proceed_node_data(line, nodeDB);

    } else if (header == ELEMENT) {
      proceed_elem_data(line, elemDB);

    }
  }

  fprintf(to_file, "%lld\n", number_of_elems(elemDB));
  print_elem_adv(elemDB, nodeDB, to_file);

  fprintf(to_file, "%lld\n", number_of_nodes(nodeDB));
  print_node_adv(nodeDB, to_file);

  meshio_finalize(mio);
  node_finalize(nodeDB);
  elem_finalize(elemDB);

  if (verbose)
    print_log(stderr, "mesh-type conversion completed.");
}
