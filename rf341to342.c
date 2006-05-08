/*
 * rf341to342.c
 *   Refine FrontSTR-format 341 mesh data into 342 mesh data
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 14, 2006
 * Last modified on Mar 17, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nodedata.h"
#include "edgedata.h"

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
  FILE *tmp_file;

  char line[MAXLEN];
  enum header_mode hm = NONE;
#ifdef DEBUG
  char tmpname[64];
#endif

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

  if (argc != 3) {
    fprintf(stderr,
	    "%s: Refine FrontSTR-format 341 mesh data into 342 mesh data\n"
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

#ifdef DEBUG
  strcpy(tmpname, progname);
  strcat(tmpname, ".tmp");
  tmp_file = fopen(tmpname, "w+");
#else
  tmp_file = tmpfile();
#endif
  if (tmp_file == NULL) {
    perror("tmpfile");
    exit(1);
  }

  {
    time_t t;
    time(&t);
    fprintf(to_file,
	    "##############################################################\n"
	    "# FrontSTR 342 mesh file refined from 341 mesh by %s\n"
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
	char tmpbuf[MAXLEN];

	print_log(log_file, "reading ELEMENT-part completed.");
	print_edge_stat(log_file);

	edge_finalize();
	node_finalize();

	/* copy tmp_file to to_file */
	print_log(log_file, "Copying element data... ");
	rewind(tmp_file);
	while (fgets(tmpbuf, MAXLEN, tmp_file)) {
	  fputs(tmpbuf, to_file);
	}
	fclose(tmp_file);
	print_log(log_file, "done.");
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
	p_elem_type = strstr(header, "341");
	if (p_elem_type == NULL) {
	  fprintf(stderr, "Error: element type is not \"341\"?\n");
	  exit(4);
	}
	p_elem_type[2] = '2';
	edge_init();
	fprintf(tmp_file, "%s\n", line);

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
      exit(5);
    }

    if (hm == NODE) {
      int node_id;
      double x, y, z;

      if (sscanf(line, "%d,%lf,%lf,%lf", &node_id, &x, &y, &z) != 4) {
	fprintf(stderr, "Error: reading node data failed\n");
	exit(6);
      }
      new_node(node_id, x, y, z);

      fprintf(to_file, "%s\n", line);

    } else if (hm == ELEMENT) {
      int elem_id, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, dummy;

      if (sscanf(line,
		 "%d,%d,%d,%d,%d,%d",
		 &elem_id, &n1, &n2, &n3, &n4, &dummy)
	  != 5) {
	fprintf(stderr, "Error: reading element data failed\n");
	exit(7);
      }

      if (middle_node(n2, n3, &n5))
	fprintf(to_file, "%s\n", last_node_data_line(line, MAXLEN));
      if (middle_node(n1, n3, &n6))
	fprintf(to_file, "%s\n", last_node_data_line(line, MAXLEN));
      if (middle_node(n1, n2, &n7))
	fprintf(to_file, "%s\n", last_node_data_line(line, MAXLEN));
      if (middle_node(n1, n4, &n8))
	fprintf(to_file, "%s\n", last_node_data_line(line, MAXLEN));
      if (middle_node(n2, n4, &n9))
	fprintf(to_file, "%s\n", last_node_data_line(line, MAXLEN));
      if (middle_node(n3, n4, &n10))
	fprintf(to_file, "%s\n", last_node_data_line(line, MAXLEN));

      fprintf(tmp_file, " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
	      elem_id, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);

    } else {
      fprintf(to_file, "%s\n", line);
    }
  }

  fclose(from_file);
  fclose(to_file);

  print_log(log_file, "mesh-type conversion completed.");

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
