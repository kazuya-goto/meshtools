#ifndef PRECISION_H
#define PRECISION_H

#ifdef USE_INT64
typedef long long index_t;
#else
typedef int index_t;
#endif

#ifdef USE_DOUBLE
typedef double coord_t;
#else
typedef float coord_t;
#endif

#endif /* PRECISION_H */
