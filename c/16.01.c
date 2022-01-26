// Packet recognition

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/dbgprint.h"
#include "lib/iminmax.h"
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

// We handle the input bit-by-bit.

typedef struct _parse_t {
  // (str.len / 8) = oct_q ... oct_r;
  // oct_q serves as the octet index; oct_r as the index within that octet.
  int oct_q, oct_r;
} Bitdex;

static void increment_bitdex(Bitdex *b) {
  if ((b->oct_r += 1) == 8) {
    b->oct_r = 0;
    b->oct_q += 1;
  }
}

typedef enum {
  FINE = 0,
  NOGO = 1,

  // This is used to delineate the "end" of something, usually end of packet.
  STOP = 2
} ParseCondition;

// This is a global variable that is 0 when no error occurs and not 0 when
// anything bad has ever occurred. Code cannot set this to 0 (only the user
// can).
ParseCondition parse_error;

// Used for debug printing only.
bool suppress_error_print = false;

// Returns 0 if Octets (o) at bit (b) is 0; 1 otherwise.
static uint8_t get_bit(Octets *o, Bitdex *b) {
  assert(b->oct_q >= 0);
  if (b->oct_q >= o->len) {
#ifndef NDEBUG
    if (!suppress_error_print)
      dbgprintf("parse_error at get_bit: range oob.\n");
#endif
    parse_error = NOGO;
    return 0;
  }
  uint8_t octet = o->octets[b->oct_q];

  // The correction 8 - (bit) - 1 had to be made since
  // the bits are in LE on this machine.
  uint8_t mask = 1 << (8 - b->oct_r - 1);

  increment_bitdex(b);

  return !!(octet & mask);
}

static uint8_t accept_version(Octets *o, Bitdex *b) {
  uint8_t bits[3] = {0};

  bits[0] = get_bit(o, b);
  bits[1] = get_bit(o, b);
  bits[2] = get_bit(o, b);

  if (parse_error == FINE) {
    uint8_t version = (bits[0] << 2) | (bits[1] << 1) | (bits[2]);
    return version;
  }

  return 0;
}

static uint8_t accept_id(Octets *o, Bitdex *b) {
  // Yeah, the same code is used to parse packet IDs as well as the versions.
  // Coincidence (see specs).
  return accept_version(o, b);
}

static uint8_t accept_nibble(Octets *o, Bitdex *b) {
  uint8_t bits[5] = {0};
  for (int i = 0; i < 5; i++) {
    bits[i] = get_bit(o, b);
  }
  if (!parse_error) {
    if (bits[0] == 0) {
      parse_error = STOP;
    }
    return (bits[1] << 3) | (bits[2] << 2) | (bits[3] << 1) | bits[4];
  }
  return 0;
}

static void discard_bits(Bitdex *b, int n) {
  for (int i = 0; i < n; i++) {
    increment_bitdex(b);
  }
}

// Accept and return the literal number.
// Most significant bits get pushed first (lower index).
static Octets accept_literal(Octets *o, Bitdex *b) {
  Octets literal = {0};

  // The number of bits in the packet are rounded up to the smallest 4*n.
  long bits_read = 0;

  if (parse_error != FINE) {
    return (Octets){0};
  }

  literal.cap = 1;
  literal.octets = xcalloc(1, sizeof(uint8_t));

  bool most_sig = true;
  uint8_t c = 0;

  for (;;) {
    uint8_t nib = accept_nibble(o, b);
    if (most_sig) {
      most_sig = false;
      c |= higher(nib);
    } else {
      most_sig = true;
      c |= nib;
      insert_octet(&literal, c);
      c = 0;
    }
    if (parse_error == STOP) {
      break;
    }
  }
  if (!most_sig) {
    insert_octet(&literal, c);

    // Correction for nibble misalignment
    for (int i = literal.len - 1; i > 0; i--) {
      literal.octets[i] >>= 4;
      literal.octets[i] |= (literal.octets[i - 1]) << 4;
    }
    literal.octets[0] >>= 4;
  }

  assert(parse_error != FINE);
  if (parse_error == NOGO) {
    free(literal.octets);
    return (Octets){0};
  } else {
    return literal;
  }
}

// Show an octet

static int clamp(int x, int up, int low) {
  if (x > up) {
    return up;
  } else if (x < low) {
    return low;
  }
  return x;
}

#ifndef NDEBUG
static void print_octets(Octets *o) {
  // I understand that this code is unreadable.
  // However it works and it's used for debugging.

  // Print the binary and the decimal representations of a potentially
  // very long series of octets, while supporting truncation at the
  // most significant 8 octets / 64 bits (lower indices).

  // Example:
  //  Octet [binary:
  //  0100001000001101011101001100001100001000100000000100001100111001... ]
  //   [decimal: (upper 64-bits BE) 4759588762210550585] (675 octets)
  int parse_error_save = parse_error;
  parse_error = FINE;
  suppress_error_print = true;
  Bitdex index = {0};
  uint8_t bit = 0;
  long n_read = 0;
  dbgprintf("Octet [binary: ");
  while ((bit = get_bit(o, &index)), !parse_error && n_read < 64) {
    if (bit)
      dbgprintf("1");
    else
      dbgprintf("0");
    n_read++;
  }
  if (!parse_error) {
    dbgprintf("... ");
  }
  dbgprintf("] [decimal: (upper 64-bits BE) ");
  bool over64bits = o->len > 8 ? !!o->octets[8] : false;
  uint64_t low64 = 0;
  for (int i = 0; i < tg_min(8, o->len); i++) {
    low64 |= (uint64_t)o->octets[i] << ((8 - i - 1) * 8);
  }
  int diff = tg_max(8, o->len) - tg_min(8, o->len);
  int n_shift = clamp(diff * 8, 63, 0);
  if (n_shift != 63) {
    dbgprintf("%lu] ", low64 >> n_shift);
  } else {
    dbgprintf("%lu] ", low64);
  }
  dbgprintf("(%d octets)\n", o->len);
  suppress_error_print = false;
  parse_error = parse_error_save;
}

static char const *id_type_str(int id) {
  switch (id) {
  case 4:
    return "literal";
  default:
    return "unknown";
  }
}
#else
#define print_octets(...)
#define id_type_str(...) NULL
#endif

typedef struct _machine_t {
  // states
} Machine;

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

  // Parse stuff
  uint8_t version, id;
  Bitdex b = {0};

  version = accept_version(&str, &b);
  id = accept_id(&str, &b);

  if (parse_error) {
    fprintf(stderr, "parse error\n");
    abort();
  } else {
    dbgprintf("Version %d, packet id %d (%s)\n", (int)version, (int)id,
              id_type_str(id));
  }

  Octets literal = accept_literal(&str, &b);
  if (parse_error != STOP) {
    fprintf(stderr, "parse_error status %d unexpected\n", parse_error);
    abort();
  } else {
    dbgprintf("Literal accepted\n");
    print_octets(&literal);
    free(literal.octets);
  }

  free(str.octets);
}