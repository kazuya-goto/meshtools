/*
 * sd342to341.c
 *   Subdevide FrontSTR-format 342 mesh data into 341 mesh data with 8
 *   times as many elements
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 10, 2006
 * Last modified on Feb 5, 2007
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
  coord_t x, y, z;

  if (sscanf(line, "%d,%f,%f,%f", &node_id, &x, &y, &z) != 4) {
    fprintf(stderr, "Error: reading node data failed\n");
    exit(1);
  }
  new_node(ndb, node_id, x, y, z);

  /* fprintf(to_file, "%s", line); */
  fprintf(to_file, "%d,%f,%f,%f\n", node_id, x, y, z);
}

typedef struct ARStat {
  coord_t min; /* min of aspect ratio */
  coord_t max;     /* max of aspect ratio */
  int min_elem_id;
  int max_elem_id;
  coord_t vmin;
  coord_t vmax;
  int vmin_elem_id;
  int vmax_elem_id;
} ARStat;

static void arstat_init(ARStat *ars)
{
  ars->min = 1e+10;
  ars->max = 0;
  ars->min_elem_id = -1;
  ars->max_elem_id = -1;
  ars->vmin = 1e+10;
  ars->vmax = 0;
  ars->vmin_elem_id = -1;
  ars->vmax_elem_id = -1;
}

static void arstat_update(ARStat *ars, coord_t ar, coord_t vr, int elem_id, int *nerr)
{
  if (ar > BIG_ASPECT_RATIO) {
    fprintf(stderr, "warning: big aspect ratio: %f at elem %d\n",
	    ar, elem_id);
    (*nerr)++;
  }
  if (ar < ars->min) {
    ars->min = ar;
    ars->min_elem_id = elem_id;
  }
  if (ar > ars->max) {
    ars->max = ar;
    ars->max_elem_id = elem_id;
  }
  if (vr < 0.5 || vr > 2.0) {
    fprintf(stderr, "warning: strange volume ratio: %f at elem %d\n",
	    vr, elem_id);
    (*nerr)++;
  }
  if (vr < ars->vmin) {
    ars->vmin = vr;
    ars->vmin_elem_id = elem_id;
  }
  if (vr > ars->vmax) {
    ars->vmax = vr;
    ars->vmax_elem_id = elem_id;
  }
}

static void print_arstat(const ARStat *ars, FILE *fp)
{
  fprintf(fp,
	  "aspect ratio: min = %f (elemID: %d), max = %f (elemID: %d)\n",
	  ars->min, ars->min_elem_id, ars->max, ars->max_elem_id);
  fprintf(fp,
	  "volume ratio: min = %f (elemID: %d), max = %f (elemID: %d)\n",
	  ars->vmin, ars->vmin_elem_id, ars->vmax, ars->vmax_elem_id);
}

static coord_t volcheck(int eid, int n,
		    int n0, int n1, int n2, int n3, NodeDB *ndb, int *nerr)
{
  coord_t vol;
  if ((vol = penta_vol(ndb, n0, n1, n2, n3)) <= 0) {
    fprintf(stderr, "Warning: negative volume: %e at elem %d/%d\n",
	    vol, eid, n);
    (*nerr)++;
  }
  return vol;
}

static void elemout(int elem_id, int *n, NodeDB *ndb)
{
  char fname[128];
  FILE *fp;
  int i;
  coord_t x, y, z;

  sprintf(fname, "e%d.inp", elem_id);
  fp = efopen(fname, "w");

  fprintf(fp, "1\ndata\nstep1\n10 1\n");
  for (i = 0; i < 10; i++) {
    node_coord(ndb, n[i], &x, &y, &z);
    fprintf(fp, "%d %f %f %f\n", i+1, x, y, z);
  }
  fprintf(fp, "1 0 tet2 1 2 4 3 7 8 6 9 10 5\n"
	  "1 0\n1 1\nID,\n");
  for (i = 0; i < 10; i++) {
    fprintf(fp, "%d %d\n", i+1, n[i]);
  }
  fclose(fp);
}

