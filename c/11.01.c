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

#define FORRC(rlim, clim)                                                      \
  for (int r = 0; r < rlim; r++) {                                             \
    for (int c = 0; c < clim; c++) {

#define ENDFORRC                                                               \
  }                                                                            \
  }

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

// Implement rule 2 of the description, which is summarized as "increase
// adjacent members of those who are flashing."
static void incadj(StringGrid *sg) {
  // The following code only executes at first call.

  static StringGrid *scratch = NULL;

  if (!scratch) {
    scratch = xmalloc(sizeof(StringGrid));
    *scratch = mkstrgrid();
    for (int rr = 0; rr < sg->nrows; rr++) {
      signed char *line = xmalloc(sg->ncols);
      movstrgrid(scratch, line);
    }

    scratch->ncols = sg->ncols;

    assert(scratch->nrows == sg->nrows);
    assert(scratch->ncols == sg->ncols);
    assert(scratch->nrows > 0);
    assert(scratch->ncols > 0);
  }

  // The subsequent code may execute more than one time.

  // Zero out the update grid.
  for (int r = 0; r < scratch->nrows; r++) {
    signed char *row = scratch->grid[r];
    for (int c = 0; c < scratch->ncols; c++) {
      row[c] = 0;
    }
  }

  // Do step 2 and 3 of the given rule description.
  int flashed = 1;

  while (flashed--) {
    // Loop while any new flashes are generated.
    // How "nflash--" works:
    //  1. At first nflash != 0 so the loop is always entered.
    //     Once it enters the loop, nflash == 0.
    //  2. If the loop does not touch nflash at all then nflash == 0
    //     at the next loop. Break (therefore, only when no more flashes occur)
    //  3. If any flash occurs, and the loop engages again, nflash is reset to 0

    for (int r = 0; r < sg->nrows; r++) {
      for (int c = 0; c < sg->ncols; c++) {
        signed char *oc = charat(sg, r, c);
        if (*oc > 9) {
          // Increase adjacent.
          for (int rr = -1; rr <= 1; rr++) {
            for (int cc = -1; cc <= 1; cc++) {
              if (rr != 0 && cc != 0) {
                signed char *relc = charat(scratch, r + rr, c + cc);
                // Negative values in scratch means that it has flashed before
                // and therefore must never flash again.
                if (relc && *relc >= 0)
                  (*relc) = 1;
              }
            }
          }
        }
      }
    }

    for (int r = 0; r < sg->nrows; r++) {
      for (int c = 0; c < sg->ncols; c++) {
        signed char *oc = charat(sg, r, c);
        signed char *sc = charat(scratch, r, c);
        *oc += *sc;
        if (*oc > 9) {
          // When a value in scratch is negative, it will never flash again.
          *sc = -1;
          flashed = 1;
        }
      }
    }

    // Zero out the update grid.
    for (int r = 0; r < scratch->nrows; r++) {
      signed char *row = scratch->grid[r];
      for (int c = 0; c < scratch->ncols; c++) {
        row[c] = 0;
      }
    }
    // Debug
    dbg_print_sg(sg);
  }
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
        (*charat(&sg, r, c))++;
      }
    }

    incadj(&sg);

    for (int r = 0; r < sg.nrows; r++) {
      for (int c = 0; c < sg.ncols; c++) {
        signed char *pch = charat(&sg, r, c);
        if (*pch > 9) {
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