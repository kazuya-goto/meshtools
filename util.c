/*
 * util.c
 *
 * Author: Kazuya Goto <goto@multi.k.u-tokyo.ac.jp>
 * Created on May 17, 2006
 * Last modified: Jun 24, 2013
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util.h"

void *emalloc(size_t size)
{
  void *ptr;
  ptr = malloc(size);
  if (ptr == NULL) {
    perror("malloc");
    fprintf(stderr, "malloc of %lu bytes failed\n", size);
    exit(2);
  }
  return ptr;
}

void *erealloc(void *ptr, size_t size)
{
  void *rptr;
  rptr = realloc(ptr, size);
  if (rptr == NULL) {
    perror("realloc");
    fprintf(stderr, "realloc of %lu bytes failed\n", size);
    exit(2);
  }
  return rptr;
}

char *estrdup(const char *source)
{
  char *dest;
  dest = (char *) emalloc((strlen(source)+1) * sizeof(char));
  return strcpy(dest, source);
}

FILE *efopen(const char *path, const char *mode)
{
  FILE *fp;
  fp = fopen(path, mode);
  if (fp == NULL) {
    perror("fopen");
    exit(2);
  }
  return fp;
}

FILE *etmpfile(void)
{
  /* char tmpname[64]; */
  FILE *tmp_file;

  /*
  strcpy(tmpname, progname());
  strcat(tmpname, ".tmp");
  tmp_file = efopen(tmpname, "w+");
  */
  tmp_file = tmpfile();
  if (tmp_file == NULL) {
    perror("tmpfile");
    exit(2);
  }
  return tmp_file;
}

enum { BUFSIZE = 4096 };

void file_copy(FILE *from_file, FILE *to_file)
{
  size_t rbytes;
  char tmpbuf[BUFSIZE];
  while ((rbytes = fread(tmpbuf, 1, sizeof(tmpbuf), from_file)) > 0)
    fwrite(tmpbuf, 1, rbytes, to_file);
}

static char *name;

void setprogname(const char *str)
{
  const char *p;

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

const char *progname(void)
{
  return name;
}

void print_log(FILE *fp, const char *log_mesg)
{
  time_t t;
  char date_str[32];

  time(&t);
  strcpy(date_str, ctime(&t));
  *strchr(date_str, '\n') = '\0';
  fprintf(fp, "%s: %s\n", date_str, log_mesg);
  fflush(fp);
}
