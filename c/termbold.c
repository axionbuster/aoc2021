#include <stdbool.h>
#include <stdio.h>

// Thanks to the following documentation:
// https://misc.flogisoft.com/bash/tip_colors_and_formatting

#define TEXTBOLD "\e[1m"
#define TEXTNORMAL "\e[0m"

void print_zerobold(char const *__restrict__ text) {
  bool bold = false;

  while (*text != '\0') {
    if (*text == '0') {
      bold = true;
      printf("%s", TEXTBOLD);
    } else if (bold) {
      bold = false;
      printf("%s", TEXTNORMAL);
    }
    printf("%c", *text);
    text++;
  }
}

int main(void) {
  printf("%s\n", "I am " TEXTBOLD "BOLD!" TEXTNORMAL);
  printf("%s\n", "0000" TEXTBOLD "0000" TEXTNORMAL "0000");
  printf("\n\n");

  print_zerobold("16391134\n");
  print_zerobold("11700010100111\n");
}