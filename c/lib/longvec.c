#include "xalloc.h"
#include <stdio.h>
#include <stdlib.h>

#include "longvec.h"

LongVec *xmklongvec() {
  LongVec *v = xmalloc(sizeof(LongVec));
  v->cap = 1;
  v->len = 0;
  v->xs = xmalloc(sizeof(long));
  return v;
}

// Insert a value (x) into v. When an expansion is required, and fails to
// expand, abort.
void xinslongvec(LongVec *v, long x) {
  if (v->len == v->cap) {
    v->cap *= 2l;
    v->xs = xrealloc(v->xs, v->cap * sizeof(long));
  }
  v->xs[v->len++] = x;
}

// Remove the top value from v as long as it is not empty.
// Abort if attempted to remove a value from an empty vector.
// Do NOT automatically compact the storage unless made empty; use
// xcompactvec().
long xpoplongvec(LongVec *v) {
  if (v->len == 0) {
    fprintf(stderr,
            "xpoplongvec: Attempt to pop element from an empty vector.\n");
    abort();
  }
  long r = v->xs[--v->len];
  if (v->len == 0) {
    v->xs = xrealloc(v->xs, sizeof(long));
    v->cap = 1;
  }
  return r;
}

// Compacts the capacity of an int vector (v) or abort.
void xcompactlongvec(LongVec *v) {
  v->xs = xrealloc(v->xs, v->len * sizeof(long));
  v->cap = v->len;
}

// Frees an int vector
void freelongvec(LongVec *v) {
  free(v->xs);
  free(v);
}
