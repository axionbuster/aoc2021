#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

typedef struct boardsiz_t {
  int ncols;
} BoardSize;

static bool posgood(BoardSize const *bs, int col) {
  if (col < 0 || col >= bs->ncols)
    return false;
  return true;
}

static bool locallow(BoardSize const *bs, int col, char *rows[3]) {
  int up, down, left, right, on;
  up = rows[0][col] - '0';
  down = rows[2][col] - '0';
  left = posgood(bs, col - 1) ? rows[1][col - 1] - '0' : INT_MAX;
  right = posgood(bs, col + 1) ? rows[1][col + 1] - '0' : INT_MAX;
  on = rows[1][col] - '0';

  return on < up && on < down && on < left && on < right;
}

int main(void) {
  long sum = 0;
  int ln = 0;
  int scan;
  BoardSize bs = {0};
  char *lines[3];
  lines[0] = xcalloc(1, sizeof(char));
  lines[1] = xcalloc(1, sizeof(char));
  lines[2] = xcalloc(1, sizeof(char));

  for (; (scan = scanf("%ms ", &lines[2])) != EOF; ln++) {
    dbgprintf("%d: %s\n", ln, lines[2]);

    if (ln == 1) {
      // The second line --- look-behind is disabled.
      int left, right, down, on;
      for (int col = 0; col < bs.ncols; col++) {
        left = posgood(&bs, col - 1) ? lines[1][col - 1] - '0' : INT_MAX;
        right = posgood(&bs, col + 1) ? lines[1][col + 1] - '0' : INT_MAX;
        down = lines[2][col] - '0';
        on = lines[1][col] - '0';
        if (on < left && on < right && on < down) {
          dbgprintf("Row %d, column %d, line \"%7s\", character: '%c'\n",
                    ln - 1, col, lines[1], lines[1][col]);
          sum += on + 1;
        }
      }
    } else if (ln != 0) {
      for (int col = 0; col < bs.ncols; col++) {
        if (locallow(&bs, col, lines)) {
          dbgprintf("Row %d, column %d, line \"%7s\", character: '%c'\n",
                    ln - 1, col, lines[1], lines[1][col]);
          sum += lines[1][col] - '0' + 1;
        }
      }
    } else {
      bs.ncols = strlen(lines[2]);
    }

    free(lines[0]);
    lines[0] = lines[1];
    lines[1] = lines[2];
  }

  // lines[0] and lines[1] are valid, and even though so is lines[2], it
  // points to the same memory location as lines[1]. This is the last line.
  // Look-ahead is disabled.
  {
    int left, right, up, on;
    for (int col = 0; col < bs.ncols; col++) {
      left = posgood(&bs, col - 1) ? lines[1][col - 1] - '0' : INT_MAX;
      right = posgood(&bs, col + 1) ? lines[1][col + 1] - '0' : INT_MAX;
      up = lines[0][col] - '0';
      on = lines[1][col] - '0';
      if (on < left && on < right && on < up) {
        dbgprintf("Row %d, column %d, line \"%7s\", character: '%c'\n", ln - 1,
                  col, lines[1], lines[1][col]);
        sum += on + 1;
      }
    }
  }

  free(lines[0]);
  free(lines[1]);

  printf("Sum: %ld.\n", sum);
  printf("Processing has ended. Thank you.\n");

  return 0;
}
