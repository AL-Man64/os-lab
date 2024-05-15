#include "utils.h"
#include <stdlib.h>

int rand_in_range(int min, int max) { return rand() % (max - min) + min; }
