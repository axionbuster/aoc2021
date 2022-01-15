// dblhash.c -- YuJin Kim (c) 2022
// AAR.
//
// Prototype for polymorphism using a union structure.
// The data structure for a "double hash" allows two-way associations
// to be made between two types of keys.
// The data structure is divided into two states.
//
// The initial or "liquid" stage saves these associations in a cheap way.
//
// The usable, "frozen" or "readonly" stage maps these associations such that
// it only takes O(1) time and memory to look up an already stored key
// from either way.
//
// The purpose of this C source file is to test the usability of an approach to
// implement the polymorphism.

#include <stdbool.h>
#include <stdio.h>

typedef struct _dblhash_t {
  // The first value "frozen" indicates the type.
  bool frozen;
  // The following are one of the following modes.
  union {
    struct _dbhash_liquid_t {
      char *message;
      int haha;
    } liquid;
    struct _dblhash_frozen_t {
      char *message;
      double hoho;
    } frozen;
  } body;
} DoubleHash;

void initdblhash(DoubleHash *d) {
  d->frozen = false;
  d->body.liquid.message = "hi";
  d->body.liquid.haha = 123456;
}

void freezedblhash(DoubleHash *d) {
  if (d->frozen) {
    fprintf(stderr, "WARNING: freezedblhash -> attempt to freeze an already "
                    "frozen structure.\n");
    return;
  }
  d->frozen = true;
  struct {
    char *m;
    double d;
  } temp = {.m = d->body.liquid.message, .d = d->body.liquid.haha};
  d->body.frozen.message = temp.m;
  d->body.frozen.hoho = temp.d;
}

void printdblhash(DoubleHash *d) {
  printf("DoubleHash { frozen? %s, body { message = \"%s\", %s = ",
         d->frozen ? "yes" : "no",
         d->frozen ? d->body.frozen.message : d->body.liquid.message,
         d->frozen ? "hoho" : "haha");
  if (d->frozen) {
    printf("%lf } }", d->body.frozen.hoho);
  } else {
    printf("%d } }", d->body.liquid.haha);
  }
}

int main(void) {
  DoubleHash dh;
  initdblhash(&dh);

  printf("Before freezing:\n");
  printdblhash(&dh);
  printf("\nAfter freezing:\n");
  freezedblhash(&dh);
  printdblhash(&dh);
  printf("\nAttempt to freeze again:\n");
  fflush(stdout);
  freezedblhash(&dh);
  fflush(stderr);
  printdblhash(&dh);
  printf("\n");

  return 0;
}