static void proceed_elem_data(const char *line,
			      NodeDB *ndb,
			      FILE *to_file,
			      ARStat *ars)
{
  int elem_id, n[10];
  coord_t ndist47, ndist58, ndist69;
  coord_t ar, vol1, vol8;
  int nerr;

  if (sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	     &elem_id, &n[0], &n[1], &n[2], &n[3], &n[4],
	     &n[5], &n[6], &n[7], &n[8], &n[9])
      != 11) {
    fprintf(stderr, "Error: reading element data failed\n");
    exit(1);
  }

  nerr = 0;
  vol1 = volcheck(elem_id, 0, n[0], n[1], n[2], n[3], ndb, &nerr);
  vol8 = 0;

  fprintf(to_file, "%d,%d,%d,%d,%d\n"
	  "%d,%d,%d,%d,%d\n"
	  "%d,%d,%d,%d,%d\n"
	  "%d,%d,%d,%d,%d\n",
	  8*elem_id-7, n[0], n[6], n[5], n[7],
	  8*elem_id-6, n[6], n[1], n[4], n[8],
	  8*elem_id-5, n[5], n[4], n[2], n[9],
	  8*elem_id-4, n[7], n[8], n[9], n[3]);

  vol8 += volcheck(elem_id, 1, n[0], n[6], n[5], n[7], ndb, &nerr);
  vol8 += volcheck(elem_id, 2, n[6], n[1], n[4], n[8], ndb, &nerr);
  vol8 += volcheck(elem_id, 3, n[5], n[4], n[2], n[9], ndb, &nerr);
  vol8 += volcheck(elem_id, 4, n[7], n[8], n[9], n[3], ndb, &nerr);

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

    vol8 += volcheck(elem_id, 5, n[4], n[7], n[5], n[6], ndb, &nerr);
    vol8 += volcheck(elem_id, 6, n[4], n[7], n[6], n[8], ndb, &nerr);
    vol8 += volcheck(elem_id, 7, n[4], n[7], n[8], n[9], ndb, &nerr);
    vol8 += volcheck(elem_id, 8, n[4], n[7], n[9], n[5], ndb, &nerr);

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

    vol8 += volcheck(elem_id, 5, n[5], n[8], n[6], n[4], ndb, &nerr);
    vol8 += volcheck(elem_id, 6, n[5], n[8], n[4], n[9], ndb, &nerr);
    vol8 += volcheck(elem_id, 7, n[5], n[8], n[9], n[7], ndb, &nerr);
    vol8 += volcheck(elem_id, 8, n[5], n[8], n[7], n[6], ndb, &nerr);

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

    vol8 += volcheck(elem_id, 5, n[6], n[9], n[4], n[5], ndb, &nerr);
    vol8 += volcheck(elem_id, 6, n[6], n[9], n[5], n[7], ndb, &nerr);
    vol8 += volcheck(elem_id, 7, n[6], n[9], n[7], n[8], ndb, &nerr);
    vol8 += volcheck(elem_id, 8, n[6], n[9], n[8], n[4], ndb, &nerr);

    if (ndist47 < ndist58)
      ar = ndist58/ndist69;
    else
      ar = ndist47/ndist69;
  }
  arstat_update(ars, ar, vol8/vol1, elem_id, &nerr);

  if (nerr > 0) {
    elemout(elem_id, n, ndb);
  }
}

void refine(FILE *from_file, const char *from_file_name,
	    FILE *to_file, int verbose)
{
  char *line;
  int mode;
  int header, header_prev = NONE;
  MeshIO *mio;
  NodeDB *nodeDB;
  ARStat ars;

  if (verbose)
    print_log(stderr, "Starting mesh-type conversion...");

  print_header(to_file, from_file_name);

  meshio_init(&mio, from_file);
  node_init(&nodeDB);
  arstat_init(&ars);

  while ((line = meshio_readline(mio, &mode, &header)) != NULL) {

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
      proceed_node_data(line, nodeDB, to_file);

    } else if (header == ELEMENT) {
      proceed_elem_data(line, nodeDB, to_file, &ars);

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

  node_finalize(nodeDB);
  meshio_finalize(mio);

  if (verbose) {
    print_log(stderr, "mesh-type conversion completed.");
    print_arstat(&ars, stderr);
  }
}
