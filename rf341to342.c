/*
 * rf341to342.c
 *   Refine FrontSTR-format 341 mesh data into 342 mesh data
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last modified on May 16, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util.h"
#include "meshio.h"
#include "nodedata.h"
#include "edgedata.h"

#define BUFSIZE 1024

static void usage(void)
{
  fprintf(stderr,
	  "%s: Refine FrontSTR-format 341 mesh data into 342 mesh data\n"
	  "Usage: %s from_file to_file\n",
	  progname(), progname());
  exit(1);
}

int main(int argc, char *argv[])
{
  int verbose = 0;
  FILE *from_file;
  char from_file_name[64];
  FILE *to_file;
  /* char tmpname[64]; */ /* for debugging */
  FILE *tmp_file;
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

  tmp_file = tmpfile();
  /* for debugging...
  strcpy(tmpname, progname);
  strcat(tmpname, ".tmp");
  tmp_file = fopen(tmpname, "w+");
  */
  if (tmp_file == NULL) {
    perror("tmpfile");
    exit(2);
  }

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

  meshio_init(from_file);
  node_init();

  while ((line = meshio_readline(&mode, &header)) != NULL) {

    if (mode == COMMENT) {
      fprintf(to_file, "%s", line);
      continue;
    }

    if (mode == HEADER) {
      /* check the previous header */
      if (header_prev == NODE && header != NODE) {
	if (verbose)
	  print_log(stderr, "reading NODE-part completed.");
	edge_init();

      } else if (header_prev == ELEMENT && header != ELEMENT) {
	char tmpbuf[BUFSIZE];

	if (verbose) {
	  print_log(stderr, "reading ELEMENT-part completed.");
	  print_edge_stat(stderr);
	}

	/* copy tmp_file to to_file */
	if (verbose)
	  print_log(stderr, "Copying element data... ");
	rewind(tmp_file);
	while (fgets(tmpbuf, sizeof(tmpbuf), tmp_file))
	  fputs(tmpbuf, to_file);
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

    /* now mode==DATA */
    if (header == NODE) {
      int node_id;
      double x, y, z;

      if (sscanf(line, "%d,%lf,%lf,%lf", &node_id, &x, &y, &z) != 4) {
	fprintf(stderr, "Error: reading node data failed\n");
	exit(1);
      }
      new_node(node_id, x, y, z);

      fprintf(to_file, "%s", line);

    } else if (header == ELEMENT) {
      int elem_id, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, dummy;

      if (sscanf(line,
		 "%d,%d,%d,%d,%d,%d",
		 &elem_id, &n1, &n2, &n3, &n4, &dummy)
	  != 5) {
	fprintf(stderr, "Error: reading element data failed\n");
	exit(1);
      }

      if (middle_node(n2, n3, &n5))
	print_last_node_data_line(to_file);
      if (middle_node(n1, n3, &n6))
	print_last_node_data_line(to_file);
      if (middle_node(n1, n2, &n7))
	print_last_node_data_line(to_file);
      if (middle_node(n1, n4, &n8))
	print_last_node_data_line(to_file);
      if (middle_node(n2, n4, &n9))
	print_last_node_data_line(to_file);
      if (middle_node(n3, n4, &n10))
	print_last_node_data_line(to_file);

      fprintf(tmp_file, " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
	      elem_id, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

    } else {
      fprintf(to_file, "%s", line);
    }
  }

  edge_finalize();
  node_finalize();
  meshio_finalize();

  fclose(tmp_file);
  if (from_file != stdin) fclose(from_file);
  if (to_file != stdout) fclose(to_file);

  if (verbose) {
    print_log(stderr, "mesh-type conversion completed.");

    after_c = clock();
    fprintf(stderr, " Total time: %.2f sec\n",
	    (double) (after_c - before_c) / (double) CLOCKS_PER_SEC);
  }

  return 0;
}
