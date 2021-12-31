// Given the inputs from day 8,
// discern which patterns are digits 2, 3 and 6.
// edit: now this program can discern
// which pattern is which digit [0-9].

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/iminmax.h"
#include "lib/xalloc.h"

static int charcmp(void const *a, void const *b) {
  int ca = *(char *)a;
  int cb = *(char *)b;
  return ca - cb;
}

static void qsortstr8(char *s) {
  size_t ns = strnlen(s, 8);
  qsort(s, ns, 1, charcmp);
}

// Calculate the simple character-set intersection represented
// by nonduplicated sorted strings (a, b).
// out is supposed to be allocated, and large enough to contain
// 8 characters.
static void strinter(const char *a, const char *b, char *out) {
  while (*a != '\0' && *b != '\0') {
    if (*a == *b) {
      *out = *a;
      out++;
      a++;
      b++;
    } else if (*a < *b) {
      a++;
    } else {
      b++;
    }
  }
}

// out is suppose to be able to collect up to 8 characters.
static void strunion(const char *a, const char *b, char *out) {
  while (*a != '\0' && *b != '\0') {
    if (*a == *b) {
      *out = *a;
      out++;
      a++;
      b++;
    } else if (*a < *b) {
      *out++ = *a++;
    } else {
      *out++ = *b++;
    }
  }
  if (*a == '\0' && *b != '\0') {
    while (*b != '\0') {
      *out++ = *b++;
    }
  } else if (*a != '\0' && *b == '\0') {
    while (*a != '\0') {
      *out++ = *a++;
    }
  }
}

int strmatch_earliest(const char *exact, char **array, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (strncmp(exact, array[i], 8) == 0) {
      return i;
    }
  }
  return -1;
}

void display_table(int *indices, int *reverse, char **sequences, char **quizzes,
                   int phase, const char *group10, const char *group20) {
  printf("%8s | %-8s | %-8s | id (%d)\n", "", "hint", "ask", phase);
  for (int i = 0; i < 10; i++) {
    int id = indices[i];
    printf("%8d | %-8s | %-8s", i, sequences[i], (i < 4) ? quizzes[i] : "");
    if (id >= 0 && id < 10) {
      printf(" | %-8d\n", id);
    } else {
      printf(" | ");
      switch (id) {
      case 10:
        printf("%-8s", group10);
        break;
      case 20:
        printf("%-8s", group20);
        break;
      default: /* do nothing */;
      }
      printf("\n");
    }
  }
  for (int i = 0; i < 10; i++) {
    int index = reverse[i];
    if (index >= 0) {
      printf("Identified: Number %d is at index %d (%s)\n", i, index,
             sequences[index]);
    }
  }
}

