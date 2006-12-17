/*
 * sd342to341.c
 *   Subdevide FrontSTR-format 342 mesh data into 341 mesh data with 8
 *   times as many elements
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 10, 2006
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
#include "refine.h"

#define BIG_ASPECT_RATIO 500

void usage(void)
{
  fprintf(stderr,
	  "Usage: %s [OPTION] [SOURCE [DEST]]\n"
	  "Subdevide FrontSTR-format 342 mesh file SOURCE, "
	  "Refine FrontSTR-format 341 mesh file SOURCE, "
	  "or standard input, into "
	  "341 mesh file DEST, "
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
	  "# FrontSTR 341 mesh file subdevided by %s\n"
	  "# Date: %s"
	  "# Original 342 mesh: %s\n"
	  "# CAUTION: The mesh may be wrong if you have BCs on surface.\n"
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

typedef struct ARStat {
  double min; /* min of aspect ratio */
  double max;     /* max of aspect ratio */
  int min_elem_id;
  int max_elem_id;
} ARStat;

static void arstat_init(ARStat *ars)
{
  ars->min = 1e+10;
  ars->max = 0;
  ars->min_elem_id = -1;
  ars->max_elem_id = -1;
}

static void arstat_update(ARStat *ars, double ar, int elem_id)
{
  if (ar > BIG_ASPECT_RATIO) {
    fprintf(stderr, "warning: big aspect ratio: %f at elem %d\n",
	    ar, elem_id);
  }
  if (ar < ars->min) {
    ars->min = ar;
    ars->min_elem_id = elem_id;
  }
  if (ar > ars->max) {
    ars->max = ar;
    ars->max_elem_id = elem_id;
  }
}

static void print_arstat(const ARStat *ars, FILE *fp)
{
  fprintf(fp,
	  "aspect ratio: min = %f (elemID: %d), max = %f (elemID: %d)\n",
	  ars->min, ars->min_elem_id, ars->max, ars->max_elem_id);

}

static void proceed_elem_data(const char *line,
			      NodeDB *ndb,
			      FILE *to_file,
			      ARStat *ars)
{
  int elem_id, n[10];
  double ndist47, ndist58, ndist69;
  double ar;

  if (sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	     &elem_id, &n[0], &n[1], &n[2], &n[3], &n[4],
	     &n[5], &n[6], &n[7], &n[8], &n[9])
      != 11) {
    fprintf(stderr, "Error: reading element data failed\n");
    exit(1);
  }
  fprintf(to_file, "%d,%d,%d,%d,%d\n"
	  "%d,%d,%d,%d,%d\n"
	  "%d,%d,%d,%d,%d\n"
	  "%d,%d,%d,%d,%d\n",
	  8*elem_id-7, n[0], n[6], n[5], n[7],
	  8*elem_id-6, n[5], n[4], n[2], n[9],
	  8*elem_id-5, n[6], n[1], n[4], n[8],
	  8*elem_id-4, n[7], n[8], n[9], n[3]);

  ndist47 = node_dist2(ndb, n[4], n[7]);
  ndist58 = node_dist2(ndb, n[5], n[8]);
  ndist69 = node_dist2(ndb, n[6], n[9]);

  if (ndist47 < ndist58 && ndist47 < ndist69) {
    fprintf(to_file, "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n",
	    8*elem_id-3, n[4], n[7], n[5], n[6],
	    8*elem_id-2, n[4], n[7], n[6], n[8],
	    8*elem_id-1, n[4], n[7], n[8], n[9],
	    8*elem_id, n[4], n[7], n[9], n[5]);
    if (ndist58 < ndist69)
      ar = ndist69/ndist47;
    else
      ar = ndist58/ndist47;
  } else if (ndist58 < ndist69) {
    fprintf(to_file, "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n",
	    8*elem_id-3, n[5], n[8], n[6], n[4],
	    8*elem_id-2, n[5], n[8], n[4], n[9],
	    8*elem_id-1, n[5], n[8], n[9], n[7],
	    8*elem_id, n[5], n[8], n[7], n[6]);
    if (ndist47 < ndist69)
      ar = ndist69/ndist58;
    else
      ar = ndist47/ndist58;
  } else {
    fprintf(to_file, "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n"
	    "%d,%d,%d,%d,%d\n",
	    8*elem_id-3, n[6], n[9], n[4], n[5],
	    8*elem_id-2, n[6], n[9], n[5], n[7],
	    8*elem_id-1, n[6], n[9], n[7], n[8],
	    8*elem_id, n[6], n[9], n[8], n[4]);
    if (ndist47 < ndist58)
      ar = ndist58/ndist69;
    else
      ar = ndist47/ndist69;
  }
  arstat_update(ars, ar, elem_id);
}

void refine(FILE *from_file, const char *from_file_name,
	    FILE *to_file, int verbose)
{
  char *line;
  int mode;
  int header, header_prev = NONE;
  MeshIO mio;
  NodeDB nodeDB;
  ARStat ars;

  if (verbose)
    print_log(stderr, "Starting mesh-type conversion...");

  print_header(to_file, from_file_name);

  meshio_init(&mio, from_file);
  node_init(&nodeDB);
  arstat_init(&ars);

  while ((line = meshio_readline(&mio, &mode, &header)) != NULL) {

    if (mode == COMMENT) {
      fprintf(to_file, "%s", line);
      continue;
    }

    if (mode == HEADER) {
      /* check the previous header */
      if (verbose) {
	if (header_prev == NODE && header != NODE)
	  print_log(stderr, "reading NODE-part completed.");
	else if (header_prev == ELEMENT && header != ELEMENT)
	  print_log(stderr, "reading ELEMENT-part completed.");
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
	p_elem_type = strstr(line, "342");
	if (p_elem_type == NULL) {
	  fprintf(stderr, "Error: element type is not \"342\"?\n");
	  exit(1);
	}
	p_elem_type[2] = '1';
	fprintf(to_file, "%s", line);

      } else {
	fprintf(to_file, "%s", line);
      }
      header_prev = header;
      continue;
    }

    assert(mode == DATA);

    if (header == NODE) {
      proceed_node_data(line, &nodeDB, to_file);

    } else if (header == ELEMENT) {
      proceed_elem_data(line, &nodeDB, to_file, &ars);

    } else if (header == EGROUP) {
      int elem_id;

      if (sscanf(line, "%d", &elem_id) != 1) {
	fprintf(stderr, "Error: reading element group data failed\n");
	exit(1);
      }
      fprintf(to_file, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
	      8*elem_id-7, 8*elem_id-6, 8*elem_id-5, 8*elem_id-4,
	      8*elem_id-3, 8*elem_id-2, 8*elem_id-1, 8*elem_id);

    } else {
      fprintf(to_file, "%s", line);
    }
  }

  node_finalize(&nodeDB);
  meshio_finalize(&mio);

  if (verbose) {
    print_log(stderr, "mesh-type conversion completed.");
    print_arstat(&ars, stderr);
  }
}
