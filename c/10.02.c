#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/charvec.h"
#include "lib/dbgprint.h"
#include "lib/longvec.h"

bool isopening(char c) { return c == '(' || c == '[' || c == '{' || c == '<'; }

char matchopener(char c) {
  switch (c) {
  case '(':
    return ')';
  case '[':
    return ']';
  case '{':
    return '}';
  case '<':
    return '>';
  default:
    fprintf(stderr, "matchopener (fatal): Not an opener '%c'\n", c);
    abort();
  }
}

// Assign a point value to a closing character.
long autocomplete_clpoints(char c) {
  switch (c) {
  case ')':
    return 1;
  case ']':
    return 2;
  case '}':
    return 3;
  case '>':
    return 4;
  default:
    fprintf(stderr, "autocomplete_clpoints (fatal): Not a closing pair '%c'\n",
            c);
    abort();
  }
}

static int longcmp(void const *a, void const *b) {
  long ia = *(long *)a;
  long ib = *(long *)b;
  if (ia > ib)
    return 1;
  else if (ia < ib)
    return -1;
  return 0;
}

int main(void) {
  LongVec *scores = xmklongvec();
  int scan, iline;
  char *line;
  for (iline = 0; (scan = scanf("%ms ", &line)) != EOF; iline++) {
    size_t llen = strlen(line);
    CharVec *cv = xmkcharvec();

    for (int i = 0; i < llen; i++) {
      char oc = line[i];
      if (isopening(oc)) {
        xinscharvec(cv, oc);
      } else {
        char ec = xpopcharvec(cv);
        ec = matchopener(ec);
        if (ec != oc) {
          dbgprintf("Mismatch found. Expect = '%c', Original = '%c'.\n\t"
                    "Line %d is discarded.\n",
                    ec, oc, iline);
          goto end_line;
        }
      }
    }

    if (cv->len) {
      dbgprintf("Line %d is incomplete; it will be filled.\n\t\"%s ... ", iline,
                line);
      long local_points = 0;
      while (cv->len) {
        char ec = xpopcharvec(cv);
        ec = matchopener(ec);
        dbgprintf("%c", ec);
        local_points *= 5;
        local_points += autocomplete_clpoints(ec);
      }
      dbgprintf("\"\n\t%ld points gathered.\n", local_points);
      xinslongvec(scores, local_points);
    }

  end_line:

    freecharvec(cv);
    free(line);
  }

  qsort(scores->xs, scores->len, sizeof(long), longcmp);
  long median_score = scores->xs[scores->len / 2];

  dbgprintf("\n\nScores:\n");
  for (int i = 0; i < scores->len; i++) {
    if (i == scores->len / 2)
      dbgprintf("--->: %ld\n", scores->xs[i]);
    else
      dbgprintf("%4d: %ld\n", i, scores->xs[i]);
  }
  dbgprintf("Median index: %zd\n\n", scores->len / 2);
  dbgflush(stderr);

  freelongvec(scores);

  printf("%d lines processed, median score %ld.\n", iline, median_score);
}
