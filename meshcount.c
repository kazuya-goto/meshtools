/*
 * meshcount.c
 *   count the number of nodes and elements in FrontSTR mesh file.
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 15, 2006
 * Last modified on May 17, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "meshio.h"

static void usage(void)
{
  fprintf(stderr,
	  "%s: count the number of nodes and elements in FrontSTR mesh file\n"
	  "Usage: %s mesh_file\n",
	  progname(), progname());
  exit(1);
}

int main(int argc, char *argv[])
{
  FILE *mesh_file;
  char *line;
  int mode;
  int header;
  int n_node = 0;
  int n_elem = 0;

  setprogname(argv[0]);

  if (argc > 2) usage();

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
    if (header == NODE) n_node++;
    else if (header == ELEMENT) n_elem++;
  }

  meshio_finalize();
  if (mesh_file != stdin) fclose(mesh_file);

  printf("%d nodes, %d elements\n", n_node, n_elem);
  return 0;
}
