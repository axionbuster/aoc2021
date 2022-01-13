#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static void initgrid(Grid *g) {
  g->grid = NULL;
  g->width = 0;
  g->height = 0;
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
#ifndef NDEBUG
  int tty = isatty(STDERR_FILENO);
#else
  int tty = 0;
#endif
  char const *normal = tty ? TEXTNORMAL : "";
  char const *boldtext = tty ? TEXTBOLD : "";
  signed char ch;
  bool bold;
  FORRC(r, g->height, c, g->width) {
    Cursor co = mkcur(r, c);
    ch = *at(g, co);
    if (!c) {
      if (bold) {
        dbgprintf(normal);
      }
      bold = false;
      dbgprintf("\n");
    }
    if (ch && bold) {
      bold = false;
      dbgprintf(normal);
    }
    if (!ch && !bold) {
      bold = true;
      dbgprintf(boldtext);
    }

    dbgprintf("%1x", ch);
  }

  if (bold) {
    dbgprintf(normal);
  }
  dbgprintf("\n");
}

// Copy ASCII numeral rows (C-string) into the grid.
static void cpyrawrow(Grid *g, char const *restrict row) {
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
    *x = (*x > 9 || *x < 0) ? 0 : *x;
  }
}

// For any element who is above 9, increment all adjacent elements by 1.
// Elements are not truncated. Mark those already flashed with a negative
// number. Return nonnegative if and only if at least one octopus flashed.
// Return value is always nonnegative.
static int flashall(Grid *g) {
  int flash = 0;
  FORRC(r, g->height, c, g->width) {
    signed char *gg = at(g, mkcur(r, c));
    if (*gg > 9) {
      flash++;
      *gg = -1;
      FORRC(rr, 3, cc, 3) {
        if (rr == 1 && cc == 1)
          continue;
        signed char *mm = at(g, mkcur(r + rr - 1, c + cc - 1));
        if (mm && *mm >= 0)
          (*mm)++;
      }
    }
  }
  return flash;
}

int main(void) {
  Grid g;
  initgrid(&g);

  char *line;
  for (int scan = 0; (scan = scanf("%ms ", &line)) != EOF; free(line)) {
    cpyrawrow(&g, line);
  }

  dbgprintf("Before any steps:");
  dbggrid(&g);
  dbgprintf("\n");

  int all_flash = g.width * g.height;
  int flash = 0;
  for (int step = 1;; step++) {
    int step_flash = 0, local_flash = 0;
    incall(&g);
    while ((local_flash = flashall(&g))) {
      step_flash += local_flash;
    }
    flash += step_flash;
    truncall(&g);
    if (step <= 10 || step % 10 == 0 || step_flash == all_flash) {
      dbgprintf("After step %d:", step);
      dbggrid(&g);
      dbgprintf("\n");
    }
    if (step == 100) {
      dbgflush(stderr);
      printf("After 100 steps, %d flashes were found.\n", flash);
      dbgflush(stdin);
    }
    if (step_flash == all_flash) {
      dbgflush(stderr);
      printf("On step %d, all octopuses flash (%d)\n", step, step_flash);
      dbgflush(stdin);
      break;
    }
  }

  delgrid(g);
  return 0;
}
