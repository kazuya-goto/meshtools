#ifndef PRECISION_H
#define PRECISION_H

#ifdef USE_INT64
typedef long long index_t;
#else
typedef int index_t;
#endif

typedef float coord_t;

#endif /* PRECISION_H */
