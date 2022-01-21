//
// Created by YuJin Kim on 1/20/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/intvec.h"
#include "lib/xalloc.h"

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

int main(void) {
  char *line = NULL;
  size_t cap = 0;
  ssize_t read = getline(&line, &cap, stdin);
  if (read <= 0) {
    fprintf(stderr, "Error reading first line.\n");
    free(line);
    return 1;
  }
  line[strlen(line) - 1] = '\0';

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

  Plan plan = {0};
  plan.cap = 1;
  plan.subs = xcalloc(plan.cap, sizeof(Substitution));
  for (int ptn = 0; ptn < rules.len; ptn++) {
    MatchPair mp = rules.mps[ptn];
    char const *needle = line;
    // Repeated linear search and push back into plan vector.
    while ((needle = strstr(needle, mp.match_with))) {
      Substitution s = {.pos = needle - line, .with = mp.replace_with};
      if (plan.len == plan.cap) {
        plan.cap *= 2;
        plan.subs = xrealloc(plan.subs, plan.cap * sizeof(Substitution));
      }
      plan.subs[plan.len++] = s;
      if (!*needle)
        break;
      needle++;
    }
  }
  qsort(plan.subs, plan.len, sizeof(Substitution), subcomp);

//  printf("\"%s\"\n", line);
//  for (int i = 0; i < plan.len; i++) {
//    Substitution sub = plan.subs[i];
//    printf("Sub \"%c%c\" (position %ld) with \"%c\"\n", line[sub.pos],
//           line[sub.pos + 1], sub.pos, sub.with);
//  }

  

  free(plan.subs);
  free(rules.mps);
  free(line);
  return 0;
}
