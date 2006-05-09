#ifndef _NODEDATA_H_
#define _NODEDATA_H_

extern void node_init(void);
extern void node_finalize(void);
extern void new_node(int id, double x, double y, double z);
extern double node_dist(int i1, int i2);
extern int number_of_nodes(void);
extern int number_of_middle_nodes(void);
extern int get_local_node_id(int i1);
extern int get_global_node_id(int li1);
extern int new_middle_node(int i1, int i2);
extern char *last_node_data_line(char *line, int maxlen);
extern void print_node_adv(FILE *fp);

#endif /* _NODEDATA_H_ */
