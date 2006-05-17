#ifndef _ELEMDATA_H_
#define _ELEMDATA_H_

extern void elem_init(char *);
extern void elem_finalize(void);
extern void new_elem(int id, int *n);
extern int number_of_elems(void);
extern void print_elem(FILE *fp);
extern void print_elem_adv(FILE *fp);

#endif /* _ELEMDATA_H_ */
