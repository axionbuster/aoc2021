#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/iminmax.h"
#include "lib/intvec.h"
#include "lib/xalloc.h"

typedef struct cursor_t {
  int row;
  int col;
} Cursor;

static bool local_low(int ncols, int col, char *rows[3]) {
  int up = rows[0] ? rows[0][col] - '0' : INT_MAX;
  int down = rows[2] ? rows[2][col] - '0' : INT_MAX;
  int left = (col > 0) ? rows[1][col - 1] - '0' : INT_MAX;
  int right = (col < ncols - 1) ? rows[1][col + 1] - '0' : INT_MAX;
  int on = rows[1][col] - '0';
  if (on < up && on < down && on < left && on < right) {
    dbgprintf("v\tup = %d, down = %d, left = %d, right = %d\n", up, down, left,
              right);
  }
  return on < up && on < down && on < left && on < right;
}

typedef struct strgrid_t {
  char **grid;
  int nrows;
  int capacity;
} StringGrid;

static StringGrid mkstrgrid() {
  StringGrid sg = {
      .grid = xcalloc(1, sizeof(char *)), .nrows = 0, .capacity = 1};
  return sg;
}

static void delstrgrid(StringGrid sg) {
  for (int r = 0; r < sg.nrows; r++) {
    free(sg.grid[r]);
  }
  free(sg.grid);
}

// Move a preallocated C-string into the string grid.
static void movstrgrid(StringGrid *sg, char *line) {
  if (sg->nrows == sg->capacity) {
    sg->capacity *= 2;
    sg->grid = xrealloc(sg->grid, sg->capacity * sizeof(char *));
  }
  sg->grid[sg->nrows++] = line;
}

typedef struct strgrid_aux_t {
  int row_length;
  StringGrid *sg;
} StringGridAux;

// Find the integer representation of the character (eg. '1' -> 1).
// If nonexistent, return INT_MAX.
static int i_sgat(StringGridAux sga, Cursor co) {
  if (co.row < 0 || co.row >= sga.sg->nrows) {
    return INT_MAX;
  }

  if (co.col < 0 || co.col >= sga.row_length) {
    return INT_MAX;
  }

  return sga.sg->grid[co.row][co.col] - '0';
}

typedef struct vcursor_t {
  Cursor *cos;
  int len;
  int cap;
} VectorCursor;

static VectorCursor mkvcursor() {
  VectorCursor vc = {.len = 0, .cap = 1, .cos = xcalloc(1, sizeof(Cursor))};
  return vc;
}

static void delvcursor(VectorCursor vc) { free(vc.cos); }

static void insvcursor(VectorCursor *vc, Cursor co) {
  if (vc->len == vc->cap) {
    vc->cap *= 2;
    vc->cos = xrealloc(vc->cos, vc->cap * sizeof(Cursor));
  }
  vc->cos[vc->len++] = co;
}

// Linearly search from the beginning the earliest index at which a value
// identical to (co) occurs. If not found or on empty storage, return -1.
static int srcvcursor(VectorCursor *vc, Cursor co) {
  for (int i = 0; i < vc->len; i++) {
    Cursor rco = vc->cos[i];
    if (rco.row == co.row && rco.col == co.col) {
      return i;
    }
  }
  return -1;
}

typedef struct indexgrid_t {
  int *indices;
  int width;
  int height;
} IndexGrid;

static IndexGrid mkigrid(int width, int height) {
  IndexGrid lg = {.width = width,
                  .height = height,
                  .indices = xmalloc(width * height * sizeof(int))};
  for (int r = 0; r < lg.height; r++) {
    for (int c = 0; c < lg.width; c++) {
      lg.indices[r * lg.width + c] = -1;
    }
  }
  return lg;
}

static void deligrid(IndexGrid lg) { free(lg.indices); }

static int igat(IndexGrid *lg, Cursor co) {
  if (co.row < 0 || co.row >= lg->height)
    return -1;
  if (co.col < 0 || co.col >= lg->width)
    return -1;
  return lg->indices[co.row * lg->width + co.col];
}

static int greedy_find_basin(StringGrid *sg, IndexGrid *lg,
                             VectorCursor *low_points, Cursor co) {
  if (igat(lg, co) != -1) {
    return igat(lg, co);
  }

  StringGridAux sga = {.row_length = lg->width, .sg = sg};

  int on = i_sgat(sga, co);
  if (on == -1) {
    return -1;
  }
  if (on == 9) {
    return -1;
  }
  if (on == 0) {
    return srcvcursor(low_points, co);
  }

  Cursor cup = {.row = co.row - 1, .col = co.col};
  Cursor cdown = {.row = co.row + 1, .col = co.col};
  Cursor cleft = {.row = co.row, .col = co.col - 1};
  Cursor cright = {.row = co.row, .col = co.col + 1};

  int up = i_sgat(sga, cup);
  int down = i_sgat(sga, cdown);
  int left = i_sgat(sga, cleft);
  int right = i_sgat(sga, cright);

  if (up < on) {
    return greedy_find_basin(sg, lg, low_points, cup);
  }

  if (down < on) {
    return greedy_find_basin(sg, lg, low_points, cdown);
  }

  if (left < on) {
    return greedy_find_basin(sg, lg, low_points, cleft);
  }

  if (right < on) {
    return greedy_find_basin(sg, lg, low_points, cright);
  }

  return srcvcursor(low_points, co);
}

