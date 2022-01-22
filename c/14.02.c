#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

// The match string, consisting of two 8-bit ASCII characters, is
// coalesced into a single 16-bit unsigned integer. What is stored
// within the array is the number of each string stored.
static long pat_counts[USHRT_MAX];

// How many of each character appears. Indices lower than 'A' are unused
// (wasted).
static long c_counts['Z' + 1];

// The re-writing rules are stored in the same way, except that
// the return value encodes what it should be.
static char rewrites[USHRT_MAX];

// Turn a two-character array into a single integer.
#define COALESCE(c1, c2) (((c1) << 8) + (c2))

// Return the first character
#define UNCOAL_FIRST(uu) (char)((uu) >> 8)

// Return the second character
#define UNCOAL_SECOND(uu) (char)((uu)&0xff)

// Print patterns and characters found
static void dbg_pat_ccnts() {
  dbgflush(stdout);
  dbgprintf("Patterns found:\n");
  for (int i = 0; i < USHRT_MAX; i++) {
    if (pat_counts[i]) {
      dbgprintf("\t(%20ld) found: %c%c\n", pat_counts[i], UNCOAL_FIRST(i),
                UNCOAL_SECOND(i));
    }
  }
  dbgprintf("Characters found:\n");
  long tot_chars = 0;
  for (char c = 'A'; c <= (char)'Z'; c++) {
    long n = c_counts[(int)c];
    if (n) {
      tot_chars += n;
      dbgprintf("\t%c (found %ld)\n", c, n);
    }
  }
  dbgprintf("\tTotal: %ld\n", tot_chars);
  dbgflush(stderr);
}

int main(int argc, char *argv[]) {
  int max_steps = argc == 2 ? atoi(argv[1]) : 1;
  if (max_steps <= 0 || argc > 2) {
    fprintf(stderr, "Usage: (program) [steps] < in > out 2> trace\n");
    abort();
  }

  // Read in the line and the rules.
  // > line
  // >
  // > ((c1c2) -> (cr))*
  // example
  // > NNCB
  // >
  // > NN -> C
  // > HC -> B
  char *line = NULL, c1, c2, cr;
  size_t cap = 0;
  ssize_t read = getline(&line, &cap, stdin);
  if (read < 0) {
    fprintf(stderr, "Read line error (EOF)\n");
    abort();
  }
  line[read - 1] = '\0';
  for (int i = 0; i < read - 1; i++) {
    c_counts[(int)line[i]]++;
  }
  for (int i = 0; i < read - 2; i++) {
    pat_counts[COALESCE(line[i], line[i + 1])]++;
  }
  while (scanf(" %c%c -> %c ", &c1, &c2, &cr) == 3) {
    rewrites[COALESCE(c1, c2)] = cr;
  }

  // DEBUG ONLY
  dbgprintf("Line: %s\n", line);
  dbg_pat_ccnts();
  dbgprintf("Rules found:\n");
  for (int pat = 0; pat < USHRT_MAX; pat++) {
    if (rewrites[pat]) {
      dbgprintf("\tInsert %c within every \"%c%c\"\n", rewrites[pat],
                UNCOAL_FIRST(pat), UNCOAL_SECOND(pat));
    }
  }

  // End interpreting first line and the rewrite rules for the L-system.
  free(line);

  // Evolve the L-system from the previous value and the rewrite rules.
  for (int step = 1; step <= max_steps; step++) {
    // Simultaneous pattern replacement
    // Deleted patterns assume negative values.
    long pat_deltas[USHRT_MAX] = {0};

    for (int pat = 0; pat < USHRT_MAX; pat++) {
      if (rewrites[pat] && pat_counts[pat]) {
        // OK, detect the pattern in RULE and, at the same time, find it
        // actually existing in our current line.
        // Actions: (1) decompose pat (2) create two new patterns (3) increment
        // character count for the replacement character.
        c1 = UNCOAL_FIRST(pat);
        c2 = UNCOAL_SECOND(pat);
        cr = rewrites[pat];
        dbgprintf("Detected pattern %c%c to become %c%c + %c%c", c1, c2, c1, cr,
                  cr, c2);
        if (pat_counts[pat] > 1) {
          dbgprintf(" (%ld times)\n", pat_counts[pat]);
        } else {
          dbgprintf("\n");
        }
        int alt1 = COALESCE(c1, cr);
        int alt2 = COALESCE(cr, c2);
        pat_deltas[pat] -= pat_counts[pat];
        pat_deltas[alt1] += pat_counts[pat];
        pat_deltas[alt2] += pat_counts[pat];
        c_counts[(int)cr] += pat_counts[pat];
      }
    }
    for (int pat = 0; pat < USHRT_MAX; pat++) {
      if (pat_deltas[pat]) {
        dbgprintf("pat_deltas[%c%c] = %ld\n", UNCOAL_FIRST(pat),
                  UNCOAL_SECOND(pat), pat_deltas[pat]);
        pat_counts[pat] += pat_deltas[pat];
      }
      assert(pat_counts[pat] >= 0);
    }

    // Report the least and most commonly occurring characters
    long c_least_n = LONG_MAX, c_most_n = LONG_MIN;
    for (char c = 'A'; c <= (char)'Z'; c++) {
      long n = c_counts[(int)c];
      if (n) {
        if (n < c_least_n) {
          c_least_n = n;
        }
        if (c_most_n < n) {
          c_most_n = n;
        }
      }
    }
    dbgprintf("\n");
    printf("After step %d:\n"
           "\tmost %ld\n"
           "\tleast %ld times\n"
           "\tDelta = %ld\n",
           step, c_most_n, c_least_n, c_most_n - c_least_n);
    dbg_pat_ccnts();
  }

  return 0;
}
