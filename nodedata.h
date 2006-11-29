#ifndef NODEDATA_H
#define NODEDATA_H

#include <stdio.h>

extern void node_init(void);
extern void node_finalize(void);
extern void new_node(int id, double x, double y, double z);
extern void reduce_node_data(void);
extern double node_dist2(int i1, int i2);
extern int number_of_nodes(void);
extern int get_local_node_id(int i1);
extern int get_global_node_id(int li1);
extern int number_of_middle_nodes(void);
extern int new_middle_node(int i1, int i2);
extern void print_last_middle_node(FILE *fp);
extern void print_node_adv(FILE *fp);

#endif /* NODEDATA_H */
