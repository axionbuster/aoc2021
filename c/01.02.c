#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define dbgprintf(args...) fprintf(stderr, args)
#else
#define dbgprintf(args...)
#endif

int main(void) {
  int larger = 0;

  int prev = INT32_MIN;
  int x = INT32_MIN, y = INT32_MIN, z = INT32_MIN;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, stdin)) != -1) {
    if (x == INT32_MIN) {
      x = atoi(line);
    } else if (y == INT32_MIN) {
      y = atoi(line);
    } else if (z == INT32_MIN) {
      z = atoi(line);
      int cur = atoi(line) + x + y;
      if (cur > prev) {
        larger++;
      }
      prev = cur;
    } else {
      int w = atoi(line);
      dbgprintf("... w = %d\n", w);
      x = y;
      y = z;
      z = w;
      int sum = x + y + z;
      if (prev < sum) {
        dbgprintf("x = %d, y = %d, z = %d, sum = %d\n", x, y, z, sum);
        larger++;
      }
      prev = sum;
    }
  }
  free(line);
  if (read == -1) {
    if (!feof(stdin)) {
      perror("getline");
      exit(EXIT_FAILURE);
    }
  }
  larger--;
  if (larger < 0) {
    larger = 0;
  }
  printf("%d\n", larger);
  return 0;
}