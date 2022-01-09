#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

typedef struct cursor_t {
  int row;
  int col;
} Cursor;

static bool locallow(int ncols, int col, char *rows[3]) {
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
      if (locallow(line_length, c, lines)) {
        dbgprintf("Found r %d, c %d '%c'\n", r, c, lines[1][c]);
        sum += lines[1][c] - '0' + 1;
      }
    }
  }

  printf("Sum = %ld\n", sum);

  delstrgrid(sg);
}
