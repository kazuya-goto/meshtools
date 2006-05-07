#include <stdio.h>
#include <stdlib.h>
#include "elemdata.h"
#include "nodedata.h"

typedef struct _ElemData342 {
  int id;
  int n[10];
} ElemData342;

enum { MAX_ELEM_INIT = 1024, MAX_ELEM_GROW = 2 };

static int n_elem = 0;

static ElemData342 *elem_data;
static int max_elem;

void elem_init(void)
{
  elem_data = (ElemData342 *) malloc(max_elem * sizeof(ElemData342));
  if (elem_data == NULL) {
    perror("elem_init");
    exit(1);
  }
  max_elem = MAX_ELEM_INIT;
}

void elem_finalize(void)
{
  free(elem_data);
}

void new_elem(int id, int n[])
{
  int i;

  if (n_elem == max_elem) {
    ElemData342 *edp;

    edp = (ElemData342 *) realloc(elem_data, MAX_ELEM_GROW * max_elem * sizeof(ElemData342));
    if (edp == NULL) {
      perror("new_elem");
      exit(1);
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

void print_elem_renum(FILE *fp)
{
  int i, j;
  for (i = 0; i < n_elem; i++) {
    fprintf(fp, " %d", elem_data[i].id);   /* = i ?? */
    for (j = 0; j < 10; j++)
      fprintf(fp, ", %d", get_new_node_id(elem_data[i].n[i]));
    fprintf(fp, "\n");
  }
}

int number_of_elems(void)
{
  return n_elem;
}

void print_elem_adv(FILE *fp)
{
  int i, j;
  /* static int f2a[10] = {4, 2, 1, 3, 9, 8, 10, 7, 6, 5}; */
  static int f2a[10] = {3, 1, 0, 2, 8, 7, 9, 6, 5, 4};

  for (i = 0; i < n_elem; i++) {
    for (j = 0; j < 10; j++)
      fprintf(fp, " %d", get_local_node_id(elem_data[i].n[f2a[j]]));
    fprintf(fp, "\n");
  }
}
