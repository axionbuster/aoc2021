// Packet recognition

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/dbgprint.h"
#include "lib/xalloc.h"

// Basic data structures

typedef struct _octet_v_t {
  uint8_t *octets;
  int len, cap;
} Octets;

static void insert_octet(Octets *o, uint8_t octet) {
  if (o->len == o->cap) {
    o->octets = xrealloc(o->octets, o->cap *= 2);
  }
  o->octets[o->len++] = octet;
}

static uint8_t to_nib(char c) {
  // The ASCII codes for digits and any alphabetical characters are
  // discontinuous.
  if ('A' <= c && c <= 'F') {
    return c - 'A' + 0xA;
  } else if ('0' <= c && c <= '9') {
    return c - '0';
  } else {
#ifndef NDEBUG
    dbgprintf("fatal: character %c is not upper-case hexadecimal.\n", c);
    abort();
#endif
  }
}

// Specialized version of isxdigit for our purposes
static bool isxdigit1(int c) {
  return ('A' <= c && c <= 'F') || ('0' <= c && c <= '9');
}

#define higher(o) ((uint8_t)(o) << 4)

// We actually handle bits.

typedef struct _parse_t {
  // (str.len / 8) = oct_q ... oct_r;
  // oct_q serves as the octet index; oct_r as the index within that octet.
  int oct_q, oct_r;
} ParseMachine;

// Slice an octet (little endian)
static uint8_t lend8sl(ParseMachine *mac, Octets *str, unsigned n_bits) {
  assert(n_bits > 0);
  assert(n_bits < 8);
  // If oct_r + n_bits go 8 or above, we have hit the octet boundary.
  bool oob = (n_bits + mac->oct_r) > 8;

  if (oob) {
    // TODO: Go above boundary.
  } else {
    // No go above boundary.

    // Get (8 - n_bits) shift right by supposing mac->oct_r == 0.
    // Then, if mac->oct_r > 0, notice that the amount of right-shifts
    // decreases.
    unsigned shift = 8 - n_bits - mac->oct_r;

    uint8_t raw = str->octets[mac->oct_q];
    raw >>= shift;
    return raw;
  }
}

int main(void) {
  // Packets are represented as a string of octets.
  Octets str = {.len = 0, .cap = 1, .octets = xcalloc(1, sizeof(uint8_t))};

  // The input stream is a hexadecimal string of characters ('0'-'F').
  // Each digit therefore represents half an octet.
  // We must reconstruct an octet at every other character.
  // If we stop reading at an incomplete reading, we must return
  //  [MS_NIB, 0000]
  // where MS_NIB represents the "most significant nibble."
  // Yes, the protocol is based on big-endian readings.
  bool most_sig = true;
  char c;
  uint8_t octet = 0;
  while (isxdigit1(c = getchar())) {
    if (most_sig) {
      most_sig = false;
      octet |= higher(to_nib(c));
    } else {
      most_sig = true;
      octet |= to_nib(c);
      insert_octet(&str, octet);

      // It's important to make sure that the least significant
      // nibble is 0 whenever the whole octet is reset.
      octet = 0;
    }
  }
  if (!most_sig) {
    insert_octet(&str, octet);
  }

  // DEBUG: Print it
  dbgprintf("Received %d octets\n", str.len);
  for (int i = 0; i < str.len; i++) {
    if (i % 16 == 0) {
      dbgprintf("[%3d-%3d]: ", i, i + 16);
    }
    dbgprintf("%02X ", str.octets[i]);
    if (i % 16 == 15) {
      dbgprintf("\n");
    }
  }
  dbgprintf("\n");

  free(str.octets);
}