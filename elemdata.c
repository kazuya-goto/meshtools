#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elemdata.h"
#include "nodedata.h"

typedef struct ElemData342 {
  int id;
  int n[10];
} ElemData342;

enum { MAX_ELEM_INIT = 1024, MAX_ELEM_GROW = 2 };

static int n_elem = 0;

static ElemData342 *elem_data;
static int max_elem;

static char *elem_header;

void elem_init(char *header)
{
  elem_data = (ElemData342 *) malloc(MAX_ELEM_INIT * sizeof(ElemData342));
  if (elem_data == NULL) {
    perror("in elem_init()");
    exit(2);
  }
  max_elem = MAX_ELEM_INIT;

  if (header == NULL) {
    elem_header = NULL;
    return;
  }

  elem_header = (char *) malloc((strlen(header)+1) * sizeof(char));
  if (header == NULL) {
    perror("in elem_init()");
    exit(2);
  }
  strcpy(elem_header, header);
}

void elem_finalize(void)
{
  free(elem_data);
  free (elem_header);
}

void new_elem(int id, int *n)
{
  int i;

  if (n_elem == max_elem) {
    ElemData342 *edp;

    edp = (ElemData342 *) realloc(elem_data, MAX_ELEM_GROW * max_elem * sizeof(ElemData342));
    if (edp == NULL) {
      perror("in new_elem()");
      exit(2);
    }
    max_elem *= MAX_ELEM_GROW;
    elem_data = edp;
  }

  if (n_elem > 0 && elem_data[n_elem - 1].id >= id) {
    fprintf(stderr, "Error: elem id is not sorted\n");
    exit(1);
  }

  elem_data[n_elem].id = id;
  for (i = 0; i < 10; i++)
    elem_data[n_elem].n[i] = n[i];
  n_elem++;
}

int number_of_elems(void)
{
  return n_elem;
}

void print_elem(FILE *fp)
{
  int i, j;

  if (elem_header != NULL)
    fprintf(fp, "%s", elem_header);
  for (i = 0; i < n_elem; i++) {
    fprintf(fp, "%d", elem_data[i].id);
    for (j = 0; j < 10; j++)
      fprintf(fp, " %d", elem_data[i].n[j]);
    fprintf(fp, "\n");
  }
}

void print_elem_adv(FILE *fp)
{
  int i, j;
  static int f2a[10] = {3, 1, 0, 2, 8, 7, 9, 6, 5, 4};

  for (i = 0; i < n_elem; i++) {
    for (j = 0; j < 10; j++)
      fprintf(fp, " %d", get_local_node_id(elem_data[i].n[f2a[j]]));
    fprintf(fp, "\n");
  }
}
