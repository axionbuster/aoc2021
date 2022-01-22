//
// Created by YuJin Kim on 1/20/22.
//
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/iminmax.h"
#include "lib/xalloc.h"

// Usage: ./14.01.[dbg|rel] [optional step-count] < input 2> trace > output

#define MAX_STEPS 4

typedef struct {
  char match_with[3];
  char replace_with;
} MatchPair;

typedef struct {
  MatchPair *mps;
  int len, cap;
} MatchRules;

typedef struct {
  ptrdiff_t pos;
  char with;
} Substitution;

typedef struct {
  Substitution *subs;
  int len, cap;
} Plan;

static int subcomp(void const *vpsub1, void const *vpsub2) {
  Substitution const *sub1 = vpsub1, *sub2 = vpsub2;
  if (sub1->pos < sub2->pos)
    return -1;
  else if (sub1->pos > sub2->pos)
    return 1;
  return 0;
}

int main(int argc, char *argv[]) {
  int max_steps = argc == 2 ? atoi(argv[1]) : MAX_STEPS;

  char *line = NULL;
  size_t cap = 0;
  ssize_t read = getline(&line, &cap, stdin);

  if (read <= 0) {
    fprintf(stderr, "Error reading first line.\n");
    free(line);
    return 1;
  }
  line[read - 1] = '\0';

  // Match rules are interpreted from the input at once and never change.
  MatchRules rules = {0};
  rules.cap = 1;
  rules.mps = xcalloc(rules.cap, sizeof(MatchPair));
  while (scanf("%2s -> %c", rules.mps[rules.len].match_with,
               &rules.mps[rules.len].replace_with) == 2) {
    rules.len++;
    if (rules.len == rules.cap) {
      rules.cap *= 2;
      rules.mps = xrealloc(rules.mps, rules.cap * sizeof(MatchPair));
    }
  }
  rules.mps = xrealloc(rules.mps, rules.len * sizeof(MatchPair));
  rules.cap = rules.len;

  char *work;
  for (int step = 1; step <= max_steps; step++) {
    // In each step:
    //  1. A PLAN is made from the LINE. A PLAN is an ordered collection
    //     of substitutions to be made, sorted by position (ascending).
    //  2. (Debug) The line and the plan for subsitutitons are shown.
    //  3. The substitutions are made and the original LINE is overwritten.
    Plan plan = {0};
    plan.cap = 1;
    plan.subs = xcalloc(plan.cap, sizeof(Substitution));
    for (int ptn = 0; ptn < rules.len; ptn++) {
      MatchPair mp = rules.mps[ptn];
      char const *needle = line;
      // Repeated linear search and push back into plan vector.
      while ((needle = strstr(needle, mp.match_with))) {
        // Note that needle and line are both absolute pointer locations.
        Substitution s = {.pos = needle - line, .with = mp.replace_with};
        // push-back s into plan.
        if (plan.len == plan.cap) {
          plan.cap *= 2;
          plan.subs = xrealloc(plan.subs, plan.cap * sizeof(Substitution));
        }
        plan.subs[plan.len++] = s;
        // end?
        if (!*needle)
          break;
        // strstr returns the pointer where the match was found so you
        // skip one character ahead before resuming search.
        needle++;
      }
    }
    qsort(plan.subs, plan.len, sizeof(Substitution), subcomp);

    dbgprintf("Step %d: \"%s\"\n", step, line);
    for (int i = 0; i < plan.len; i++) {
      Substitution sub = plan.subs[i];
      dbgprintf("Sub \"%c%c\" (position %ld) with \"%c\"\n", line[sub.pos],
                line[sub.pos + 1], sub.pos, sub.with);
    }

    // The far-below sprintf calls read from an earlier copy of
    // 'line,' while writing to a new copy of that 'line,' called 'work.'
    //
    // This new copy of 'line' is to become 'line' itself by reference
    // re-assignment. The old 'line' is to be freed.
    int subs_used = 0, last_sub = 0, wrote = 0, len = strlen(line);
    work = xcalloc(len * 2 + 1, sizeof(char));
    strcpy(work, line);

    // The last character at len - 1 is always printed, and is handled
    // by a special routine outside of the inner pattern-substitution loop.
    for (int i = 0; i < len - 1; i++) {
      for (int my_sub = last_sub; my_sub < plan.len; my_sub++) {
        if (plan.subs[my_sub].pos != i) {
          continue;
        }
        // Linearly search for a pattern exactly at position = (i).
        // Due to the order imposed by earlier sorting this is not expected
        // to be costly.
        Substitution sub = plan.subs[my_sub];
        wrote += sprintf(work + wrote, "%c%c", line[sub.pos], sub.with);
        subs_used++;
        goto skip_i;
      }
      // If search fails (and the inner loop terminates), the character at
      // (i) from the read buffer ('line') should be written (to 'work').
      wrote += sprintf(work + wrote, "%c", line[i]);
    skip_i: /* follow-through */
      // If search succeeds, that character at (i) has been already written.
      // Advance to the next character from the read buffer ('line').
      continue;
    }
    wrote += sprintf(work + wrote, "%c", line[len - 1]);
    dbgprintf("work = %s\n", work);
    free(line);
    line = work;
    free(plan.subs);

    // Note at this point pointers line and work are aliases of each other.

    // Now we must count the least and most occuring ones.
    // Trust that the string is composed of letters [A-Z] only.
    len = strlen(line);
    char charmost = '?', charleast = '?';
    int counts['Z' - 'A' + 1] = {0};
    int charmostcount = INT_MIN, charleastcount = INT_MAX;
    for (int i = 0; i < len; i++) {
      char linechar = line[i];
      assert(linechar >= 'A');
      assert(linechar <= 'Z');
      counts[(int)linechar - 'A']++;
    }
    dbgprintf("Table\n");
    for (int i = 0; i <= 'Z' - 'A'; i++) {
      if (counts[i]) {
        dbgprintf("%4c | %d\n", i + 'A', counts[i]);
        if (counts[i] <= charleastcount) {
          charleastcount = counts[i];
          charleast = 'A' + i;
        }
        if (charmostcount <= counts[i]) {
          charmostcount = counts[i];
          charmost = 'A' + i;
        }
      }
    }
    printf("After step %d: (length %d)\n"
           "The most occurring ('%c') %d times\n"
           "The least occurring ('%c') %d times\n"
           "QUANTITY: %d\n",
           step, len, charmost, charmostcount, charleast, charleastcount,
           charmostcount - charleastcount);
  }
  dbgprintf("\n");

  free(rules.mps);
  free(work);
  return 0;
}
