/*
 * fstr2adv.c
 *   Convert FrontSTR-format mesh file into Adventure mesh file
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Apr 7, 2006
 * Last modified on May 17, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>      /* for time(), clock() and ctime() */
#include "util.h"
#include "meshio.h"
#include "nodedata.h"
#include "elemdata.h"

static void usage(void)
{
  fprintf(stderr,
	  "%s: Convert FrontSTR-format mesh file into Adventure mesh file\n"
	  "Usage: %s from_file to_file\n",
	  progname(), progname());
  exit(1);
}

int main(int argc, char *argv[])
{
  int verbose = 0;
  FILE *from_file;
  FILE *to_file;
  char *line;
  int mode;
  int header, header_prev = NONE;
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
    default:
      fprintf(stderr, "unknown option -%c\n", argv[0][1]);
      usage();
    }
  }

  if (argc > 2) usage();

  if (verbose)
    print_log(stderr, "Starting mesh-type conversion...");

  if (argc >= 1) {
    from_file = fopen(argv[0], "r");
    if (from_file == NULL) {
      perror(argv[0]);
      exit(2);
    }
  } else {
    from_file = stdin;
  }

  if (argc == 2) {
    to_file = fopen(argv[2], "w");
    if (to_file == NULL) {
      perror(argv[2]);
      exit(2);
    }
  } else {
    to_file = stdout;
  }

  meshio_init(from_file);
  node_init();
  elem_init(NULL);

  while ((line = meshio_readline(&mode, &header)) != NULL) {

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
	if (verbose && header_prev != NODE)
	  print_log(stderr, "Start reading NODE-part...");

      } else if (header == ELEMENT) {
	char *p_elem_type;
	if (verbose && header_prev != ELEMENT)
	  print_log(stderr, "Start reading ELEMENT-part...");
	p_elem_type = strstr(line, "342");
	if (p_elem_type == NULL) {
	  fprintf(stderr, "Error: element type is not \"342\"?\n");
	  exit(1);
	}
      }
      header_prev = header;
      continue;
    }

    /* now mode==DATA */
    if (header == NODE) {
      int node_id;
      double x, y, z;

      if (sscanf(line, "%d,%lf,%lf,%lf", &node_id, &x, &y, &z) != 4) {
	fprintf(stderr, "Error: reading node data failed\n");
	exit(1);
      }
      new_node(node_id, x, y, z);

    } else if (header == ELEMENT) {
      int elem_id, n[10];

      if (sscanf(line,
		 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		 &elem_id, &n[0], &n[1], &n[2], &n[3], &n[4],
		 &n[5], &n[6], &n[7], &n[8], &n[9])
	  != 11) {
	fprintf(stderr, "Error: reading element data failed\n");
	exit(1);
      }
      new_elem(elem_id, n);
    }
  }

  meshio_finalize();
  if (from_file != stdin) fclose(from_file);

  /* element data */
  fprintf(to_file, "%d\n", number_of_elems());
  print_elem_adv(to_file);
  /* node data */
  fprintf(to_file, "%d\n", number_of_nodes());
  print_node_adv(to_file);

  node_finalize();
  elem_finalize();
  if (to_file != stdout) fclose(to_file);

  if (verbose) {
    print_log(stderr, "mesh-type conversion completed.");

    after_c = clock();
    fprintf(stderr, " Total time: %.2f sec\n",
	    (double) (after_c - before_c) / (double) CLOCKS_PER_SEC);
  }

  return 0;
}
