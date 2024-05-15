#include <stdlib.h>

int rand_in_range(int min, int max) {
  if (min == max) {
    return 0;
  }
  return rand() % (max - min) + min;
}
