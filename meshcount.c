/*
 * meshcount.c
 *   count the number of nodes and elements in FrontSTR mesh file.
 *
 * Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
 * Created on Mar 15, 2006
 * Last modified on May 19, 2006
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "meshio.h"

static void usage(void)
{
  fprintf(stderr, "Usage: %s [OPTION] [MESHFILE]\n"
	  "Count the number of nodes and elements in MESHFILE.\n"
	  "  -h   display help\n",
	  progname());
  exit(1);
}

int main(int argc, char *argv[])
{
  FILE *mesh_file;
  int mode;
  int header;
  int n_node = 0;
  int n_elem = 0;

  setprogname(argv[0]);
  argc--;
  argv++;

  for (; argc > 0; argc--, argv++) {
    if (argv[0][0] != '-')
      break;
    switch (argv[0][1]) {
    case 'h':
      usage();
    default:
      fprintf(stderr, "Error: unknown option -%c\n", argv[0][1]);
      usage();
    }
  }

  if (argc > 1) {
    fprintf(stderr, "Error: too many arguments\n");
    usage();
  }

  if (argc == 1) {
    mesh_file = fopen(argv[0], "r");
    if (mesh_file == NULL) {
      perror(argv[0]);
      exit(2);
    }
  } else {
    mesh_file = stdin;
  }

  meshio_init(mesh_file);

  while (meshio_readline(&mode, &header)) {

    if (mode == COMMENT) continue;
    if (mode == HEADER) continue;

    assert(mode == DATA);

    if (header == NODE) n_node++;
    else if (header == ELEMENT) n_elem++;
  }

  meshio_finalize();
  if (mesh_file != stdin) fclose(mesh_file);

  printf("%d nodes, %d elements\n", n_node, n_elem);
  return 0;
}
