/*
 * sd342to341.c
 *   Subdevide FrontSTR-format 342 mesh data into 341 mesh data with 8
 *   times as many elements
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 10, 2006
 * Last modified on Mar 17, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/times.h>
#include <unistd.h>
#include "nodedata.h"

#define MAXLEN 1024

enum header_mode {NONE, HEADER, NODE, ELEMENT, EGROUP, OTHER};

static void print_log(FILE *log_file, char *log_mesg);


int main(int argc, char **argv)
{
  char *progname;
  char logname[64];
  FILE *log_file;

  FILE *from_file;
  FILE *to_file;

  char line[MAXLEN];
  enum header_mode hm = NONE;

  struct tms before, after;
  long cps;
  clock_t before_c, after_c;

  times(&before);
  cps = sysconf(_SC_CLK_TCK);

  before_c = clock();

  progname = basename(argv[0]);
  if (argc != 3) {
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
    exit(1);
  }
  print_log(log_file, "Starting mesh-type conversion...");

  from_file = fopen(argv[1], "r");
  if (from_file == NULL) {
    perror(argv[1]);
    exit(1);
  }

  to_file = fopen(argv[2], "w");
  if (to_file == NULL) {
    perror(argv[2]);
    exit(1);
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
	    progname, ctime(&t), argv[1]);
  }

  while (fgets(line, MAXLEN, from_file)) {
    /* remove '\n' at the end of line */
    line[strlen(line)-1] = '\0';

    /* skip comments */
    if (line[0] == '#' ||
	(line[0] == '!' && line[1] == '!')) {
      fprintf(to_file, "%s\n", line);
      continue;
    }

    /* header lines */
    if (line[0] == '!') {
      char *header = line+1;
      while (header[0] == ' ') header++;


      /* check the previous header mode */
      if (hm == NODE) {
        print_log(log_file, "reading NODE-part completed.");

      } else if (hm == ELEMENT) { /* reading ELEMENT completed! */
        print_log(log_file, "reading ELEMENT-part completed.");
        node_finalize();
      }

      /* check the current header */
      if (strncmp(header, "NODE", 4) == 0) {
        print_log(log_file, "Start reading NODE-part...");
	hm = NODE;
	node_init();
	fprintf(to_file, "%s\n", line);

      } else if (strncmp(header, "ELEMENT", 7) == 0) {
	char *p_elem_type;

        print_log(log_file, "Start reading ELEMENT-part...");
	hm = ELEMENT;
	p_elem_type = strstr(header, "342");
	if (p_elem_type == NULL) {
	  fprintf(stderr, "Error: element type is not \"342\"?\n");
	  exit(1);
	}
	p_elem_type[2] = '1';
	fprintf(to_file, "%s\n", line);

      } else if (strncmp(header, "EGROUP", 6) == 0) {
	hm = EGROUP;
	fprintf(to_file, "%s\n", line);

      } else {
	hm = OTHER;
	fprintf(to_file, "%s\n", line);
      }
      continue;
    }

    /* data lines */
    if (hm == NONE) {
      fprintf(stderr,
	      "Error: unknown file format (no header before data line)\n");
      exit(1);
    }

    if (hm == NODE) {
      int node_id;
      double x, y, z;

      if (sscanf(line, "%d,%lf,%lf,%lf", &node_id, &x, &y, &z) != 4) {
	fprintf(stderr, "Error: reading node data failed\n");
	exit(1);
      }
      new_node(node_id, x, y, z);

      fprintf(to_file, "%s\n", line);

    } else if (hm == ELEMENT) {
      int elem_id, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10;
      double ndist58, ndist69, ndist710;

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
#ifdef DEBUG
	if (ndist69 < ndist710) fprintf(log_file, "%f\n", ndist710/ndist58);
	else fprintf(log_file, "%f\n", ndist69/ndist58);
#endif
      } else if (ndist69 < ndist710) {
	fprintf(to_file, " %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n",
		8*elem_id-3, n6, n9, n7, n5,
		8*elem_id-2, n6, n9, n5, n10,
		8*elem_id-1, n6, n9, n10, n8,
		8*elem_id, n6, n9, n8, n7);
#ifdef DEBUG
	if (ndist58 < ndist710) fprintf(log_file, "%f\n", ndist710/ndist69);
	else fprintf(log_file, "%f\n", ndist58/ndist69);
#endif
      } else {
	fprintf(to_file, " %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n"
		" %d, %d, %d, %d, %d\n",
		8*elem_id-3, n7, n10, n5, n6,
		8*elem_id-2, n7, n10, n6, n8,
		8*elem_id-1, n7, n10, n8, n9,
		8*elem_id, n7, n10, n9, n5);
#ifdef DEBUG
	if (ndist58 < ndist69) fprintf(log_file, "%f\n", ndist69/ndist710);
	else fprintf(log_file, "%f\n", ndist58/ndist710);
#endif
      }

    } else if (hm == EGROUP) {
      int elem_id;

      if (sscanf(line, "%d", &elem_id) != 1) {
	fprintf(stderr, "Error: reading element group data failed\n");
	exit(1);
      }
      fprintf(to_file, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
	      8*elem_id-7, 8*elem_id-6, 8*elem_id-5, 8*elem_id-4,
	      8*elem_id-3, 8*elem_id-2, 8*elem_id-1, 8*elem_id);

    } else {
      fprintf(to_file, "%s\n", line);
    }
  }

  fclose(from_file);
  fclose(to_file);

  print_log(log_file, "mesh-type conversion completed.");

  times(&after);
  fprintf(log_file, "  User time: %.2f sec\n",
          ((double) (after.tms_utime + after.tms_cutime)
           - (before.tms_utime + before.tms_cutime)) / (double) cps);
  fprintf(log_file, "System time: %.2f sec\n",
          ((double) (after.tms_stime + after.tms_cstime)
           - (before.tms_stime + before.tms_cstime)) / (double) cps);

  after_c = clock();
  fprintf(log_file, " Total time: %.2f sec\n",
	  (double) (after_c - before_c) / (double) CLOCKS_PER_SEC);

  exit(0);
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
