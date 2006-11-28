/*
 * sd342to341.c
 *   Subdevide FrontSTR-format 342 mesh data into 341 mesh data with 8
 *   times as many elements
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 10, 2006
 * Last modified on Nov 22, 2006
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

#define BIG_ASPECT_RATIO 500

static void usage(void)
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

int main(int argc, char *argv[])
{
  int verbose = 0;
  FILE *from_file;
  char from_file_name[64];
  FILE *to_file;
  char *line;
  int mode;
  int header, header_prev = NONE;
  double armin = 1e+10; /* min of aspect ratio */
  double armax = 0;     /* max of aspect ratio */
  int armin_elem_id = -1;
  int armax_elem_id = -1;
  clock_t before_c, after_c;

  before_c = clock();

  setprogname(argv[0]);
  argc--;
  argv++;

  for (; argc > 0; argc--, argv++) {
    if (argv[0][0] != '-')
      break;
    switch (argv[0][1]) {
    case 'v':
      verbose++;
      break;
    case 'h':
      usage();
    default:
      fprintf(stderr, "Error: unknown option -%c\n", argv[0][1]);
      usage();
    }
  }

  if (argc > 2) {
    fprintf(stderr, "Error: too many arguments\n");
    usage();
  }

  if (verbose)
    print_log(stderr, "Starting mesh-type conversion...");

  if (argc >= 1) {
    from_file = fopen(argv[0], "r");
    if (from_file == NULL) {
      perror(argv[0]);
      exit(2);
    }
    strcpy(from_file_name, argv[0]);
  } else {
    from_file = stdin;
    strcpy(from_file_name, "stdin");
  }

  if (argc == 2) {
    to_file = fopen(argv[1], "w");
    if (to_file == NULL) {
      perror(argv[1]);
      exit(2);
    }
  } else {
    to_file = stdout;
  }

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

  meshio_init(from_file);
  node_init();

  while ((line = meshio_readline(&mode, &header)) != NULL) {

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
      int node_id;
      double x, y, z;

      if (sscanf(line, "%d,%lf,%lf,%lf", &node_id, &x, &y, &z) != 4) {
	fprintf(stderr, "Error: reading node data failed\n");
	exit(1);
      }
      new_node(node_id, x, y, z);

      /* fprintf(to_file, "%s", line); */
      fprintf(to_file, "%d,%f,%f,%f\n", node_id, x, y, z);

    } else if (header == ELEMENT) {
      int elem_id, n[10];
      double ndist47, ndist58, ndist69, ar;

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

      ndist47 = node_dist2(n[4], n[7]);
      ndist58 = node_dist2(n[5], n[8]);
      ndist69 = node_dist2(n[6], n[9]);

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
      if (ar < armin) {
	armin = ar;
	armin_elem_id = elem_id;
      }
      if (ar > armax) {
	armax = ar;
	armax_elem_id = elem_id;
      }
      if (ar > BIG_ASPECT_RATIO) {
	fprintf(stderr, "warning: big aspect ratio: %f at elem %d\n",
		ar, elem_id);
      }

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

  node_finalize();
  meshio_finalize();

  if (from_file != stdin) fclose(from_file);
  if (to_file != stdout) fclose(to_file);

  if (verbose) {
    print_log(stderr, "mesh-type conversion completed.");
    fprintf(stderr,
	    "aspect ratio: min = %f (elemID: %d), max = %f (elemID: %d)\n",
	    armin, armin_elem_id, armax, armax_elem_id);

    after_c = clock();
    fprintf(stderr, " Total time: %.2f sec\n",
	    (double) (after_c - before_c) / (double) CLOCKS_PER_SEC);
  }

  return 0;
}
