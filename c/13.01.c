#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/iminmax.h"
#include "lib/xalloc.h"

typedef enum {
  FOLD_LEFT,
  FOLD_UP,
} Fold;

typedef struct {
  char *xs;
  long width, height;
} Grid;

#define FORRC(r, r_max, c, c_max)                                              \
  for (long r = 0; r < r_max; r++)                                             \
    for (long c = 0; c < c_max; c++)

#define at(g, c, r) ((g).xs[((c) + (r) * ((g).width))])

static void dbg_grid(Grid *g) {
  long counter = 0;
  FORRC(r, g->height, c, g->width) {
    if (counter == (g->width - 1)) {
      dbgprintf("%c\n", at(*g, c, r));
      counter = 0;
    } else {
      dbgprintf("%c", at(*g, c, r));
      counter++;
    }
  }
  dbgprintf("\n");
}

static void fold_up(Grid *g, long line) {
  assert(line > 0);
  assert(line < g->height);
  for (long mainline = line + 1, counterline = line - 1; mainline < g->height;
       mainline++, counterline--) {
    assert(counterline >= 0);
    for (long c = 0; c < g->width; c++) {
      char m = at(*g, c, mainline);
      if (m == '#') {
        at(*g, c, counterline) = m;
      }
    }
  }
  g->height = line;
}

static void fold_left(Grid *g, int line) {
  // First, you need to make character-by-character overwrites for '#'.
  // Second, you need to resize the array horizontally, which is done by copying
  // slices around.
  assert(line > 0);
  assert(line < g->width);
  for (long mainline = line + 1, counterline = line - 1; mainline < g->width;
       mainline++, counterline--) {
    assert(counterline >= 0);
    // Make character-by-character overwrites.
    for (long r = 0; r < g->height; r++) {
      char m = at(*g, mainline, r);
      if (m == '#') {
        at(*g, counterline, r) = m;
      }
    }
  }
  // For the grid layout it's kinda inflexible with the width.
  // When you resize the width you also need to shuffle the data around.
  // This is done by stitching together the isolated streaks of valid memory
  // into a contiguous array. The old memory region and the new region
  // may very well overlap in each step of the copying.
  long old_row_size = g->width;
  long new_row_size = g->width - line - 1;
  for (long r = 1; r < g->height; r++) {
    memmove(&g->xs[r * new_row_size], &g->xs[r * old_row_size], new_row_size);
  }
  g->width = new_row_size;
}

static long visible_dots(Grid *g) {
  long count = 0;
  FORRC(r, g->height, c, g->width) {
    dbgprintf("visible_dots: access (r, c) = (%ld, %ld) offset=%ld\n", r, c,
              &at(*g, c, r) - g->xs);
    if (at(*g, c, r) == '#')
      count++;
  }
  return count;
}

typedef struct {
  long column, row;
} Cursor;

typedef struct {
  Cursor *xs;
  long len, cap, max_column, max_row;
} Vector;

int main(void) {
  Grid g = {0};
  Vector v = {0};
  v.cap = 1;
  v.xs = xcalloc(1, sizeof(Cursor));

  // Read the lines, find out maximum column and row.
  int scan;
  long c, r;
  for (scan = 0; (scan = scanf("%ld,%ld ", &c, &r)) == 2;) {
    // Insert coords into v while expanding size as required.
    // Update max column and row statistics.
    if (v.len == v.cap) {
      v.cap *= 2;
      v.xs = xrealloc(v.xs, v.cap * sizeof(Cursor));
    }
    v.xs[v.len].column = c;
    v.xs[v.len].row = r;
    v.len++;
    v.max_column = tg_max(v.max_column, c);
    v.max_row = tg_max(v.max_row, r);
  }
  v.cap = v.len;
  v.xs = xrealloc(v.xs, v.len * sizeof(Cursor));

  // Allocate the grid and fill it.
  g.width = v.max_column + 1;
  g.height = v.max_row + 1;
  g.xs = xmalloc((size_t)g.width * (size_t)g.height);
  FORRC(c, g.width, r, g.height) { at(g, c, r) = '.'; }
  for (int i = 0; i < v.len; i++) {
    Cursor co = v.xs[i];
    dbgprintf("(c, r) = (%ld, %ld) offset=%ld\n", co.column, co.row,
              &at(g, co.column, co.row) - g.xs);
    at(g, co.column, co.row) = '#';
  }

  // Display the grid (debug)
  dbg_grid(&g);

  // Read the first fold.
  Fold fold;
  char fold_c;
  long along;

  dbgprintf("scanning for fold along\n");
  dbgflush(stdout);
  dbgflush(stderr);
  scan = scanf("fold along %c=%ld ", &fold_c, &along);
  if (scan == 2) {
    switch (fold_c) {
    case 'y':
      fold = FOLD_UP;
      break;
    case 'x':
      fold = FOLD_LEFT;
      break;
    default:
      fprintf(stderr,
              "fold along direction is wrong: %c must be either 'x' or 'y'.\n",
              fold_c);
      free(g.xs);
      free(v.xs);
      return 1;
    }
  } else {
    fprintf(stderr, "Abort while reading fold information.\n");
    free(g.xs);
    free(v.xs);
    return 1;
  }

  printf("%ld points read. Max column = %ld, max row = %ld\n", v.len,
         v.max_column, v.max_row);
  printf("The fold: Fold %s, along %c=%ld\n", fold == FOLD_LEFT ? "LEFT" : "UP",
         fold_c, along);

  // Vector goes out of scope here.
  free(v.xs);
  v = (Vector){0};

  if (fold == FOLD_UP) {
    dbgprintf("FOLD_UP initiated\n");
    fold_up(&g, along);
    dbg_grid(&g);
  } else {
    dbgprintf("FOLD_LEFT initiated\n");
    fold_left(&g, along);
    dbg_grid(&g);
  }

  printf("After the first fold, %ld dots are visible.\n", visible_dots(&g));

  free(g.xs);
  return 0;
}