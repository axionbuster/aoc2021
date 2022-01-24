#include <inttypes.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

int main(void) {
  int larger = 0;

  int prev = INT32_MIN;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, stdin)) != -1) {
    int curr = atoi(line);
    if (prev < curr) {
      larger++;
    }
    prev = curr;
  }
  free(line);
  if (read == -1) {
    if (!feof(stdin)) {
      perror("getline");
      exit(EXIT_FAILURE);
    }
  }
  larger--;
  if (larger < 0) {
    larger = 0;
  }
  printf("%d\n", larger);
  return 0;
}