static int intcmp_desc(void const *a, void const *b) {
  int ia = *(int *)a;
  int ib = *(int *)b;
  return ib - ia;
}

int main(void) {
  long sum = 0;
  int scan = 0;
  char *line = 0;
  size_t line_length = 0;
  StringGrid sg = mkstrgrid();

  for (; (scan = scanf("%ms ", &line)) != EOF;) {
    movstrgrid(&sg, line);
  }

  printf("Read %d lines.\n", sg.nrows);

  if (sg.nrows <= 2) {
    fprintf(stderr, "More than 2 lines must be supplied. Error!\n");
    return 1;
  }

  line_length = strlen(sg.grid[0]);

  VectorCursor vc = mkvcursor();

  for (int r = 0; r < sg.nrows; r++) {
    char *lines[3] = {0};

    if (r == 0) {
      lines[0] = NULL;
      lines[1] = sg.grid[r];
      lines[2] = sg.grid[r + 1];
      assert(lines[1]);
      assert(lines[2]);
    } else if (r == sg.nrows - 1) {
      lines[0] = sg.grid[r - 1];
      lines[1] = sg.grid[r];
      lines[2] = NULL;
      assert(lines[0]);
      assert(lines[1]);
    } else {
      lines[0] = sg.grid[r - 1];
      lines[1] = sg.grid[r];
      lines[2] = sg.grid[r + 1];
      assert(lines[0]);
      assert(lines[1]);
      assert(lines[2]);
    }

    for (int c = 0; c < line_length; c++) {
      if (local_low(line_length, c, lines)) {
        dbgprintf("Found r %d, c %d '%c'\n", r, c, lines[1][c]);
        sum += lines[1][c] - '0' + 1;
        insvcursor(&vc, (Cursor){.row = r, .col = c});
      }
    }
  }

  printf("Sum = %ld\n", sum);

  // Time to do the secondary processing, which is to assign to each number the
  // basin number. The basin numbers are to be printed in a grid structure.

  IndexGrid ig = mkigrid(line_length, sg.nrows);
  int *basin_freqs = xcalloc(vc.len, sizeof(int));

  dbgprintf("\nBasin chart (need big screen):\n");
  dbgprintf("%4s ", "");
  for (int c = 0; c < line_length; c++) {
    dbgprintf("[%2d] ", c);
  }
  dbgprintf("\n");

  int dbg_n_buggy = 0;
  for (int r = 0; r < sg.nrows; r++) {
    dbgprintf("[%2d] ", r);
    for (int c = 0; c < line_length; c++) {
      Cursor co = {.row = r, .col = c};
      int basin = greedy_find_basin(&sg, &ig, &vc, co);
      char ch = sg.grid[r][c];
      if (ch == '9') {
        dbgprintf("%3s  ", "X");
      } else if (basin == -1) {
        dbgprintf("%3s  ", ".");
        dbg_n_buggy++;
      } else {
        basin_freqs[basin]++;
        dbgprintf("%3d  ", basin);
      }
    }
    dbgprintf("\n");
  }
  dbgprintf("\n* X = the character was '9', so no basin assigned.\n"
            "* . = Unassigned (due to a bug; %d found).\n",
            dbg_n_buggy);

  dbgprintf("\nGrid legend:\n");
  for (int i = 0; i < vc.len; i++) {
    Cursor co = vc.cos[i];
    char ch = sg.grid[co.row][co.col];
    dbgprintf("[%2d] @ (r %2d, c %2d) '%c' (%d found)\n", i, co.row, co.col, ch,
              basin_freqs[i]);
  }

  dbgprintf("\nTop 3 basin sizes:\n");
  qsort(basin_freqs, vc.len, sizeof(int), intcmp_desc);
  long basin_top3 = 1;
  for (int i = 0; i < tg_min(3, vc.len); i++) {
    dbgprintf("A basin of size %d\n", basin_freqs[i]);
    basin_top3 *= basin_freqs[i];
  }

  dbgflush(stderr);
  printf("Product: %ld\n", basin_top3);

  free(basin_freqs);
  deligrid(ig);
  delvcursor(vc);
  delstrgrid(sg);
}
