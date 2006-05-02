#include <stdio.h>
#include <stdlib.h>
#include "elemdata.h"
#include "nodedata.h"

typedef struct _ElemData342 {
  int id;
  int n1;
  int n2;
  int n3;
  int n4;
  int n5;
  int n6;
  int n7;
  int n8;
  int n9;
  int n10;
} ElemData342;

static int n_elem = 0;

static ElemData342 *elem_data;
static int max_elem = 1024;

void elem_init(void)
{
  elem_data = (ElemData342 *) malloc(max_elem * sizeof(ElemData342));
  if (elem_data == NULL) {
    perror("elem_init");
    exit(1);
  }
}

void elem_finalize(void)
{
  free(elem_data);
}

void new_elem(int id,
	      int n1, int n2, int n3, int n4, int n5,
	      int n6, int n7, int n8, int n9, int n10)
{
  if (n_elem == max_elem) {
    max_elem *= 2;
    elem_data = (ElemData342 *) realloc(elem_data, max_elem * sizeof(ElemData342));
    if (elem_data == NULL) {
      perror("new_elem");
      exit(1);
    }
  }

  if (n_elem > 0 && elem_data[n_elem - 1].id >= id) {
    fprintf(stderr, "Error: elem id is not sorted\n");
    exit(1);
  }

  elem_data[n_elem].id = id;
  elem_data[n_elem].n1 = n1;
  elem_data[n_elem].n2 = n2;
  elem_data[n_elem].n3 = n3;
  elem_data[n_elem].n4 = n4;
  elem_data[n_elem].n5 = n5;
  elem_data[n_elem].n6 = n6;
  elem_data[n_elem].n7 = n7;
  elem_data[n_elem].n8 = n8;
  elem_data[n_elem].n9 = n9;
  elem_data[n_elem].n10 = n10;
  n_elem++;
}

void print_elem_renum(FILE *fp)
{
  int i;
  for (i = 0; i < n_elem; i++) {
    fprintf(fp, " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
	    elem_data[i].id,                   /* = i ?? */
	    get_new_node_id(elem_data[i].n1),
	    get_new_node_id(elem_data[i].n2),
	    get_new_node_id(elem_data[i].n3),
	    get_new_node_id(elem_data[i].n4),
	    get_new_node_id(elem_data[i].n5),
	    get_new_node_id(elem_data[i].n6),
	    get_new_node_id(elem_data[i].n7),
	    get_new_node_id(elem_data[i].n8),
	    get_new_node_id(elem_data[i].n9),
	    get_new_node_id(elem_data[i].n10));
  }
}

int number_of_elems(void)
{
  return n_elem;
}

void print_elem_adv(FILE *fp)
{
  int i;
  for (i = 0; i < n_elem; i++) {
    fprintf(fp, "%d %d %d %d %d %d %d %d %d %d\n",
	    get_local_node_id(elem_data[i].n4),
	    get_local_node_id(elem_data[i].n2),
	    get_local_node_id(elem_data[i].n1),
	    get_local_node_id(elem_data[i].n3),
	    get_local_node_id(elem_data[i].n9),
	    get_local_node_id(elem_data[i].n8),
	    get_local_node_id(elem_data[i].n10),
	    get_local_node_id(elem_data[i].n7),
	    get_local_node_id(elem_data[i].n6),
	    get_local_node_id(elem_data[i].n5));
  }
}
