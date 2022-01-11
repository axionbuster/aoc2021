#ifndef LONGVEC_H
#define LONGVEC_H
#include <stddef.h>

typedef struct {
  long *xs;
  size_t len;
  size_t cap;
} LongVec;

// Allocate a longvec or abort.
LongVec *xmklongvec();

// Insert a value (x) longo v. When an expansion is required, and fails to
// expand, abort.
void xinslongvec(LongVec *v, long x);

// Remove the top value from v as long as it is not empty.
// Abort if attempted to remove a value from an empty vector.
// Do NOT automatically compact the storage unless made empty; use
// xcompactlongvec().
long xpoplongvec(LongVec *v);

// Compacts the capacity of a long vector (v) or abort.
void xcompactlongvec(LongVec *v);

// Frees a long vector
void freelongvec(LongVec *v);
#endif
