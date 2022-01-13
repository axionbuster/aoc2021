#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

#define TEXTBOLD "\033[1m"
#define TEXTNORMAL "\033[0m"

typedef struct grid_t {
  // Ordinary numbers (integers), not characters.
  signed char *grid;
  int width, height;
} Grid;

#define FORRC(r, rlim, c, clim)                                                \
  for (int r = 0; r < rlim; r++)                                               \
    for (int c = 0; c < clim; c++)

typedef struct cursor_t {
  int row, column;
} Cursor;

#define mkcur(r, c) ((Cursor){.row = (r), .column = (c)})

#define PRINTCUR "Cursor { .row = %d, .column = %d }"
#define EXPANDCUR(cur) cur.row, cur.column

static void initgrid(Grid *g) {
  g->grid = NULL;
  g->width = 0;
  g->height = 0;
}

// Create a zero-initialized grid of the same dimensions as g. Free this
// structure using delgrid() & free().
static Grid *mkscratcher(Grid *g) {
  Grid *ng = xmalloc(sizeof(Grid));
  ng->width = g->width;
  ng->height = g->height;
  size_t sz = (size_t)ng->width * (size_t)ng->height;
  ng->grid = xcalloc(sz, 1);
  return ng;
}

static void delgrid(Grid g) { free(g.grid); }

static signed char *at(Grid *g, Cursor c) {
  if (c.column < 0 || c.column >= g->width)
    return NULL;
  if (c.row < 0 || c.row >= g->height)
    return NULL;
  return &g->grid[c.column + c.row * g->width];
}

static void dbggrid(Grid *g) {
  signed char ch;
  bool bold;
  FORRC(r, g->height, c, g->width) {
    Cursor co = mkcur(r, c);
    ch = *at(g, co);
    if (!c) {
      if (bold) {
        dbgprintf(TEXTNORMAL);
      }
      bold = false;
      dbgprintf("\n");
    }
    if (ch && bold) {
      bold = false;
      dbgprintf(TEXTNORMAL);
    }
    if (!ch && !bold) {
      bold = true;
      dbgprintf(TEXTBOLD);
    }

    dbgprintf("%1x", ch);
  }

  if (bold) {
    dbgprintf(TEXTNORMAL);
  }
  dbgprintf("\n");
}

static void cpyrawrow(Grid *g, char const *row) {
  size_t ll = g->width ? g->width : strlen(row);
  size_t sz = (size_t)g->width * (size_t)g->height;
  size_t nsz = sz + ll;
  g->grid = xrealloc(g->grid, nsz);
  g->width = g->width ? g->width : ll;
  g->height++;

  for (int i = 0; i < g->width; i++) {
    g->grid[sz + i] = (int)row[i] - '0';
    assert(g->grid[sz + i] >= 0);
    assert(g->grid[sz + i] <= 9);
  }
}

static void incall(Grid *g) {
  FORRC(r, g->height, c, g->width) {
    Cursor co = mkcur(r, c);
    (*at(g, co)) += 1;
  }
}

static void truncall(Grid *g) {
  FORRC(r, g->height, c, g->width) {
    Cursor co = mkcur(r, c);
    signed char *x = at(g, co);
    *x = *x > 9 ? 0 : *x;
  }
}

// For any element who is above 9, increment all adjacent elements by 1.
// Return number of elements that have flashed.
// Elements are not truncated.
static int flashall(Grid *g) {
  Grid *scratch = mkscratcher(g);

  // Count the number of flashes occured in the loop body.
  int chain = 0;

  // Mark all increases
  FORRC(r, g->height, c, g->width) {
    Cursor main_co = mkcur(r, c);
    signed char main_elem = *at(g, main_co);
    if (main_elem <= 9)
      continue;
    // Flashing criterion met.
    chain += 1;
    // Go through every one of the 8 adjacent elements, mark as "will
    // increase its value by 1 (as a result of the main [center] element
    // flashing)."
    FORRC(rr, 3, cc, 3) {
      int roff = rr - 1;
      int coff = cc - 1;
      if (roff == 0 && coff == 0) {
        continue;
      }
      Cursor adj_co = mkcur(r + roff, c + coff);
      signed char *adj_elem = at(g, adj_co);
      // Check bounds
      if (adj_elem) {
        signed char *at_scratch = at(scratch, adj_co);
        *at_scratch += 1;
      }
    }
  }

  // Perform all increases and clear scratch.
  FORRC(r, g->height, c, g->width) {
    Cursor co = mkcur(r, c);
    *at(g, co) += *at(scratch, co);
    *at(scratch, co) = 0;
  }

  dbgprintf("Main:");
  dbggrid(g);
  dbgprintf("Chain reactions? %s\n", chain ? "yes" : "no");

  delgrid(*scratch);
  free(scratch);

  return chain;
}

int main(void) {
  Grid g;
  initgrid(&g);

  char *line;
  for (int scan = 0; (scan = scanf("%ms ", &line)) != EOF; free(line)) {
    cpyrawrow(&g, line);
  }

  dbgprintf("Before any steps:\n");
  dbggrid(&g);
  dbgprintf("\n");

  for (int step = 1; step <= 100; step++) {
    dbgprintf("Step %d:\n\n", step);
    int chain = 0;
    do {
      incall(&g);
      chain = flashall(&g);
      truncall(&g);
      dbgprintf("\n");
    } while (chain);
  }

  delgrid(g);
  return 0;
}
