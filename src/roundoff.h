#ifndef _roundoff_h_
#define _roundoff_h_
#include <float.h>
#include <math.h>

#define EPS DBL_EPSILON
#define HALFEPS (EPS/2)

inline double infinity(){ return 1.0/0.0; }

void initialize_roundoff();

int roundoff_ok();

#endif // _roundoff_h_
