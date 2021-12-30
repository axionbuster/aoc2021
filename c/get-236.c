// Given the inputs from day 8,
// discern which patterns are digits 2, 3 and 6.

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

int main(void) {
  char *sequences[10];
  char *quizzes[4];
  for (int i = 0; i < 10; i++) {
    sequences[i] = xcalloc(8, sizeof(char));
  }
  for (int i = 0; i < 4; i++) {
    quizzes[i] = xcalloc(8, sizeof(char));
  }
  int scan = scanf("%7s %7s %7s %7s %7s %7s %7s %7s %7s %7s | %7s %7s %7s %7s",
                   sequences[0], sequences[1], sequences[2], sequences[3],
                   sequences[4], sequences[5], sequences[6], sequences[7],
                   sequences[8], sequences[9], quizzes[0], quizzes[1],
                   quizzes[2], quizzes[3]);
  if (scan != 14) {
    if (scan == EOF) {
      fprintf(stderr, "Parse failure. early exhaustion of input.\n");
      exit(1);
    }
    fprintf(stderr, "Parse failure. expect %d words, get %d\n", 14, scan);
    exit(1);
  }
  printf("%8s | %8s | %8s\n", "", "hint", "ask");
  for (int i = 0; i < 10; i++) {
    printf("%8d | %8s | %8s\n", i, sequences[i], (i < 4) ? quizzes[i] : "");
  }
  for (int i = 0; i < 10; i++) {
    free(sequences[i]);
    if (i < 4)
      free(quizzes[i]);
  }
  return 0;
}