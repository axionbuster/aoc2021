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

enum { SMALL = 1, BIG = 2, EXTREME = 255 };

typedef struct _grid_t {
  int *xs;
  size_t width, height;
} Grid;

static void init_grid(Grid *g, size_t width, size_t height) {
  // Account for the special value (final row/column)
  if (width >= EXTREME || height >= EXTREME) {
    fprintf(stderr, "invalid ranges\n");
    abort();
  }
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
  if (co.row == EXTREME) {
    co.row = g->height - 1;
  }
  if (co.column == EXTREME) {
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

typedef struct _memoi_t {
  size_t length;
  int *xs;
} Memoization;

static void init_memoi(Memoization *m, size_t length) {
  m->length = length;
  m->xs = xmalloc(m->length * sizeof(int));
  for (size_t i = 0; i < m->length; i++) {
    m->xs[i] = -1;
  }
}

static void delete_memoi(Memoization m) { free(m.xs); }

// Find all paths from start to -1 (end).
static int find_n_paths(Memoization *m, Grid *g, int start) {
  if (m->xs[start] >= 0) {
    return m->xs[start];
  }

  int n_local_paths = 0;

  for (int c = start; c < g->width; c++) {
    int x = *at(g, cursor(start, c));
    if (c == g->width - 1 && x) {
      n_local_paths += 1;
    } else if (x) {
      n_local_paths += find_n_paths(m, g, c);
    }
  }

  m->xs[start] = n_local_paths;
  return n_local_paths;
}

static void dbg_memoi(Memoization *m) {
  dbgprintf("Memoization dump\n");
  for (size_t i = 0; i < m->length; i++) {
    dbgprintf("%4zu -> ", i);
    if (m->xs[i] >= 0) {
      dbgprintf("%d\n", m->xs[i]);
    } else {
      dbgprintf("\"Not Searched\"\n");
    }
  }
  dbgprintf("\n");
}

int main(void) {
  // 0 start
  // 255 end
  // %d %d means: first integer -> second integer.

  int scan, t1, t2;
  Grid g = {0};
  init_grid(&g, 10, 10);
  Memoization m = {0};
  init_memoi(&m, 10);

  while ((scan = scanf("%d %d", &t1, &t2)) != EOF) {
    if (scan != 2)
      break;

    // Undirected graph
    Cursor co = cursor(t1, t2);
    Cursor co_alt = cursor(t2, t1);

    int *p = at(&g, co);
    if (p) {
      *p = SMALL;
      (*at(&g, co_alt)) = SMALL;
    } else {
      fprintf(stderr, "Invalid pointer\n");
      abort();
    }
  }

  dbg_grid(&g);

  dbgprintf("\n");

  int answer = find_n_paths(&m, &g, 0);

  printf("%d paths found.\n", answer);

  dbg_memoi(&m);

  delete_memoi(m);
  delete_grid(g);
  return 0;
}
