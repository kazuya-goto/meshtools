#ifndef ELEMDATA_H
#define ELEMDATA_H

#include <stdio.h>
#include "nodedata.h"

/* data set for a single element. */
typedef struct ElemData {
  int id;
  int n[10];
} ElemData;

typedef struct ElemDB {
  int n_elem;
  int npe; /* number of nodes per element */
  ElemData *elem_data;
  int max_elem;
  char *elem_header;
} ElemDB;

extern void elem_init(ElemDB *eldb, const char *header);
extern void elem_finalize(ElemDB *eldb);
extern void new_elem(ElemDB *eldb, int id, const int *n);
extern int number_of_elems(ElemDB *eldb);
extern void print_elem(ElemDB *eldb, FILE *fp);
extern void print_elem_adv(ElemDB *eldb, NodeDB *ndb, FILE *fp);

#endif /* ELEMDATA_H */
