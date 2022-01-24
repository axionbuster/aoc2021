#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FORWARDS "forward"
#define FORWARDL sizeof(FORWARDS)
#define UPS "up"
#define UPL sizeof(UPS)
#define DOWNS "down"
#define DOWNL sizeof(DOWNS)

int main(void) {
  int x = 0, y = 0;
  char *s = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&s, &len, stdin)) != -1) {
    // Really here for debugging string compares
    int cmp;
    if ((cmp = strncmp(s, FORWARDS, FORWARDL - 1)) == 0) {
      int dx = atoi(s + FORWARDL);
      x += dx;
    } else if ((cmp = strncmp(s, UPS, UPL - 1)) == 0) {
      int dy = atoi(s + UPL);
      // "up" DECREASES y
      y -= dy;
    } else if (read == 1) {
      // Empty line
      break;
    } else {
      assert((cmp = strncmp(s, DOWNS, DOWNL - 1)) == 0);
      int dy = atoi(s + DOWNL);
      // "down" INCREASES y
      y += dy;
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