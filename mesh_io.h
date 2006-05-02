#ifndef _MESH_IO_H_
#define _MESH_IO_H_

enum header_mode {NONE, HEADER, NODE, ELEMENT, EGROUP, OTHER};

extern int is_comment(char *line);
extern int is_header(char *line, int *hmode);
extern int is_data(char *line);

#endif /* _MESH_IO_H_ */
