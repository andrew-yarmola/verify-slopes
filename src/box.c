#include <stdlib.h>
#include "box.h"

double scale[6];
static bool scale_initialized = false; 

void compute_center_and_size(Box& box)
{
  for (int i = 0; i < 6; ++i) {
    // GMT paper page 419 of Annals
    // size guarantees that :
    // center - size <= true_center - true_size
    // center + size >= true_center + true_size
    // where box operations are floating point. 
    box.center[i] = scale[i] * box.center_digits[i];
    box.size[i]= (1 + 2 * EPS) * (box.size_digits[i] * scale[i] + HALFEPS * fabs(box.center_digits[i]));
  }
}

void compute_cover(Box& box)
{
  box.cover.lattice = ACJ(
      XComplex(box.center[3], box.center[0]),
      XComplex(box.size[3], box.size[0]),
      0.,
      0.
      );
  box.cover.loxodromic_sqrt = ACJ(
      XComplex(box.center[4], box.center[1]),
      0.,
      XComplex(box.size[4], box.size[1]),
      0.
      );
  box.cover.parabolic = ACJ(
      XComplex(box.center[5], box.center[2]),
      0.,
      0.,
      XComplex(box.size[5], box.size[2])
      );
}

void compute_nearer(Box& box)
{
  double m[6];
  for (int i = 0; i < 6; ++i) {
    m[i] = 0; // inconclusive cases
    if (box.center_digits[i] > 0 && // center is positive 
        box.center_digits[i] > box.size_digits[i] &&  // true diff is positive
        box.center[i]        > box.size[i]) { // machine diff is >= 0
      // Want lower bound on true_center - true_size.  Assume no overflow or underflow 
      // Note, sign(center_digits) == sign(center), unless center == 0. Also, size is always >= 0. 
      // GMT paper page 419 of Annals gives with true arithmetic
      //      center - size <= true_center - true_size
      // Now, in machine arthimetric, by IEEE, if 
      //      center > size then center (-) size >= 0.
      // Lemma 7.0 gives,
      //      (1-EPS)(*)( center (-) size ) <= center - size <= true_center - size. 
      m[i] = (1 - EPS) * (box.center[i] - box.size[i]);
    } else if (box.center_digits[i] < 0 && // center is negative
        box.center_digits[i] < -box.size_digits[i] && // true sum is negative
        box.center[i]        < -box.size[i]) {  // machine sum is negative
      // Want upper bound on true_center - true_size.  Assume no overflow or underflow
      // Note, sign(center_digits) == sign(center), unless center == 0. Also, size is always >= 0. 
      // GMT paper page 419 of Annals gives with true arithmetic
      //      true_center + true_size <= center + size
      // Now, in machine arthimetric, by IEEE, if 
      //      -center > size then (-center) (-) size >= 0.
      // Lemma 7.0 gives,
      //      (1-EPS)(*)( (-center) (-) size ) <= -center - size <= -true_center - true_size.
      // So,
      //      -((1-EPS)(*)( (-center) (-) size )) >= true_center + true_size.
      // Note, negation is exact for machine numbers
      m[i] = -(( 1 - EPS) * ((-box.center[i]) - box.size[i]));
    }
  }

  box.nearer.lattice = XComplex(m[3], m[0]);
  box.nearer.loxodromic_sqrt = XComplex(m[4], m[1]);
  box.nearer.parabolic = XComplex(m[5], m[2]);
}

