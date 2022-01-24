#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define dbgprintf(args...) fprintf(stderr, ##args)
#else
#define dbgprintf(args...)
#endif

#define perrorf(pattern, args...)                                              \
  fprintf(stderr, pattern ": %s\n", ##args, strerror(errno))

// Program parameters
// (Change if you get a different input file)
#define NBITS 12

typedef struct {
  uint64_t *data;
  ssize_t len;
  ssize_t cap;
} IntVec;

static IntVec *mkintvec(void) {
  IntVec *v = calloc(1, sizeof(IntVec));
  v->data = calloc(1, sizeof(uint64_t));
  v->len = 0;
  v->cap = 1;
  return v;
}

static void freeintvec(IntVec *v) {
  free(v->data);
  free(v);
}

static void insintvec(IntVec *v, uint64_t i) {
  if (v->len == v->cap) {
    v->cap *= 2;
    v->data = realloc(v->data, v->cap * sizeof(uint64_t));
    if (!v->data) {
      fprintf(stderr, "insintvec: realloc");
      exit(1);
    }
  }
  v->data[v->len++] = i;
}

static void compactvec(IntVec *v) {
  v->data = realloc(v->data, v->len * sizeof(uint64_t));
}

typedef struct {
  int *n0;
  int *n1;
} BitStats;

static BitStats *mkbstats(void) {
  BitStats *bs = calloc(1, sizeof(BitStats));
  bs->n0 = calloc(NBITS, sizeof(int));
  bs->n1 = calloc(NBITS, sizeof(int));
  return bs;
}

static void freebstats(BitStats *bs) {
  free(bs->n1);
  free(bs->n0);
  free(bs);
}

static void mark1(BitStats *bs, int bit_pos) { bs->n1[bit_pos]++; }

static void mark0(BitStats *bs, int bit_pos) { bs->n0[bit_pos]++; }

static int maxat(BitStats *bs, int bit_pos, int prefer) {
  assert(prefer == 0 || prefer == 1);
  if (bs->n0[bit_pos] < bs->n1[bit_pos]) {
    return 1;
  } else if (bs->n1[bit_pos] < bs->n0[bit_pos]) {
    return 0;
  } else {
    return prefer;
  }
}

static int minat(BitStats *bs, int bit_pos, int prefer) {
  assert(prefer == 0 || prefer == 1);
  if (bs->n0[bit_pos] < bs->n1[bit_pos]) {
    return 0;
  } else if (bs->n1[bit_pos] < bs->n0[bit_pos]) {
    return 1;
  } else {
    return prefer;
  }
}

#define BIT(n, p) (((n) & (1 << (p))) ? 1 : 0)

int main(int argc, char *argv[]) {
  FILE *file = NULL;
  if (argc != 2 || argv[1] == NULL) {
    fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
    return 1;
  }
  if ((file = fopen(argv[1], "r")) == NULL) {
    perrorf("Opening file \"%s\"", argv[1]);
    return 1;
  }
  IntVec *numbers = mkintvec();
  BitStats *bit_stats = mkbstats();

  char *s = NULL;
  size_t ns = 0;
  ssize_t read;
  while ((read = getline(&s, &ns, file)) > 0) {
    if (read == 1) {
      // Empty line
      break;
    }
    long lx = strtol(s, NULL, 2);
    dbgprintf("Number %lu for string %s", lx, s);
    insintvec(numbers, (uint64_t)lx);
  }
  free(s);
  if (read == -1) {
    if (!feof(file)) {
      perror("Reading file");
      exit(1);
    }
  }
  compactvec(numbers);

  for (int i = 0; i < numbers->len; i++) {
    uint64_t n = numbers->data[i];
    for (int j = 0; j < NBITS; j++) {
      uint64_t b = BIT(n, j);
      if (b) {
        mark1(bit_stats, j);
      } else {
        mark0(bit_stats, j);
      }
    }
  }

  // Find the gamma
  uint64_t gamma = 0, epsilon = 0;
  for (int i = 0; i < NBITS; i++) {
    int d = maxat(bit_stats, i, 0);
    gamma |= d << i;
  }
  for (int i = 0; i < NBITS; i++) {
    int d = minat(bit_stats, i, 0);
    epsilon |= d << i;
  }
  printf("gamma = (dec) %lu epsilon = (dec) %lu\nMultiply: (dec) %lu\n",
         (unsigned long)gamma, (unsigned long)epsilon,
         (unsigned long)(gamma * epsilon));

  // Go through every digit position.
  // Eliminate each number whose criterion doesn't match.
  // After an arrival, if this arrival was last or if this
  // leaves only one digit to be traveled, then return
  // (the first) of them.

  // 1 if traversable; 0 if not.
  int *oxy_traversable = malloc(sizeof(int) * numbers->len);
  int *co2_traversable = malloc(sizeof(int) * numbers->len);
  memset(oxy_traversable, true, sizeof(int) * numbers->len);
  memset(co2_traversable, true, sizeof(int) * numbers->len);
  int last_oxy_traversed = -1;
  int last_co2_traversed = -1;
  int n_oxy_traversed = 0;
  int n_co2_traversed = 0;
  for (int d = 0; d < NBITS; d++) {
    n_oxy_traversed = 0;
    for (int i = 0; i < numbers->len; i++) {
    }
  }

  freebstats(bit_stats);
  freeintvec(numbers);
  fclose(file);
  return 0;
}
