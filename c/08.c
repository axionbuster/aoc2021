// Add up the 4-digit numbers created by concatenating the digits reconstructed
// from the hints.

#include "lib/dbgprint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CLASS069 = 10, CLASS235 = 20 };

// Look up class by string length (index).
//  Lengths 0, 1 ==> not possible (-1).
//  Specific number is returned if it's that digit.
//  So, for example, '2' is returned if it's exactly '2'.
//  Value 10 (CLASS069) is returned if it's one of 0, 6 and 9.
//  Value 20 (CLASS235) is returned if it's one of 2, 3 and 5.
static int lookup1478[8] = {
    -1, -1, 1, 7, 4, CLASS069, CLASS235, 8,
};

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

// out is supposed to be able to collect up to 8 characters.
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

// Return the index of the earliest occurrence of exact in array
// through linear search at beginning or return -1 when not found.
static int strmatch(const char *exact, char array[10][8]) {
  for (size_t i = 0; i < 10; i++) {
    if (strncmp(exact, array[i], 8) == 0) {
      return i;
    }
  }
  return -1;
}

static void dbgprintassoc(int cl, int ix) {
  dbgprintf("Location of digit '%d' at index %d\n", cl, ix);
}

typedef struct {
  int classes[10];
  int indices[10];
} Bidict;

// Associate classes with indices.
static void putbd(Bidict *bd, int c, int i) {
  bd->classes[i] = c;
  bd->indices[c] = i;
}

int main(void) {
  size_t nlines = 0;
  long sum = 0;
  char patterns[10][8] = {0};
  char queries[4][8] = {0};
  Bidict bd;

  for (int i = 0; i < 10; i++) {
    bd.classes[i] = -1;
    bd.indices[i] = -1;
  }

  for (int scan = 0;
       (scan = scanf(
            "%7s %7s %7s %7s %7s %7s %7s %7s %7s %7s | %7s %7s %7s %7s",
            patterns[0], patterns[1], patterns[2], patterns[3], patterns[4],
            patterns[5], patterns[6], patterns[7], patterns[8], patterns[9],
            queries[0], queries[1], queries[2], queries[3])) != EOF;
       nlines++) {
    if (scan != 14) {
      fprintf(stderr, "Parse failure. expect %d words, get %d\n", 14, scan);
      return 1;
    }

    // Find 1, 4, 7 and 8.
    // Find the classes "10" and "20".
    // At completion all numbers are classed into one of the three classes.
    for (int p = 0; p < 10; p++) {
      qsortstr8(patterns[p]);
      size_t pattern_len = strnlen(patterns[p], 8);
      int pattern_class = lookup1478[pattern_len];
      if (pattern_class > 0 && pattern_class < 10) {
        putbd(&bd, pattern_class, p);
      }
      bd.classes[p] = pattern_class;
    }

    // for (int p = 0; p < 10; p++) {
    //   dbgprintf("Pattern \"%s\" (index %d) is found to have class %d.\n",
    //             patterns[p], p, bd.classes[p]);
    // }
    // dbgprintf("\n");

    // dbgprintf("Class 1 --> %d\n", bd.indices[1]);
    // dbgprintf("Class 4 --> %d\n", bd.indices[4]);
    // dbgprintf("Class 7 --> %d\n", bd.indices[7]);
    // dbgprintf("Class 8 --> %d\n", bd.indices[8]);

    // Pass 1 (union with 1 and intersection with 4)
    // This step identifies the locations of digits 2 and 9.
    for (int p = 0; p < 10; p++) {
      int c = bd.classes[p];
      if (c < 10) {
        continue;
      }
      char uni1[8] = {0}, int4[8] = {0};
      strunion(patterns[p], patterns[bd.indices[1]], uni1);
      strinter(patterns[p], patterns[bd.indices[4]], int4);
      int mat1 = strmatch(uni1, patterns);
      int mat4 = strmatch(int4, patterns);
      if (mat1 == -1) {
        // dbgprintassoc(2, p);
        putbd(&bd, 2, p);
      } else if (mat4 != -1) {
        // dbgprintassoc(9, p);
        putbd(&bd, 9, p);
      }
    }

    // Pass 2 (union with 2 and intersection with 9)
    // This step identifies the locations of digits 5 and 6.
    for (int p = 0; p < 10; p++) {
      int c = bd.classes[p];
      if (c < 10) {
        continue;
      }
      char uni2[8] = {0}, int9[8] = {0};
      strunion(patterns[p], patterns[bd.indices[2]], uni2);
      strinter(patterns[p], patterns[bd.indices[9]], int9);
      int mat2 = strmatch(uni2, patterns);
      int mat9 = strmatch(int9, patterns);
      if (mat2 != -1 && c == 10) {
        // dbgprintassoc(5, p);
        putbd(&bd, 5, p);
      } else if (mat9 != -1 && c == 20) {
        // dbgprintassoc(6, p);
        putbd(&bd, 6, p);
      }
    }
    // dbgprintf("\n");

    // Pass 3 (deduction)
    // By deduction, reduce the remaining two digits (3 and 0).
    for (int p = 0; p < 10; p++) {
      int c = bd.classes[p];
      if (c == 10) {
        putbd(&bd, 3, p);
      } else if (c == 20) {
        putbd(&bd, 0, p);
      }
    }

    // Debug: report.
    for (int p = 0; p < 10; p++) {
      dbgprintassoc(bd.classes[p], p);
    }
    dbgprintf("\n");

    // Lastly: process the queries.
    static long const rpowten[4] = {1000, 100, 10, 1};
    for (int q = 0; q < 4; q++) {
      char sorted_query[8] = {0};
      strncpy(sorted_query, queries[q], 7);
      qsortstr8(sorted_query);
      int ix = strmatch(sorted_query, patterns);
      int cl = bd.classes[ix];
      sum += rpowten[q] * cl;
      printf("%d", cl);
      dbgflush(stdout);
      dbgprintf(" (unsorted \"%s\", sorted \"%s\")\n", queries[q],
                sorted_query);
    }
    printf("\n");
  }
  printf("\nThe added up number is %lu.\n", sum);
  return 0;
}