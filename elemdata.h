#ifndef ELEMDATA_H
#define ELEMDATA_H

#include <stdio.h>
#include "precision.h"
#include "nodedata.h"

struct ElemDB;
typedef struct ElemDB ElemDB;

extern void elem_init(ElemDB **eldb_p, const char *header);
extern void elem_finalize(ElemDB *eldb);
extern void new_elem(ElemDB *eldb, index_t id, const index_t *n);
extern index_t number_of_elems(ElemDB *eldb);
extern void print_elem(ElemDB *eldb, FILE *fp);
extern void print_elem_adv(ElemDB *eldb, NodeDB *ndb, FILE *fp);

#endif /* ELEMDATA_H */
