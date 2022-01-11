#include "xalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charvec.h"

CharVec *xmkcharvec() {
  CharVec *v = xmalloc(sizeof(CharVec));
  v->cap = 1;
  v->len = 0;
  v->xs = xmalloc(sizeof(char));
  return v;
}

void migratecharvec(CharVec *cv, char *cstr) {
  free(cv->xs);
  cv->cap = strlen(cstr);
  cv->len = cv->cap;
  cv->xs = cstr;
}

bool hascstring(CharVec *cv) { return strnlen(cv->xs, cv->len) < cv->len; }

void xinscharvec(CharVec *v, char x) {
  if (v->len == v->cap) {
    v->cap *= 2;
    v->xs = xrealloc(v->xs, v->cap);
  }
  v->xs[v->len++] = x;
}

char xpopcharvec(CharVec *v) {
  if (v->len == 0) {
    fprintf(stderr,
            "xpopcharvec: Attempt to pop element from an empty vector.\n");
    abort();
  }
  int r = v->xs[--v->len];
  if (v->len == 0) {
    v->xs = xrealloc(v->xs, sizeof(char));
    v->cap = 1;
  }
  return r;
}

void xcompactcharvec(CharVec *v) {
  v->xs = xrealloc(v->xs, v->len);
  v->cap = v->len;
}

void freecharvec(CharVec *v) {
  free(v->xs);
  free(v);
}
