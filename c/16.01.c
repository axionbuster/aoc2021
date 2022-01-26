// Packet recognition

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/charvec.h"
#include "lib/dbgprint.h"
#include "lib/xalloc.h"

// Parsing

typedef enum _parse_stat { FINE = 0, NOGO = 1, STOP = 2 } ParseStat;

ParseStat parse_error = FINE;

#define REMLEN(cv, i) ((cv).len - (i))

static int accept_version(CharVec *cv, int *index) {
  int save = *index;
  if (parse_error || REMLEN(*cv, *index) < 3)
    goto all_fail;
  char temp[4] = {cv->xs[*index], cv->xs[*index + 1], cv->xs[*index + 2], 0};
  errno = 0;
  long ver = strtol(temp, NULL, 2);
  if (errno) {
    goto all_fail;
  }
  *index += 3;
  printf("VVV");
  return ver;
all_fail:
  parse_error = NOGO;
  *index = save;
  return ver;
}

static int accept_tag(CharVec *cv, int *index) {
  return accept_version(cv, index);
}

static void accept_literal(CharVec *cv, int *index, char out[5]) {
  int save = *index;
  if (parse_error || REMLEN(*cv, *index) < 5)
    goto all_fail;
  char temp[5];
  for (int i = 0; i < 5; i++) {
    temp[i] = cv->xs[i + *index];
  }
  parse_error = temp[0] == '1' ? FINE : STOP;
  memcpy(out, temp + 1, 4);
  *index += 5;
  printf("AAAAA");
  return;
all_fail:
  parse_error = NOGO;
  *index = save;
  return;
}

static CharVec *accept_literal_string(CharVec *cv, int *index) {
  if (parse_error)
    goto fail_all;
  int save = *index;
  CharVec *temp_cv = xmkcharvec();
  while (!parse_error) {
    char temp[5];
    accept_literal(cv, index, temp);
    for (int i = 0; i < 4; i++) {
      xinscharvec(temp_cv, temp[i]);
    }
    if (parse_error == NOGO) {
      goto fail_all;
    }
  }
  return temp_cv;
fail_all:
  if (cv)
    freecharvec(cv);
  *index = save;
  return NULL;
}

// Inputs

#define BIT(l, b) (!!((l) & (1ul << (b))))

int main(void) {
  // Read in the input
  CharVec *cv = xmkcharvec();
  char *line = NULL;
  size_t cap = 0;
  ssize_t read = getline(&line, &cap, stdin);
  for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++) {
    char const c[2] = {line[i], 0};
    long l = strtol(c, NULL, 16);
    xinscharvec(cv, BIT(l, 3) + '0');
    xinscharvec(cv, BIT(l, 2) + '0');
    xinscharvec(cv, BIT(l, 1) + '0');
    xinscharvec(cv, BIT(l, 0) + '0');
  }
  xinscharvec(cv, '\0');
  free(line);
  printf("%s\n", cv->xs);
  int index = 0;
  accept_version(cv, &index);
  accept_tag(cv, &index);
  CharVec *lits = accept_literal_string(cv, &index);
  printf("\n");
  freecharvec(lits);
  freecharvec(cv);
}