#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

extern void setprogname(const char *str);
extern const char *progname(void);
extern void print_log(FILE *fp, const char *log_mesg);

#endif /* UTIL_H */
