#ifndef _ELEMDATA_H_
#define _ELEMDATA_H_

extern void elem_init(void);
extern void elem_finalize(void);
extern void new_elem(int id,
		     int n1, int n2, int n3, int n4, int n5,
		     int n6, int n7, int n8, int n9, int n10);
extern void print_elem_renum(FILE *fp);
extern int number_of_elems(void);
extern void print_elem_adv(FILE *fp);

#endif /* _ELEMDATA_H_ */
