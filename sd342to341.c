/*
 * sd342to341.c
 *   Subdevide FrontSTR-format 342 mesh data into 341 mesh data with 8
 *   times as many elements
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 10, 2006
 * Last modified on May 16, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "meshio.h"
#include "nodedata.h"

static void print_log(FILE *log_file, char *log_mesg);


int main(int argc, char *argv[])
{
  char *progname;
  char logname[64];
  FILE *log_file;

  FILE *from_file;
  char from_file_name[64];
  FILE *to_file;

  char *line;
  int mode;
  int header, header_prev = NONE;

  double armin = 1e+10; /* min of aspect ratio */
  double armax = 0;     /* max of aspect ratio */

  clock_t before_c, after_c;

  before_c = clock();

  /* progname = basename(argv[0]); */
  if ((progname = strrchr(argv[0], '/')) == NULL &&
      (progname = strrchr(argv[0], '\\')) == NULL) {
    fprintf(stderr, "strange path??\n");
    progname = argv[0];
  } else {
    progname++;
  }

  if (argc > 3) {
    fprintf(stderr,
	    "%s: Subdevide FrontSTR-format 342 mesh data into 341 mesh data\n"
	    "    with 8 times as many elements\n"
	    "Usage: %s from_file to_file\n",
	    progname, progname);
    exit(1);
  }

  strcpy(logname, progname);
  strcat(logname, ".log");
  log_file = fopen(logname, "w");
  if (log_file == NULL) {
    perror(logname);
    exit(2);
  }
  print_log(log_file, "Starting mesh-type conversion...");

  if (argc >= 2) {
    from_file = fopen(argv[1], "r");
    if (from_file == NULL) {
      perror(argv[1]);
      exit(2);
    }
    strcpy(from_file_name, argv[1]);
  } else {
    from_file = stdin;
    strcpy(from_file_name, "stdin");
  }

  if (argc == 3) {
    to_file = fopen(argv[2], "w");
    if (to_file == NULL) {
      perror(argv[2]);
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
	    "# FrontSTR 341 mesh file subdevided from 342 mesh by %s\n"
	    "# Date: %s"
	    "# Original: %s\n"
	    "##############################################################\n",
	    progname, ctime(&t), from_file_name);
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
      if (header_prev == NODE && header != NODE)
        print_log(log_file, "reading NODE-part completed.");
      else if (header_prev == ELEMENT && header != ELEMENT)
        print_log(log_file, "reading ELEMENT-part completed.");

      /* check the current header */
      if (header == NODE) {
	if (header_prev != NODE)
	  print_log(log_file, "Start reading NODE-part...");
	fprintf(to_file, "%s", line);

      } else if (header == ELEMENT) {
	char *p_elem_type;
	if (header_prev != ELEMENT)
	  print_log(log_file, "Start reading ELEMENT-part...");
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

    /* now mode==DATA */
    if (header == NONE) {
      fprintf(stderr,
	      "Error: unknown file format (no header before data line)\n");
      exit(1);
    }

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
      int elem_id, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10;
      double ndist58, ndist69, ndist710, ar;

      if (sscanf(line,
		 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		 &elem_id, &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10)
	  != 11) {
	fprintf(stderr, "Error: reading element data failed\n");
	exit(1);
      }
      fprintf(to_file, " %d, %d, %d, %d, %d\n"
	      " %d, %d, %d, %d, %d\n"
	      " %d, %d, %d, %d, %d\n"
	      " %d, %d, %d, %d, %d\n",
	      8*elem_id-7, n1, n7, n6, n8,
	      8*elem_id-6, n6, n5, n3, n10,
	      8*elem_id-5, n7, n2, n5, n9,
	      8*elem_id-4, n8, n9, n10, n4);

      ndist58 = node_dist(n5, n8);
      ndist69 = node_dist(n6, n9);
      ndist710 = node_dist(n7, n10);

      if (ndist58 < ndist69 && ndist58 < ndist710) {
	fprintf(to_file, " %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n",
		8*elem_id-3, n5, n8, n6, n7,
		8*elem_id-2, n5, n8, n7, n9,
		8*elem_id-1, n5, n8, n9, n10,
		8*elem_id, n5, n8, n10, n6);
	if (ndist69 < ndist710)
	  ar = ndist710/ndist58;
	else
	  ar = ndist69/ndist58;
      } else if (ndist69 < ndist710) {
	fprintf(to_file, " %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n",
		8*elem_id-3, n6, n9, n7, n5,
		8*elem_id-2, n6, n9, n5, n10,
		8*elem_id-1, n6, n9, n10, n8,
		8*elem_id, n6, n9, n8, n7);
	if (ndist58 < ndist710)
	  ar = ndist710/ndist69;
	else
	  ar = ndist58/ndist69;
      } else {
	fprintf(to_file, " %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n",
		8*elem_id-3, n7, n10, n5, n6,
		8*elem_id-2, n7, n10, n6, n8,
		8*elem_id-1, n7, n10, n8, n9,
		8*elem_id, n7, n10, n9, n5);
	if (ndist58 < ndist69)
	  ar = ndist69/ndist710;
	else
	  ar = ndist58/ndist710;
      }
      if (ar < armin) armin = ar;
      if (ar > armax) armax = ar;

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

  fprintf(log_file, "aspect ratio: min = %f, max = %f\n", armin, armax);
  print_log(log_file, "mesh-type conversion completed.");

  after_c = clock();
  fprintf(log_file, " Total time: %.2f sec\n",
	  (double) (after_c - before_c) / (double) CLOCKS_PER_SEC);

  fclose(log_file);
  return 0;
}


static void print_log(FILE *log_file, char *log_mesg)
{
  time_t t;
  char date_str[32];

  time(&t);
  strcpy(date_str, ctime(&t));
  *strchr(date_str, '\n') = '\0';
  fprintf(log_file, "%s: %s\n", date_str, log_mesg);
  fflush(log_file);
}
