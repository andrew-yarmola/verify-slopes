#ifndef _box_h_
#define _box_h_
#include "SL2ACJ.h"

typedef struct {
  XComplex lattice;
  XComplex loxodromic_sqrt;
  XComplex parabolic;
} XParams;

typedef struct {
  ACJ lattice;
  ACJ loxodromic_sqrt;
  ACJ parabolic;
} ACJParams;

typedef struct {
  // Initial box:
  //    [-8 * 2^0, 8 * 2^0] x [-8 * 2^(-1/6), 8 * 2^(-1/6)] x
  //    [-8 * 2^(-2/6), 8 * 2^(-2/6)] x [-8 * 2^(-3/6), 8 * 2^(-3/6)]
  //    [-8 * 2^(-4/6), 8 * 2^(-4/6)] x [-8 * 2^(-5/6), 8 * 2^(-5/6)]
  // Essentially, center: (0,0,0,0,0,0) and in dimension j we take all
  //    points with |x_j - c_j| <= 8 * 2^(-j/6) where
  //        c_j is the jth center coord
  //        x_j is the point in jth dimenson to include
  // See compute_center_and_size() 
  double center_digits[6] = {0, 0, 0, 0, 0, 0};
  double size_digits[6] = {8, 8, 8, 8, 8, 8};
  double center[6];
  double size[6];
  ACJParams cover;
  XParams nearer; // all values closer to 0 than in box or 0 if box overlaps
  XParams further; // all values futher from 0 that in the box
  XParams greater; // all values greater than in the box
} Box;

typedef struct {
  // Initial disk:
  //    center at (0,0) and complex radius 8 * 2^(-1/2) + 8 * 2^0 i
  // See compute_center_and_radius()
	double center_digits[2] = {0, 0};
	double size_digits[2] = {8, 8};
  double center[2];
  double size[2];
  int pos = 0;
	ACJ c; // center
	ACJ r; // radius
} Rect;

Box build_box(char* where);
Rect initial_lattice_cover();
Rect child(const Rect& disk, int dir);

#endif // _box_h_
