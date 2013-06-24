/*
 * elemdata.c
 *
 * Author: Kazuya Goto <goto@multi.k.u-tokyo.ac.jp>
 * Created on May 02, 2006
 * Last modified: Jun 24, 2013
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elemdata.h"
#include "nodedata.h"
#include "util.h"

/* data set for a single element. */
struct ElemData {
  index_t id;
  index_t n[10];
};

typedef struct ElemData ElemData;

struct ElemDB {
  index_t n_elem;
  int npe; /* number of nodes per element */
  ElemData *elem_data;
  index_t max_elem;
  char *elem_header;
};

enum { MAX_ELEM_INIT = 1024, MAX_ELEM_GROW = 2 };


/* initialize elem_data */
void elem_init(ElemDB **eldb_p, const char *header)
{
  ElemDB *eldb;

  *eldb_p = (ElemDB *) emalloc(sizeof(ElemDB));

  eldb = *eldb_p;

  eldb->n_elem = 0;
  eldb->elem_data = (ElemData *) emalloc(MAX_ELEM_INIT * sizeof(ElemData));
  eldb->max_elem = MAX_ELEM_INIT;

  if (header == NULL) {
    eldb->elem_header = NULL;
    return;
  }

  eldb->elem_header = estrdup(header);

  if (strstr(eldb->elem_header, "342") != NULL)
    eldb->npe = 10;
  else if (strstr(eldb->elem_header, "341") != NULL)
    eldb->npe = 4;
  else {
    fprintf(stderr, "Error: element type not supported\n");
    exit(1);
  }
}

/* finalize elem_data */
void elem_finalize(ElemDB *eldb)
{
  free(eldb->elem_data);
  free(eldb->elem_header);

  free(eldb);
}

/* register a new element in elem_data */
void new_elem(ElemDB *eldb, index_t id, const index_t *n)
{
  int i;

  if (eldb->n_elem == eldb->max_elem) {
    eldb->max_elem *= MAX_ELEM_GROW;
    eldb->elem_data = 
      (ElemData *) erealloc(eldb->elem_data,
			    eldb->max_elem * sizeof(ElemData));
  }

  if (eldb->n_elem > 0 && eldb->elem_data[eldb->n_elem - 1].id >= id) {
    fprintf(stderr, "Error: elem id is not sorted\n");
    exit(1);
  }

  eldb->elem_data[eldb->n_elem].id = id;
  for (i = 0; i < eldb->npe; i++)
    eldb->elem_data[eldb->n_elem].n[i] = n[i];
  eldb->n_elem++;
}

/* return the number of elements */
index_t number_of_elems(ElemDB *eldb)
{
  return eldb->n_elem;
}

/* print element data */
void print_elem(ElemDB *eldb, FILE *fp)
{
  index_t i;
  int j;

  if (eldb->elem_header != NULL)
    fprintf(fp, "%s", eldb->elem_header);
  for (i = 0; i < eldb->n_elem; i++) {
    fprintf(fp, "%lld", (long long) eldb->elem_data[i].id);
    for (j = 0; j < eldb->npe; j++)
      fprintf(fp, ",%lld", (long long) eldb->elem_data[i].n[j]);
    fprintf(fp, "\n");
  }
}

/* print element data in Adventure .msh format */
void print_elem_adv(ElemDB *eldb, NodeDB *ndb, FILE *fp)
{
  index_t i;
  int j;
  static int f2a[10] = {3, 1, 0, 2, 8, 7, 9, 6, 5, 4};

  for (i = 0; i < eldb->n_elem; i++) {
    for (j = 0; j < eldb->npe; j++)
      fprintf(fp, " %lld", (long long) get_local_node_id(ndb, eldb->elem_data[i].n[f2a[j]]));
    fprintf(fp, "\n");
  }
}
