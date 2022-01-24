#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define dbgprintf(args...) fprintf(stderr, args)
#else
#define dbgprintf(args...)
#endif

// Program parameters
// (Change if you get a different input file)
#define NBITS 12

int main(void) {
  int *n0 = calloc(NBITS, sizeof(int));
  int *n1 = calloc(NBITS, sizeof(int));
  char *s = NULL;
  size_t r = 0;
  ssize_t n;
  int dbg_isfirst = 1;

  while ((n = getline(&s, &r, stdin)) != -1) {
    if (dbg_isfirst) {
      assert(strlen(s) == NBITS + 1);
      dbg_isfirst = 0;
    }

    if (n == 1) {
      // Empty line
      break;
    }

    for (int i = 0; i < NBITS; i++) {
      int k = NBITS - 1 - i;
      if (s[i] == '1') {
        n1[k]++;
      } else {
        assert(s[i] == '0');
        n0[k]++;
      }
    }
  }
  free(s);
  if (n == -1) {
    if (!feof(stdin)) {
      perror("getline");
      exit(1);
    }
  }

  int eps = 0, gam = 0;

  printf("eps: ");
  for (int i = NBITS; i > 0; i--) {
    int k = i - 1;
    int d = n0[k] < n1[k] ? 0 : 1;
    eps += d << k;
    printf("%d", d);
  }
  printf(" (dec %d)\n", eps);
  printf("gam: ");
  for (int i = NBITS; i > 0; i--) {
    int k = i - 1;
    int d = n0[k] < n1[k] ? 1 : 0;
    gam += d << k;
    printf("%d", d);
  }
  printf(" (dec %d)\n", gam);
  printf("Power: %ld\n", (long)eps * gam);

  free(n1);
  free(n0);
  return 0;
}
