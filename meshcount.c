/*
 * meshcount.c
 *   count the number of nodes and elements in FrontSTR mesh file.
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 15, 2006
 * Last modified on May 16, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "meshio.h"

int main(int argc, char *argv[])
{
  char *progname;
  FILE *mesh_file;
  char *line;
  int mode;
  int header;
  int n_node = 0;
  int n_elem = 0;

  /* progname = basename(argv[0]); */
  if ((progname = strrchr(argv[0], '/')) == NULL &&
      (progname = strrchr(argv[0], '\\')) == NULL) {
    fprintf(stderr, "strange path??\n");
    progname = argv[0];
  } else {
    progname++;
  }

  if (argc > 2) {
    fprintf(stderr,
	    "%s: count the number of nodes and elements in FrontSTR mesh file\n"
	    "Usage: %s mesh_file\n",
	    progname, progname);
    exit(1);
  }

  if (argc == 2) {
    mesh_file = fopen(argv[1], "r");
    if (mesh_file == NULL) {
      perror(argv[1]);
      exit(2);
    }
  } else {
    mesh_file = stdin;
  }

  meshio_init(mesh_file);

  while ((line = meshio_readline(&mode, &header)) != NULL) {

    if (mode == COMMENT) continue;
    if (mode == HEADER) continue;

    /* now mode==DATA */
    if (header == NONE) {
      fprintf(stderr,
	      "Error: unknown file format (no header before data line)\n");
      exit(1);
    }
    if (header == NODE) n_node++;
    else if (header == ELEMENT) n_elem++;
  }

  meshio_finalize();
  if (mesh_file != stdin) fclose(mesh_file);
  printf("%d nodes, %d elements\n", n_node, n_elem);
  return 0;
}