void compute_further(Box& box)
{
  double m[6];
  for (int i = 0; i < 6; ++i) {
    m[i] = 0; // inconclusive cases
    if (box.center_digits[i] > -box.size_digits[i]) { // true sum is positive 
      // Want upper bound of true_center + true_size. Assume no overflow or underflow
      // Note, sign(center_digits) == sign(center), unless center == 0. Also, size is always >= 0. 
      // GMT paper page 419 of Annals gives with true arithmetic
      //      true_center + true_size <= center + size
      // By IEEE (+) and (-) resepct <= and >=, so center (+) size >=0 and
      // Lemma 7.0 for floating point arithmetic gives and upper bound
      //      (1+EPS)(*)(center (+) size) >= center + size >= true_center + true_size
      m[i] = (1 + EPS) * (box.center[i] + box.size[i]);
    } else { // true sum is <= 0
      // Want lower bound of true_center - true_size. Assume no overflow or underflow
      // Note, sign(center_digits) == sign(center), unless center == 0 
      // GMT paper page 419 of Annals gives with true arithmetic
      //      center - size <= true_center - true_size
      // By IEEE, (+) and (-) respects <= and >=, and negation is exact.
      // Thus, (-center) (+) size >=0 and Lemma 7.0 for floating point arithmetic gives
      //        (1+EPS)(*)( (-center) (+) size) ) >= (-center) + size
      // So,
      //      -((1+EPS)(*)( (-center) (+) size) ))<= center - size <= true_center - true_size
      m[i] = -((1 + EPS) * ((-box.center[i]) + box.size[i]));
    }
  }

  box.further.lattice = XComplex(m[3], m[0]);
  box.further.loxodromic_sqrt = XComplex(m[4], m[1]);
  box.further.parabolic = XComplex(m[5], m[2]);
}

void compute_greater(Box& box)
{
  double m[6];
  for (int i = 0; i < 6; ++i) {
    m[i] = 0; // inconclusive cases
    if (box.center_digits[i] > -box.size_digits[i]) { // true sum is positive
      // Want upper bound of true_center + true_size. Assume no overflow or underflow
      // Note, sign(center_digits) == sign(center), unless center == 0. Also, size is always >= 0. 
      // GMT paper page 419 of Annals gives with true arithmetic
      //      true_center + true_size <= center + size.
      // Notice that center + size >= true_center + true_size > 0.
      // By IEEE, center (+) size >=0, as it's guanrateed to evaluate to nearest representable.
      // Lemma 7.0 for floating point arithmetic gives and upper bound
      //      (1+EPS)(*)(center (+) size) >= center + size >= true_center + true_size
      m[i] = (1 + EPS) * (box.center[i] + box.size[i]);
    } else if (box.center_digits[i] < -box.size_digits[i] && // true sum is negative
        box.center[i]        < -box.size[i]) { // machine sum is <= 0
      // Want upper bound of true_center + true_size. Assume no overflow or underflow
      // Note, sign(center_digits) == sign(center), unless center == 0. Also, size is always >= 0. 
      // GMT paper page 419 of Annals gives with true arithmetic
      //      true_center + true_size <= center + size.
      // Notice that center + size < 0.
      // By IEEE, center (+) size <= 0, as it's guanrateed to evaluate to nearest representable.
      // Lemma 7.0 for floating point arithmetic gives a bound
      //      (1-EPS)(*)| center (+) size | < | center + size |
      // So,
      //      -((1-EPS)(*)(-(center (+) size))) >= center + size >= true_center + true_size
      m[i] = -((1 - EPS) * (-(box.center[i] + box.size[i])));
    }
  }

  box.greater.lattice = XComplex(m[3], m[0]);
  box.greater.loxodromic_sqrt = XComplex(m[4], m[1]);
  box.greater.parabolic = XComplex(m[5], m[2]);
}

Box build_box(char* where) {
  Box box;
  // Global scaling of boxes - runs once
  if (!scale_initialized) {
    for (int i = 0; i < 6; ++i) {
      scale[i] = pow(2, -i / 6.0);
    }
    scale_initialized = true;
  } 
  for (int i = 0; i < 6; ++i) {
    box.center_digits[i] = 0;
    box.size_digits[i] = 8;
  }
  size_t pos = 0;
  size_t idx = 0;
  int dir;
  while (where[idx] != '\0') {
    if (where[idx] == '0') {
      dir = 0;
    } else if (where[idx] == '1') {
      dir = 1;
    } else {
      fprintf(stderr, "Fatal: boxcode is invalid %s\n", where);
      exit(6);
    }
    box.size_digits[pos] *= 0.5;
    box.center_digits[pos] += (2 * dir - 1) * box.size_digits[pos];
    ++pos;
    if (pos == 6) {
      pos = 0;
    }
    ++idx;
  }
  compute_center_and_size(box);
  compute_cover(box);
  compute_nearer(box);
  compute_further(box);
  compute_greater(box);
  return box;    
}

