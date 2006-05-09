/*
 * fstr2adv.c
 *   Convert FrontSTR-format mesh file into Adventure mesh file
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Apr 7, 2006
 * Last modified on Apr 7, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>      /* for time(), clock() and ctime() */
#include "nodedata.h"
#include "elemdata.h"

#define MAXLEN 1024

enum header_mode {NONE, HEADER, NODE, ELEMENT, EGROUP, OTHER};

static void print_log(FILE *log_file, char *log_mesg);


int main(int argc, char *argv[])
{
  char *progname;
  char logname[64];
  FILE *log_file;

  FILE *from_file;
  FILE *to_file;

  char line[MAXLEN];
  enum header_mode hm = NONE;

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
	    "%s: Convert FrontSTR-format mesh file into Adventure mesh file\n"
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

  while (fgets(line, MAXLEN, from_file)) {
    /* remove '\n' at the end of line */
    line[strlen(line)-1] = '\0';

    /* skip comments */
    if (line[0] == '#' ||
	(line[0] == '!' && line[1] == '!')) {
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
      }

      /* check the current header */
      if (strncmp(header, "NODE", 4) == 0) {
        print_log(log_file, "Start reading NODE-part...");
	hm = NODE;
	node_init();

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
	elem_init();

      } else if (strncmp(header, "EGROUP", 6) == 0) {
	hm = EGROUP;

      } else {
	hm = OTHER;

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

    } else if (hm == ELEMENT) {
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

    } else if (hm == EGROUP) {

    } else {

    }
  }

  fclose(from_file);

  to_file = fopen(argv[2], "w");
  if (to_file == NULL) {
    perror(argv[2]);
    exit(1);
  }

  /*  {
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
  */

  fprintf(to_file, "%d\n", number_of_elems());
  print_elem_adv(to_file);

  fprintf(to_file, "%d\n", number_of_nodes());
  print_node_adv(to_file);

  node_finalize();
  elem_finalize();
  fclose(to_file);

  print_log(log_file, "mesh-type conversion completed.");

  after_c = clock();
  fprintf(log_file, " Total time: %.2f sec\n",
	  (double) (after_c - before_c) / (double) CLOCKS_PER_SEC);

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
