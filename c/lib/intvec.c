#include "xalloc.h"
#include <stdio.h>
#include <stdlib.h>

#include "intvec.h"

IntVec *xmkintvec() {
  IntVec *v = xmalloc(sizeof(IntVec));
  v->cap = 1;
  v->len = 0;
  v->xs = xmalloc(sizeof(int));
  return v;
}

// Insert a value (x) into v. When an expansion is required, and fails to
// expand, abort.
void xinsintvec(IntVec *v, int x) {
  if (v->len == v->cap) {
    v->cap *= 2;
    v->xs = xrealloc(v->xs, v->cap);
  }
  v->xs[v->len++] = x;
}

// Remove the top value from v as long as it is not empty.
// Abort if attempted to remove a value from an empty vector.
// Do NOT automatically compact the storage unless made empty; use
// xcompactvec().
int xpopintvec(IntVec *v) {
  if (v->len == 0) {
    fprintf(stderr,
            "xpopintvec: Attempt to pop element from an empty vector.\n");
    abort();
  }
  int r = v->xs[--v->len];
  if (v->len == 0) {
    v->xs = xrealloc(v->xs, sizeof(int));
    v->cap = 1;
  }
  return r;
}

// Compacts the capacity of an int vector (v) or abort.
void xcompactvec(IntVec *v) {
  v->xs = xrealloc(v->xs, v->len);
  v->cap = v->len;
}

// Frees an int vector
void freeintvec(IntVec *v) {
  free(v->xs);
  free(v);
}
