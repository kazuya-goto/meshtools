/*
 * main.c
 *   Main routine for sd342to341 and rf341to342
 *
 * Author: Kazuya Goto <goto@multi.k.u-tokyo.ac.jp>
 * Created on Dec 15, 2006
 * Last modified on Jun 24, 2013
 *
 */
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "refine.h"

int main(int argc, char *argv[])
{
  int verbose = 0;
  FILE *from_file;
  char from_file_name[64];
  FILE *to_file;
  double tc_s, tc_e;
  double te_s, te_e;

  tc_s = get_cputime();
  te_s = get_wtime();

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
    case 'h':
      usage();
    default:
      fprintf(stderr, "Error: unknown option -%c\n", argv[0][1]);
      usage();
    }
  }

  if (argc > 2) {
    fprintf(stderr, "Error: too many arguments\n");
    usage();
  }

  if (argc >= 1) {
    from_file = efopen(argv[0], "r");
    strcpy(from_file_name, argv[0]);
  } else {
    from_file = stdin;
    strcpy(from_file_name, "stdin");
  }

  if (argc == 2)
    to_file = efopen(argv[1], "w");
  else
    to_file = stdout;

  refine(from_file, from_file_name, to_file, verbose);

  if (from_file != stdin) fclose(from_file);
  if (to_file != stdout) fclose(to_file);

  if (verbose) {
    tc_e = get_cputime();
    te_e = get_wtime();
    fprintf(stderr, " Total time [sec]: %.3f (cpu), %.3f (elapse)\n",
            tc_e - tc_s, te_e - te_s);
  }

  return 0;
}
