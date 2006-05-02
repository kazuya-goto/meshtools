#include <stdio.h>
#include "mesh_io.h"



int is_comment(char *line)
{
  return (line[0] == '#' || (line[0] == '!' && line[1] == '!'));
}

int is_header(char *line, int *hmode)
{
  enum header_mode hm;

  if (line[0] == '!' && line[1] != '!') {
    char *header = line+1;
    while (header[0] == ' ') header++;

    if (strncmp(header, "NODE", 4) == 0)
      hm = NODE;
    else if (strncmp(header, "ELEMENT", 7) == 0)
      hm = ELEMENT;
    else if (strncmp(header, "EGROUP", 6) == 0)
      hm = EGROUP;
    else
      hm = OTHER;

    if (hm != *hmode) {
      *hmode = hm;
      return 1;
    }
  }
  return 0;
}

int is_data(char *line)
{
  return (line[0] != '#' && line[0] != '!');
}
