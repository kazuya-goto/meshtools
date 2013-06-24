#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

extern void *emalloc(size_t size);
extern void *erealloc(void *ptr, size_t size);
extern char *estrdup(const char *source);

extern FILE *efopen(const char *path, const char *mode);
extern FILE *etmpfile(void);
extern void file_copy(FILE *from_file, FILE *to_file);

extern void setprogname(const char *str);
extern const char *progname(void);
extern void print_log(FILE *fp, const char *format, ...);

extern double get_cputime(void);
extern double get_wtime(void);

#endif /* UTIL_H */
