#ifndef INTVEC_H
#define INTVEC_H
#include <stddef.h>

typedef struct {
  int *xs;
  size_t len;
  size_t cap;
} IntVec;

// Allocate an intvec or abort.
IntVec *xmkintvec();

// Insert a value (x) into v. When an expansion is required, and fails to
// expand, abort.
void xinsintvec(IntVec *v, int x);

// Remove the top value from v as long as it is not empty.
// Abort if attempted to remove a value from an empty vector.
// Do NOT automatically compact the storage unless made empty; use
// xcompactvec().
int xpopintvec(IntVec *v);

// Compacts the capacity of an int vector (v) or abort.
void xcompactvec(IntVec *v);

// Frees an int vector
void freeintvec(IntVec *v);
#endif
