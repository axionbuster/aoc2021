#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/charvec.h"
#include "lib/dbgprint.h"

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

long closingscore(char c) {
  switch (c) {
  case ')':
    return 3;
  case ']':
    return 57;
  case '}':
    return 1197;
  case '>':
    return 25137;
  default:
    dbgprintf("closingscore (warning): c '%c' unrecognized, return -1.\n", c);
    return -1;
  }
}

int main(void) {
  long total_score = 0;
  int scan, iline;
  char *line;
  for (iline = 0; (scan = scanf("%ms ", &line)) != EOF; iline++) {
    size_t llen = strlen(line);
    long illegal_score = 0;
    CharVec *cv = xmkcharvec();

    for (int i = 0; i < llen; i++) {
      char oc = line[i];
      if (isopening(oc)) {
        xinscharvec(cv, oc);
      } else {
        char ec = xpopcharvec(cv);
        ec = matchopener(ec);
        if (ec != oc) {
          dbgprintf("Mismatch found. Expect = '%c', Original = '%c'\n", ec, oc);
          long little_score = closingscore(oc);
          dbgprintf("\tThe mismatch to cost %ld points.\n", little_score);
          illegal_score += little_score;
        }
      }
    }

    total_score += illegal_score;
    if (illegal_score)
      dbgprintf("\nLine %-4d ... score was %ld\n\n", iline, illegal_score);

    freecharvec(cv);
    free(line);
  }

  printf("%d lines processed, total score %ld.\n", iline, total_score);
}
