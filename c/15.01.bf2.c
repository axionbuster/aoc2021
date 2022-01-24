// Bellman-Ford implementation with grid

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

typedef struct _grid_t {
  int *xs, rows, columns;
} Grid;

// These macros caused me a lot of headaches so I'm going to replace them
// with a better impl.

#define at(g, r, c) ((g).xs[(r) * (g).columns + (c)])
#define i_at(g, r, c) ((r) * (g).columns + (c))
#define up(g, r, c) ((g).xs[(r - 1) * (g).columns + (c)])
#define i_up(g, r, c) ((r - 1) * (g).columns + (c))
#define down(g, r, c) ((g).xs[(r + 1) * (g).columns + (c)])
#define i_down(g, r, c) ((r + 1) * (g).columns + (c))
#define left(g, r, c) ((g).xs[(r) * (g).columns + (c - 1)])
#define i_left(g, r, c) ((r) * (g).columns + (c - 1))
#define right(g, r, c) ((g).xs[(r) * (g).columns + (c + 1)])
#define i_right(g, r, c) ((r) * (g).columns + (c + 1))
#define good(g, r, c)                                                          \
  ((r) >= 0 && (r) < (g).rows && (c) >= 0 && (c) < (g).columns)
#define for_rc(g, r, c)                                                        \
  for (int r = 0; r < (g).rows; r++)                                           \
    for (int c = 0; c < (g).columns; c++)

// The reason why this structure isn't used everywhere is because it was added
// right in the middle of the algorithm impl.

typedef struct _cursor_t {
  int row, column;
} Cursor;

#define cursor(r, c)                                                           \
  (Cursor) { .row = (r), .column = (c) }

#define PRIcursor(co) (co).row, (co).column

// Though the problem doesn't require one to trace the shortest path, thus
// eliminating a direct need for `pred`, I need it nevertheless for debugging
// purposes.
typedef struct _solution_t {
  int *cost;
  Cursor *pred;
  long n;
} Solution;

void delete_solution(Solution s) {
  free(s.cost);
  free(s.pred);
}

// Show bellman-ford solution at end.
void show_grid_status(Grid g, Solution s) {
  int r = g.rows - 1;
  int c = g.columns - 1;

  while (s.cost[i_at(g, r, c)]) {
    Cursor co = s.pred[i_at(g, r, c)];
    dbgprintf("(0, 0) -> (%d, %d) [%d], cost is %d, pred (%d, %d)\n", r, c,
              at(g, r, c), s.cost[i_at(g, r, c)], PRIcursor(co));
    r = co.row;
    c = co.column;
  }
}

// By saturated addition, (x + y) < z is correctly evaluated.
int saddcmp(int x, int y, int z) {
  unsigned ux = x, uy = y;
  if (ux + uy > INT_MAX) {
    return 0;
  } else {
    return x + y < z;
  }
}