int main(void) {
  char *sequences[10];
  char *quizzes[4];
  int indices[10], reverse[10];
  for (int i = 0; i < 10; i++) {
    sequences[i] = xcalloc(8, sizeof(char));
    indices[i] = -1;
    reverse[i] = -1;
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
  for (int i = 0; i < 10; i++) {
    qsortstr8(sequences[i]);
  }
  // Identify which ones are 1, 4, 7 and 8.
  // In-band signaling:
  //  negative: unknown (default)
  //  [0-9]: the number specified
  //  10: 2, 3, or 5.
  //  20: 0, 6, or 9.
  //  others: undefined
  for (int i = 0; i < 10; i++) {
    size_t len = strnlen(sequences[i], 8);
    switch (len) {
    case 2:
      indices[i] = 1;
      reverse[1] = i;
      break;
    case 3:
      indices[i] = 7;
      reverse[7] = i;
      break;
    case 4:
      indices[i] = 4;
      reverse[4] = i;
      break;
    case 5:
      indices[i] = 10;
      break;
    case 6:
      indices[i] = 20;
      break;
    case 7:
      indices[i] = 8;
      reverse[8] = i;
      break;
    default: /* do nothing */;
    }
  }
  display_table(indices, reverse, sequences, quizzes, 1, "2,3,5?", "0,6,9?");
  // Now do the union in 1, 4, 9
  // Gues which index represents number 2.
  int indguess2 = -1;
  printf(" %-8s | orig | gues | %-8s      | %-8s      | %-8s      \n", "str",
         "union1", "union4", "union7");
  const char *patterns147[3] = {sequences[reverse[1]], sequences[reverse[4]],
                                sequences[reverse[7]]};
  for (int i = 0; i < 10; i++) {
    int indicator = indices[i];
    if (indicator < 10) {
      continue;
    }
    char const *pattern = sequences[i];
    char result1[8] = {0};
    char result4[8] = {0};
    char result7[8] = {0};
    strunion(pattern, patterns147[0], result1);
    strunion(pattern, patterns147[1], result4);
    strunion(pattern, patterns147[2], result7);

    // Specific questions
    int mat1 = strmatch_earliest(result1, sequences, 10);
    if (mat1 == -1) {
      indguess2 = i;
    }

    printf(" %-8s | %-4d | %-4d | %-8s (%-2d) | %-8s (%-2d) | %-8s (%-2d) \n",
           pattern, i, indicator, result1,
           strmatch_earliest(result1, sequences, 10), result4,
           strmatch_earliest(result4, sequences, 10), result7,
           strmatch_earliest(result7, sequences, 10));
  }
  // Guess which index is the number 9.
  int indguess9 = -1;
  printf(" %-8s | orig | gues | %-8s      | %-8s      | %-8s      \n", "str",
         "inter1", "inter4", "inter7");
  for (int i = 0; i < 10; i++) {
    int indicator = indices[i];
    if (indicator < 10) {
      continue;
    }
    char const *pattern = sequences[i];
    char result1[8] = {0};
    char result4[8] = {0};
    char result7[8] = {0};
    strinter(pattern, patterns147[0], result1);
    strinter(pattern, patterns147[1], result4);
    strinter(pattern, patterns147[2], result7);

    // Some specific questions
    // Guess 9
    int mat4 = strmatch_earliest(result4, sequences, 10);
    if (mat4 != -1) {
      indguess9 = i;
    }
    printf(" %-8s | %-4d | %-4d | %-8s (%-2d) | %-8s (%-2d) | %-8s (%-2d) \n",
           pattern, i, indicator, result1,
           strmatch_earliest(result1, sequences, 10), result4,
           strmatch_earliest(result4, sequences, 10), result7,
           strmatch_earliest(result7, sequences, 10));
  }
  printf("The number 2 is at index %d.\n", indguess2);
  indices[indguess2] = 2;
  reverse[2] = indguess2;
  printf("The number 9 is at index %d.\n", indguess9);
  indices[indguess9] = 9;
  reverse[9] = indguess9;
  display_table(indices, reverse, sequences, quizzes, 2, "3,5?", "0,6?");
  // Gues which index represents number 5.
  int indguess5 = -1;
  printf(" %-8s | orig | gues | %-8s      | %-8s      | %-8s      \n", "str",
         "union1", "union4", "union7");
  for (int i = 0; i < 10; i++) {
    int indicator = indices[i];
    if (indicator < 10) {
      continue;
    }
    char const *pattern = sequences[i];
    char result2[8] = {0};
    char result9[8] = {0};
    strunion(pattern, sequences[reverse[2]], result2);
    strunion(pattern, sequences[reverse[9]], result9);

    // Specific questions
    int mat1 = strmatch_earliest(result2, sequences, 10);
    if (mat1 == -1) {
      indguess5 = i;
    }

    printf(" %-8s | %-4d | %-4d | %-8s (%-2d) | %-8s (%-2d)\n", pattern, i,
           indicator, result2, strmatch_earliest(result2, sequences, 10),
           result9, strmatch_earliest(result9, sequences, 10));
  }
  // Guess which index is the number 9.
  int indguess6 = -1;
  printf(" %-8s | orig | gues | %-8s      | %-8s      | %-8s      \n", "str",
         "inter1", "inter4", "inter7");
  for (int i = 0; i < 10; i++) {
    int indicator = indices[i];
    if (indicator < 10) {
      continue;
    }
    char const *pattern = sequences[i];
    char result2[8] = {0};
    char result9[8] = {0};
    strinter(pattern, sequences[reverse[2]], result2);
    strinter(pattern, sequences[reverse[9]], result9);

    // Some specific questions
    // Guess 9
    int mat9 = strmatch_earliest(result9, sequences, 10);
    if (mat9 != -1) {
      indguess6 = i;
    }
    printf(" %-8s | %-4d | %-4d | %-8s (%-2d) | %-8s (%-2d)\n", pattern, i,
           indicator, result2, strmatch_earliest(result2, sequences, 10),
           result9, strmatch_earliest(result9, sequences, 10));
  }
  printf("The number 5 is at index %d.\n", indguess5);
  indices[indguess5] = 5;
  reverse[5] = indguess5;
  printf("The number 6 is at index %d.\n", indguess6);
  indices[indguess6] = 6;
  reverse[6] = indguess6;
  for (int i = 0; i < 10; i++) {
    int number = indices[i];
    if (number == 10) {
      indices[i] = 3;
      reverse[3] = i;
    } else if (number == 20) {
      indices[i] = 0;
      reverse[0] = i;
    }
  }
  display_table(indices, reverse, sequences, quizzes, 3, "", "");
  printf("Answering queries: \n");
  for (int i = 0; i < 4; i++) {
    char sorted_q[8] = {0};
    strncpy(sorted_q, quizzes[i], 7);
    qsortstr8(sorted_q);
    int found_at = strmatch_earliest(sorted_q, sequences, 10);
    if (found_at == -1) {
      fprintf(stderr, "Match for pattern \"%s\" Not found. Incorrect error?\n",
              quizzes[i]);
      exit(1);
    }
    dbgprintf("pattern \"%s\" (sorted \"%s\"), index %d\n", quizzes[i],
              sorted_q, found_at);
    int number = reverse[found_at];
    printf("The query %s is found at %d to be number %d.\n", sorted_q, found_at,
           number);
  }
  for (int i = 0; i < 10; i++) {
    free(sequences[i]);
    if (i < 4)
      free(quizzes[i]);
  }
  return 0;
}
