#include "xalloc.h"
#include <stdio.h>
#include <stdlib.h>

void *xmalloc(size_t sz) {
  void *p = malloc(sz);
  if (!p) {
    fprintf(stderr, "abort: xmalloc\n");
    abort();
  }
  return p;
}

void *xrealloc(void *p, size_t nsz) {
  void *np = realloc(p, nsz);
  if (!np && nsz) {
    fprintf(stderr, "abort: xrealloc\n");
    abort();
  }
  return np;
}

void *xcalloc(size_t nelem, size_t szelem) {
  void *p = calloc(nelem, szelem);
  if (!p) {
    fprintf(stderr, "abort: xcalloc\n");
    abort();
  }
  return p;
}
