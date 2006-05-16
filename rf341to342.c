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
#include "meshio.h"
#include "nodedata.h"
#include "edgedata.h"

#define BUFSIZE 1024

static void print_log(FILE *log_file, char *log_mesg);


int main(int argc, char *argv[])
{
  char *progname;
  char logname[64];
  FILE *log_file;

  FILE *from_file;
  FILE *to_file;
  /* char tmpname[64]; */ /* for debugging */
  FILE *tmp_file;

  char *line;
  int mode;
  int header, header_prev = NONE;

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
    exit(2);
  }
  print_log(log_file, "Starting mesh-type conversion...");

  from_file = fopen(argv[1], "r");
  if (from_file == NULL) {
    perror(argv[1]);
    exit(2);
  }

  to_file = fopen(argv[2], "w");
  if (to_file == NULL) {
    perror(argv[2]);
    exit(2);
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
	    "# FrontSTR 342 mesh file refined from 341 mesh by %s\n"
	    "# Date: %s"
	    "# Original: %s\n"
	    "##############################################################\n",
	    progname, ctime(&t), argv[1]);
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
	print_log(log_file, "reading NODE-part completed.");

	edge_init();

      } else if (header_prev == ELEMENT && header != ELEMENT) {
	char tmpbuf[BUFSIZE];

	print_log(log_file, "reading ELEMENT-part completed.");
	print_edge_stat(log_file);

	/* copy tmp_file to to_file */
	print_log(log_file, "Copying element data... ");
	rewind(tmp_file);
	while (fgets(tmpbuf, sizeof(tmpbuf), tmp_file))
	  fputs(tmpbuf, to_file);
	print_log(log_file, "done.");
      }

      /* check the current header */
      if (header == NODE) {
	if (header_prev != NODE)
	  print_log(log_file, "Start reading NODE-part...");
	fprintf(to_file, "%s", line);

      } else if (header == ELEMENT) {
	char *p_elem_type;
	if (header_prev != ELEMENT)
	  print_log(log_file, "Start reading ELEMENT-part...");
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
  fclose(from_file);
  fclose(to_file);

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
