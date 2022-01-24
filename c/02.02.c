#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define dbgprintf(args...) fprintf(stderr, args)
#define dbgvalp() dbgprintf("x = %d, y = %d, r = %d\n", x, y, r)
#else
#define dbgprintf(args...)
#define dbgvalp()
#endif

#define FORWARDS "forward"
#define FORWARDL sizeof(FORWARDS)
#define UPS "up"
#define UPL sizeof(UPS)
#define DOWNS "down"
#define DOWNL sizeof(DOWNS)

int main(void) {
  int x = 0, y = 0, r = 0;
  char *s = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&s, &len, stdin)) != -1) {
    // Really here for debugging string compares
    int cmp;
    if ((cmp = strncmp(s, FORWARDS, FORWARDL - 1)) == 0) {
      int dx = atoi(s + FORWARDL);
      x += dx;
      y += dx * r;
      dbgvalp();
    } else if ((cmp = strncmp(s, UPS, UPL - 1)) == 0) {
      int dr = atoi(s + UPL);
      // "up" DECREASES r
      r -= dr;
      dbgvalp();
    } else if (read == 1) {
      // Empty line
      break;
    } else {
      assert((cmp = strncmp(s, DOWNS, DOWNL - 1)) == 0);
      int dr = atoi(s + DOWNL);
      // "down" INCREASES r
      r += dr;
      dbgvalp();
    }
  }

  if (read == -1) {
    if (!feof(stdin)) {
      perror("getline");
      exit(EXIT_FAILURE);
    }
  }

  printf("distance (%d) * depth (%d) = %ld\n", x, y, (long)x * y);
  return 0;
}