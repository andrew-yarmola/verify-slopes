#include "SL2ACJ.h"

typedef struct {
  int pos = 0;
  // Initial disk: center at (0,0) and complex radius 8 * 2^(-1/2) + 8 * 2^0 i
  // See compute_center_and_radius()
	double center_digits[2] = {0, 0};
	double size_digits[2] = {8, 8};
  double center[2];
  double size[2];
	ACJ c; // center
	ACJ r; // radius
} Rect;

Rect initial_lattice_cover();
Rect child(const Rect& disk, int dir);

// Initial disk box dimensions are 2^(6/2), 2^(5/2) 

double disk_scale[2] = {
  pow(2., -0. / 2.),
  pow(2., -1. / 2.)
};

void compute_center_and_radius(Rect& disk) {
	for (int i = 0; i < 2; ++i) {
    // GMT paper page 419 of Annals
    // disk_size guarantees that :
    // disk_center - disk_size <= true_center - true_size
    // disk_center + disk_size >= true_center + true_size
    // where box operations are floating point. 
    disk.center[i] = disk_scale[i] * disk.center_digits[i];
    disk.size[i] = (1 + 2 * EPS) * (disk.size_digits[i] * disk_scale[i] +
        HALFEPS * fabs(disk.center_digits[i]));
  }
  // Notice that the center and radius of a disk are constant and do not
  // depend on (z1, z2, z3). In particular, this is used like interval arithmetic.
  disk.c = ACJ(XComplex(disk.center[1], disk.center[0]), 0., 0., 0.);
  disk.r = ACJ(XComplex(disk.size[1],   disk.size[0]),   0., 0., 0.);
}

Rect initial_lattice_cover() {
  Rect disk;
  compute_center_and_radius(disk);
  return disk;
}

Rect child(const Rect& disk, int dir) {
	Rect child(disk);
	child.size_digits[child.pos] *= 0.5;
	child.center_digits[child.pos] += (2 * dir - 1) * child.size_digits[child.pos];
	++child.pos;
	if (child.pos == 2) { child.pos = 0; }
  compute_center_and_radius(child);
	return child;
}

void desc(Rect& disk) {
  printf("center: %f + %f i, rad: %f\n", disk.c.f.re, disk.c.f.im, absUB(disk.r));
}

struct Queue {
  Rect* rect;
  struct Queue* next;
};

typedef struct Queue Queue;


const char *words[] = {
  "test",
  "of",
  "words"
};


int main() {
  Rect large = initial_lattice_cover();
  Queue head = {&large, NULL};
  Rect child0 = child(*head.rect, 0);
  Rect child1 = child(*head.rect, 1);
  Rect child00 = child(child0, 0);
  Rect child11 = child(child1, 1);
  desc(*head.rect);
  desc(child0);
  desc(child1);
  desc(child00);
  desc(child11);
  int length = sizeof(words) / sizeof(char*);
  for (int i = 0; i < length; i++) {
    printf("%s\n", words[i]);
  } 
}