Solution bellman_ford_grid(Grid const *g, int start_r, int start_c) {
  long n_vertices = (long)g->columns * (long)g->rows;

  Solution s = {.n = n_vertices,
                .cost = xmalloc(sizeof(int) * n_vertices),
                .pred = xmalloc(sizeof(Cursor) * n_vertices)};

  for (int i = 0; i < n_vertices; i++) {
    s.cost[i] = INT_MAX;
    s.pred[i] = (Cursor){.row = -1, .column = -1};
  }

  s.cost[i_at(*g, start_r, start_c)] = 0;

  for (long repeat = 0; repeat < n_vertices; repeat++) {
    long n_updates = 0;

    for_rc(*g, r, c) {
      Cursor co = cursor(r, c);
      int here_cost = s.cost[i_at(*g, r, c)];

      // Up
      if (good(*g, r - 1, c)) {
        int update = up(*g, r, c);
        if (saddcmp(here_cost, update, s.cost[i_up(*g, r, c)])) {
          dbgprintf("Up from (%d, %d), assign %d (from %d)\n", PRIcursor(co),
                    here_cost + update, s.cost[i_up(*g, r, c)]);
          n_updates++;
          s.cost[i_up(*g, r, c)] = here_cost + update;
          s.pred[i_up(*g, r, c)] = cursor(r, c);
        }
      }
      // Down
      if (good(*g, r + 1, c)) {
        int update = down(*g, r, c);
        if (saddcmp(here_cost, update, s.cost[i_down(*g, r, c)])) {
          dbgprintf("Down from (%d, %d), assign %d (from %d)\n", PRIcursor(co),
                    here_cost + update, s.cost[i_down(*g, r, c)]);
          n_updates++;
          s.cost[i_down(*g, r, c)] = here_cost + update;
          s.pred[i_down(*g, r, c)] = cursor(r, c);
        }
      }
      // Left
      if (good(*g, r, c - 1)) {
        int update = left(*g, r, c);
        if (saddcmp(here_cost, update, s.cost[i_left(*g, r, c)])) {
          dbgprintf("Left from (%d, %d), assign %d (from %d)\n", PRIcursor(co),
                    here_cost + update, s.cost[i_left(*g, r, c)]

          );
          n_updates++;
          s.cost[i_left(*g, r, c)] = here_cost + update;
          s.pred[i_left(*g, r, c)] = cursor(r, c);
        }
      }
      // Right
      if (good(*g, r, c + 1)) {
        int update = right(*g, r, c);
        if (saddcmp(here_cost, update, s.cost[i_right(*g, r, c)])) {
          dbgprintf("Right from (%d, %d), assign %d (from %d)\n", PRIcursor(co),
                    here_cost + update, s.cost[i_right(*g, r, c)]);
          n_updates++;
          s.cost[i_right(*g, r, c)] = here_cost + update;
          s.pred[i_right(*g, r, c)] = cursor(r, c);
        }
      }
    }

    if (n_updates) {
      dbgprintf("%ld updates were made.\n", n_updates);
    } else {
      dbgprintf("No further updates made in step %ld\n", repeat);
      break;
    }

    dbgprintf("Bellman-Ford Generation %ld\n", repeat);
    show_grid_status(*g, s);
    dbgprintf("Costs\n");
    for_rc(*g, r, c) {
      dbgprintf("(%d, %d) cost = %d, from (%d, %d)\n", r, c,
                s.cost[i_at(*g, r, c)], PRIcursor(s.pred[i_at(*g, r, c)]));
    }
  }

  return s;
}

Grid read_problem() {
  Grid g = {0};
  char *line = NULL;
  size_t cap = 0;
  // used for grid allocation
  size_t row_cap = 0;
  while (getline(&line, &cap, stdin) > 0) {
    if (g.columns == 0) {
      g.columns = strlen(line) - 1;
      g.rows = 1;
      g.xs = xcalloc(g.columns, sizeof(int));
      for (int i = 0; i < g.columns; i++) {
        g.xs[i] = line[i] - '0';
      }
      row_cap = 1;
    } else {
      if (g.rows == row_cap) {
        row_cap *= 2;
        g.xs =
            xrealloc(g.xs, (size_t)row_cap * (size_t)g.columns * sizeof(int));
      }
      for (int i = 0; i < g.columns; i++) {
        g.xs[i + g.rows * g.columns] = line[i] - '0';
      }
      g.rows++;
    }
  }
  free(line);
  dbgprintf("The grid (width = %d, height = %d)\n", g.columns, g.rows);
  for (int r = 0; r < g.rows; r++) {
    for (int c = 0; c < g.columns; c++) {
      dbgprintf("%d", at(g, r, c));
    }
    dbgprintf("\n");
  }
  return g;
}

int main(void) {
  Grid g = read_problem();

  Solution s = bellman_ford_grid(&g, 0, 0);

  // Only shown at debug compilation
  show_grid_status(g, s);

  printf("Cost %d.\n", s.cost[i_at(g, g.rows - 1, g.columns - 1)]);

  delete_solution(s);
  free(g.xs);
}
