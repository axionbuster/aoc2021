#include <stdio.h>

int main(void) {
  for (int i = 16; i < 256; i++) {
    printf("\e[48;5;%dm%%03d", i);
    printf("\e[0m");
    if ((i - 15) % 6) {
      printf("\n");
    } else {
      printf(" ");
    }
  }
}