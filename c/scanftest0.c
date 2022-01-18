#include <stdio.h>
#include <string.h>

int main(void) {
  char e1[64] = {0};
  char e2[64] = {0};

  for (int scan = 0; (scan = scanf("%[^\n -] - %[^\n -] ", e1, e2)) != EOF;) {
    printf("Recognize: bridge %s (%zu characters) with %s (%zu characters).\n",
           e1, strlen(e1), e2, strlen(e2));
  }

  return 0;
}
