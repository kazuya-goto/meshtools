/*
 * meshcount.c
 *   count the number of nodes and elements in FrontSTR mesh file.
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 15, 2006
 * Last modified on Mar 18, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#define MAXLEN 1024

enum header_mode {NONE, HEADER, NODE, ELEMENT, EGROUP, OTHER};

int main(int argc, char **argv)
{
  char *progname;
  FILE *mesh_file;
  char line[MAXLEN];
  enum header_mode hm = NONE;
  int n_node = 0;
  int n_elem = 0;

  progname = basename(argv[0]);
  if (argc != 2) {
    fprintf(stderr,
	    "%s: count the number of nodes and elements in FrontSTR mesh file\n"
	    "Usage: %s mesh_file\n",
	    progname, progname);
    exit(1);
  }

  mesh_file = fopen(argv[1], "r");
  if (mesh_file == NULL) {
    perror(argv[1]);
    exit(1);
  }

  while (fgets(line, MAXLEN, mesh_file)) {

    /* skip comments */
    if (line[0] == '#' ||
	(line[0] == '!' && line[1] == '!'))
      continue;

    /* header lines */
    if (line[0] == '!') {
      char *header = line+1;
      while (header[0] == ' ') header++;

      if (strncmp(header, "NODE", 4) == 0)
	hm = NODE;
      else if (strncmp(header, "ELEMENT", 7) == 0)
	hm = ELEMENT;
      else if (strncmp(header, "EGROUP", 6) == 0)
	hm = EGROUP;
      else
	hm = OTHER;

      continue;
    }

    /* data lines */
    if (hm == NONE) {
      fprintf(stderr,
	      "Error: unknown file format (no header before data line)\n");
      exit(1);
    }
    if (hm == NODE)
      n_node++;
    else if (hm == ELEMENT)
      n_elem++;
  }

  fclose(mesh_file);
  printf("%d nodes, %d elements\n", n_node, n_elem);
  exit(0);
}
