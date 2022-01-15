// allpaths.c -- YuJim Kim (C) 2022 ARR
//
// Using integer path representations, find all paths
// in the graph

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

typedef struct _grid_t {
  int *xs;
  size_t width, height;
} Grid;

static void init_grid(Grid *g, size_t width, size_t height) {
  // Account for the special value (final row/column)
  size_t size = (width + 1) * (height + 1);
  g->xs = xcalloc(size, sizeof(int));
  g->width = width + 1;
  g->height = height + 1;
}

static void delete_grid(Grid g) { free(g.xs); }

typedef struct _cursor_t {
  int row, column;
} Cursor;

#define cursor(r, c) ((Cursor){.row = (r), .column = (c)})

// Extreme values, namely exactly -1, are substituded with the last reachable
// indices.
static int *at(Grid *g, Cursor co) {
  if (co.row == -1) {
    co.row = g->height - 1;
  }
  if (co.column == -1) {
    co.column = g->width - 1;
  }
  bool exit = false;
  if (co.row < 0)
    exit = true;
  if (co.row >= g->height)
    exit = true;
  if (co.column < 0)
    exit = true;
  if (co.column >= g->width)
    exit = true;
  if (exit)
    return NULL;
  else
    return &g->xs[co.column + co.row * g->width];
}

static void dbg_grid(Grid *g) {
  dbgprintf("Symmetric --- omit when row < column\n");
  dbgprintf("%4s ", "");
  for (int i = 0; i < g->width; i++) {
    if (i == g->width - 1)
      dbgprintf("%4s\n", "X");
    else
      dbgprintf("%4d ", i);
  }
  for (int i = 0; i < g->height; i++) {
    if (i == g->height - 1)
      dbgprintf("%4s ", "Y");
    else
      dbgprintf("%4d ", i);
    for (int j = 0; j < g->width; j++) {
      int v = *at(g, cursor(i, j));
      if (j >= i && v)
        dbgprintf("%4d ", v);
      else
        dbgprintf("%4s ", "");
    }
    dbgprintf("\n");
  }
  dbgprintf("\n");
}

int main(void) {
  // 0 start
  // -1 end
  // %d %d means: first integer -> second integer.

  int scan, t1, t2;
  Grid g = {0};
  init_grid(&g, 10, 10);
  while ((scan = scanf("%d %d", &t1, &t2)) != EOF) {
    if (scan != 2)
      break;

    // Undirected graph
    Cursor co = cursor(t1, t2);
    Cursor co_alt = cursor(t2, t1);

    int *p = at(&g, co);
    if (p) {
      *p = 1;
      (*at(&g, co_alt)) = 1;
    } else {
      fprintf(stderr, "Invalid pointer\n");
      return 1;
    }
  }

  dbg_grid(&g);

  delete_grid(g);
  return 0;
}
