#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include "roundoff.h"

void initialize_roundoff(){
    printf("underflow sate : %d\n", fetestexcept(FE_UNDERFLOW));
    printf("overflow state : %d\n", fetestexcept(FE_OVERFLOW));
}

int roundoff_ok(){
    printf("underflow sate : %d\n", fetestexcept(FE_UNDERFLOW));
    printf("overflow state : %d\n", fetestexcept(FE_OVERFLOW));
    if (fetestexcept(FE_UNDERFLOW) != 0 || fetestexcept(FE_OVERFLOW) != 0) return 0;
    return 1;
}
