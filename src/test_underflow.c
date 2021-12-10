#include <stdio.h>
#include <stdlib.h>
#include "roundoff.h"

int main() {
  initialize_roundoff();
  double small = DBL_MIN;
  printf("Attempting to trigger underflow detection.\n");
  printf("This test may abort after this line. This would be acceptable behavior.\n");
  fflush(stdout);
  small = small*small;
  if (!roundoff_ok()) {
    printf("Underflow correctly detected.\n");
    exit(0);
  } else {
    printf("Error --  underflow has occurred, but was not detected!\n");
    printf("small = %g\n", small);
  }
  exit(-1);
}

