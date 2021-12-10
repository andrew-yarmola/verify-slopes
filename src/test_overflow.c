#include <stdio.h>
#include <stdlib.h>
#include "roundoff.h"

int main() {
  initialize_roundoff();
  double large = DBL_MAX;
  printf("Attempting to trigger overflow detection.\n");
  printf("This test may abort after this line. This would be acceptable behavior.\n");
  fflush(stdout);
  large = large*large;
  if (!roundoff_ok()) {
    printf("Overflow correctly detected.\n");
    exit(0);
  } else {
    printf("Error --  overflow has occurred, but was not detected!\n");
    printf("large = %g\n", large);
  }
  exit(-1);
}

