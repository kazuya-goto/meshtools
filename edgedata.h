#ifndef _EDGEDATA_H_
#define _EDGEDATA_H_

extern void edge_init(void);
extern void edge_finalize(void);
extern int middle_node(int i1, int i2, int *mnidp);
extern void print_edge_stat(FILE *log_file);

#endif /* _EDGEDATA_H_ */
