#ifndef ELEMDATA_H
#define ELEMDATA_H

#include <stdio.h>
#include "nodedata.h"

struct ElemDB;
typedef struct ElemDB ElemDB;

extern void elem_init(ElemDB **eldb_p, const char *header);
extern void elem_finalize(ElemDB *eldb);
extern void new_elem(ElemDB *eldb, int id, const int *n);
extern int number_of_elems(ElemDB *eldb);
extern void print_elem(ElemDB *eldb, FILE *fp);
extern void print_elem_adv(ElemDB *eldb, NodeDB *ndb, FILE *fp);

#endif /* ELEMDATA_H */
