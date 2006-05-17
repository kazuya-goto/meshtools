#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util.h"

static char *name;

void setprogname(char *str)
{
  char *p;

  /* strcpy(progname, basename(str)); */
  if ((p = strrchr(str, '/')) == NULL &&
      (p = strrchr(str, '\\')) == NULL)
    p = str;
  else
    p++;

  name = (char *) malloc((strlen(p)+1) * sizeof(char));
  if (name == NULL) {
    perror("in setprogname()");
    exit(2);
  }
  strcpy(name, p);
}

char *progname(void)
{
  return name;
}

void print_log(FILE *fp, char *log_mesg)
{
  time_t t;
  char date_str[32];

  time(&t);
  strcpy(date_str, ctime(&t));
  *strchr(date_str, '\n') = '\0';
  fprintf(fp, "%s: %s\n", date_str, log_mesg);
  fflush(fp);
}

