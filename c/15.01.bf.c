// Day 15, Part 1, Bellman-Ford

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbgprint.h"
#include "lib/iminmax.h"
#include "lib/xalloc.h"

typedef struct _pair_t {
  int from, to, weight;
} Pair;

#define make_pair(f, t, w) ((Pair){.from = (f), .to = (t), .weight = (w)})

typedef struct _pair_v_t {
  Pair *pairs;
  int cap, len;
} PairVector;

void insert_pair_vector(PairVector *pv, Pair p) {
  if (pv->len + 1 >= pv->cap) {
    pv->cap *= 2;
    pv->pairs = xrealloc(pv->pairs, pv->cap * sizeof(Pair));
  }
  pv->pairs[pv->len++] = p;
  pv->pairs[pv->len++] = make_pair(p.to, p.from, p.weight);
}

typedef struct _path_t {
  int cost, pred;
} Path;

#define DEFAULT_PATH ((Path){.cost = INT_MAX, .pred = -1})

// Saturated addition of two unsigned numbers.
unsigned sadd(unsigned x, unsigned y, unsigned max) {
  unsigned r = x + y;
  if (r < 0) {
    return tg_max(x, y);
  }
  if (r >= max) {
    return max;
  }
  return r;
}

Path *bellman_ford(int start, int end, PairVector const *pv) {
  assert(start < end);
  Path *paths = xmalloc(sizeof(Path) * end);
  for (int i = 0; i < end; i++) {
    paths[i] = DEFAULT_PATH;
  }
  paths[start].cost = 0;
  for (int r = 0; r < end; r++) {
    for (int i = 0; i < pv->len; i++) {
      Pair p = pv->pairs[i];
      int new_cost = (int)sadd(paths[p.from].cost, p.weight, INT_MAX);
      int old_cost = paths[p.to].cost;
      if (new_cost < old_cost) {
        paths[p.to] = (Path){.cost = new_cost, .pred = p.from};
      }
    }
  }
  return paths;
}

#define inspair(pv, f, t, w) insert_pair_vector((pv), make_pair((f), (t), (w)))

int main(void) {
  PairVector pv =
      (PairVector){.cap = 1, .len = 0, .pairs = xcalloc(1, sizeof(Pair))};

  int st = 0, en = 5;
  inspair(&pv, 0, 1, 4);
  inspair(&pv, 1, 2, 4);
  inspair(&pv, 2, 3, 4);
  inspair(&pv, 0, 3, 16);
  inspair(&pv, 0, 4, 3);
  inspair(&pv, 3, 4, 20);

  Path *paths = bellman_ford(st, en, &pv);

  printf("From vertex %d to vertices 0 ... %d\n", st, en);
  for (int i = 0; i < en; i++) {
    Path p = paths[i];
    if (p.cost < INT_MAX) {
      printf("%d -> %d (cost: %d, pred: %d)\n", st, i, p.cost, p.pred);
    } else {
      printf("%d -> %d (unreachable)\n", st, i);
    }
  }

  free(paths);
  paths = bellman_ford(3, en, &pv);

  printf("From vertex %d to vertices 0 ... %d\n", 3, en);
  for (int i = 0; i < en; i++) {
    Path p = paths[i];
    if (p.cost < INT_MAX) {
      printf("%d -> %d (cost: %d, pred: %d)\n", 3, i, p.cost, p.pred);
    } else {
      printf("%d -> %d (unreachable)\n", 3, i);
    }
  }

  free(paths);
  free(pv.pairs);
}
