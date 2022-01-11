#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

#define TEXTBOLD "\e[1m"
#define TEXTNORMAL "\e[0m"

// Note that signed char is treated as ordinary integers, not characters.
typedef struct strgrid_t {
  signed char **grid;
  int ncols;
  int nrows;
  int caprows;
} StringGrid;

static StringGrid mkstrgrid() {
  StringGrid sg = {.ncols = 0, .nrows = 0, .caprows = 1};
  sg.grid = xcalloc(1, sizeof(signed char *));
  return sg;
}

static void delstrgrid(StringGrid sg) {
  for (int row = 0; row < sg.nrows; row++) {
    free(sg.grid[row]);
  }
  free(sg.grid);
}

static void movstrgrid(StringGrid *sg, signed char *row) {
  if (sg->nrows == sg->caprows) {
    sg->caprows *= 2;
    sg->grid = xrealloc(sg->grid, sg->caprows * sizeof(signed char *));
  }
  sg->grid[sg->nrows++] = row;
}

static signed char *charat(StringGrid *sg, int r, int c) {
  if (r < 0 || r >= sg->nrows)
    return NULL;
  if (c < 0 || c >= sg->ncols)
    return NULL;
  return &sg->grid[r][c];
}

// Simulation specific predicate. This predicate checks whether all octopuses
// are below 9.
/* static bool allbelow9(StringGrid *sg) {
  for (int r = 0; r < sg->nrows; r++) {
    for (int c = 0; c < sg->ncols; c++) {
      signed char ch = *charat(sg, r, c);
      if (ch >= 9)
        return false;
    }
  }
  return true;
} */

// This safely increases all adjacent cells to increase by 1. For nonexistant
// cells, including (r, c) proper, no action is performed.
static void incadj(StringGrid *sg, int r, int c) {
  static StringGrid *scratch = NULL;

  if (!scratch) {
    scratch = xmalloc(sizeof(StringGrid *));
    *scratch = mkstrgrid();
    for (int rr = 0; rr < sg->nrows; rr++) {
      signed char *line = xcalloc(rr, sizeof(signed char));
      scratch->grid[rr] = line;
    }
  }

  for (int rr = 0; rr < sg->nrows; rr++) {
    memset(scratch->grid[rr], 0, sg->ncols);
  }

  for (int rr = -1; rr <= 1; rr++) {
    for (int rc = -1; rc <= 1; rc++) {
      if (rr == 0 && rc == 0)
        continue;
      signed char *pc = charat(sg, r + rr, c + rc);
      if (pc) {
        assert(*pc < SCHAR_MAX);
        (*pc)++;
      }
    }
  }
}

// Print, alongwith a newline, the number in a line; bold characters '0'.
static void dbg_print_sg_line(StringGrid *sg, int r) {
  bool bold = false;

  for (int c = 0; c < sg->ncols; c++) {
    signed char ch = *charat(sg, r, c);
    if (ch == 0) {
      bold = true;
      dbgprintf(TEXTBOLD);
    } else if (bold) {
      dbgprintf(TEXTNORMAL);
    }
    dbgprintf("%x", ch);
  }

  if (bold) {
    dbgprintf(TEXTNORMAL);
  }
  dbgprintf("\n");
}

static void dbg_print_sg(StringGrid *sg) {
  for (int r = 0; r < sg->nrows; r++) {
    dbg_print_sg_line(sg, r);
  }
  dbgprintf("\n");
}

int main(void) {
  int scan, iline;
  char *line;
  StringGrid sg = mkstrgrid();

  for (iline = 0; (scan = scanf("%ms ", &line)) != EOF; iline++) {
    int i = 0;
    if (iline == 0) {
      sg.ncols = strlen(line);
    }
    signed char *alias = (signed char *)line;
    for (i = 0; *line != '\0'; i++, line++) {
      *line = *line - '0';
    }
    movstrgrid(&sg, (signed char *)alias);
  }

  dbgprintf("Before any steps:\n");
  for (int r = 0; r < sg.nrows; r++) {
    dbg_print_sg_line(&sg, r);
  }
  dbgprintf("\n");

  for (int step = 1; step <= 100; step++) {
    for (int r = 0; r < sg.nrows; r++) {
      for (int c = 0; c < sg.ncols; c++) {
        // signed char *psc = charat(&sg, r, c);
        // dbgprintf("Ptr %p (%d) at r %3d, c %3d.\n", psc, *psc, r, c);
        // dbgflush(stderr);
        (*charat(&sg, r, c))++;
      }
    }

    for (int r = 0; r < sg.nrows; r++) {
      for (int c = 0; c < sg.ncols; c++) {
        signed char ch = *charat(&sg, r, c);
        if (ch >= 9) {
          incadj(&sg, r, c);
        }
      }
    }
    for (int r = 0; r < sg.nrows; r++) {
      for (int c = 0; c < sg.ncols; c++) {
        signed char *pch = charat(&sg, r, c);
        if (*pch >= 9) {
          *pch = 0;
        }
      }
    }

    if (step <= 10 || step % 10 == 0) {
      dbgprintf("After step %d:\n", step);
      dbg_print_sg(&sg);
    }
  }

  delstrgrid(sg);
  return 0;
}