#ifndef CHARVEC_H
#define CHARVEC_H
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  char *xs;
  size_t len;
  size_t cap;
} CharVec;

// Allocate a charvec or abort.
CharVec *xmkcharvec();

// Migrate (move and own) a C-string. If cstr is allocated using malloc() or
// similar, it can be safely freed by a call to freecharvec(); used by
// xinscharvec() and xpopcharvec(). If, on the other hand, cstr is not managed
// by malloc() or similar, then, calls to xinscharvec(), xpopcharvec(), and
// freecharvec() can fail at reallocation points.
void migratecharvec(CharVec *cv, char *cstr);

// Returns true (_Bool) if the CharVec seems to terminate with the null
// character by (and including) len - 1.
bool hascstring(CharVec *cv);

// Insert a value (x) into v. When an expansion is required, and fails to
// expand, abort.
void xinscharvec(CharVec *v, char x);

// Remove the top value from v as long as it is not empty.
// Abort if attempted to remove a value from an empty vector.
// Do NOT automatically compact the storage unless made empty; use
// xcompactcharvec().
char xpopcharvec(CharVec *v);

// Compacts the capacity of a char vector (v) or abort.
void xcompactcharvec(CharVec *v);

// Frees an int vector
void freecharvec(CharVec *v);
#endif
