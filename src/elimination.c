#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue>
#include <vector>

#include "elimination.h"
#include "e2words.h"

extern double g_max_area;
extern double g_min_e2_area;

// Helper functions

// If inequalities is false, crash the program
void check(bool inequalities, char* where)
{
    if (!inequalities) {
        fprintf(stderr, "Fatal: verify error at %s\n", where);
        exit(3);
    }
}

// Replaces contents of code with the parsed word
void parse_word(char* code)
{
    char buf[MAX_CODE_LEN];
    strncpy(buf, code, MAX_CODE_LEN);
    char * start = strchr(buf,'(');
    char * end = strchr(buf,')');
    size_t len = end - start - 1;
    strncpy(code, start+1, len);
    code[len] = '\0'; 
}

// Returns a word pair encoded in code
word_pair get_word_pair(char* code)
{
    word_pair pair;
    char * start = strchr(code,'(');
    char * mid = strchr(code,',');
    char * end = strchr(code,')');
    size_t first_len = mid - start - 1;
    size_t second_len = end - mid - 1;
    strncpy(pair.first, start+1, first_len);
    strncpy(pair.second, mid+1, second_len);
    pair.first[first_len] = '\0';
    pair.second[second_len] = '\0';
    return pair;
}

// Gives number of g and G in a word
int g_length(char* word)
{
    int g_len = 0;
    char* c = word;
    while (*c != '\0') {
      if (*c == 'g' || *c == 'G') {
        ++g_len;
      }
      ++c;
    }
    return g_len;
}

// Elimination Tools

// Returns the G SLACJ matrix
SL2ACJ construct_G(const ACJParams& p)
{
  ACJ i(XComplex(0, 1));
  const ACJ& P = p.parabolic;
  const ACJ& S = p.loxodromic_sqrt;
  return SL2ACJ((i * P) * S, i / S, i * S, ACJ(0));
}

// Lattice translation parameter: z -> z + T
ACJ construct_T(const ACJParams& p, int M, int N)
{
  // Note, int -> double conversion is exact
  return p.lattice * double(N) + double(M);
}

// SL2ACJ matrix constrution from parameters and a word.
// Note, floating point arithmetic is not commutative and so neither is ACJ.
// We have chosen a right to left order of multiplication for validation to pass.
// Different orders of multiplication produce slightly different rounding
// errors. Since our data is greedy, this order is necessary for all words.
SL2ACJ construct_word(const ACJParams& p, const char* word)
{
  ACJ one(1), zero(0);
  SL2ACJ w(one, zero, zero, one);
  SL2ACJ G(construct_G(p));
  SL2ACJ g(inverse(G));

  int M = 0;
  int N = 0;
  ACJ T;
  size_t pos;
  for (pos = strlen(word); pos > 0; --pos) {
    char h = word[pos-1];
    switch(h) {
      case 'm': --M; break;
      case 'M': ++M; break;
      case 'n': --N; break;
      case 'N': ++N; break;
      default: {
                 if (M != 0 || N != 0) {
                   T = construct_T(p, M, N);
                   w = SL2ACJ(w.a + T * w.c, w.b + T * w.d, w.c, w.d);
                   M = N = 0;
                 }
                 if (h == 'g')
                   w = g * w;
                 else if (h == 'G')
                   w = G * w;
               }
    }
  }
  if (M != 0 || N != 0) {
    T = construct_T(p, M, N);
    w = SL2ACJ(w.a + T * w.c, w.b + T * w.d, w.c, w.d);
  }
  return w;
}

// Cusp area lower bound
inline const double areaLB(const XParams& nearer, char* where)
{
    // Area is |lox_sqrt|^2*|Im(lattice)|.
    XComplex lox_sqrt = nearer.loxodromic_sqrt;
    double lat_im     = nearer.lattice.im;
    if (lat_im  < 0) { // this should never happen
        fprintf(stderr,
            "Fatal: lattice imaginary part is negative at %s\n", where);
        exit(5);
    }
    // Apply Lemma 7.0 of GMT.
    double lox_re = (1-EPS)*(lox_sqrt.re*lox_sqrt.re);
    double lox_im = (1-EPS)*(lox_sqrt.im*lox_sqrt.im);
    double lox_norm = (1-EPS)*(lox_re + lox_im);
    return (1-EPS)*(lox_norm*lat_im);
}

// Our compact parameter space has the following bounds:
// 0. |lox_sqrt| >= 1
// 1.
//    a. Im(lox_sqrt) >= 0
//    b. Im(lattice) >= 0
//    c. Im(parabolic) >= 0
//    d. Re(parabolic) >= 0
// 2. -1/2 <= Re(lattice) <= 1/2
// 3. |lattice| >= 1
// 4. Im(parabolic) <= Im(lattice)/2
// 5. Re(parabolic) <= 1/2
// 6. |lox_sqrt^2| Im(L) <= g_max_area (area of fundamental paralleogram)
void verify_out_of_bounds(char* where, char bounds_code)
{
    Box box = build_box(where);
    switch(bounds_code) {
        case '0': {
            check(absUB(box.further.loxodromic_sqrt) < 1, where);
            break; }
        case '1': {
            check(box.greater.loxodromic_sqrt.im < 0
             || box.greater.lattice.im < 0
             || box.greater.parabolic.im < 0
             || box.greater.parabolic.re < 0, where);
            break; }
        case '2': {
            check(fabs(box.nearer.lattice.re) > 0.5, where);
            break; }
        case '3': {
            check(absUB(box.further.lattice) < 1, where);
            break; }
        case '4': {
            // Note: we can exclude the box if and only if the parabolic
            // imag part is bigger than half the lattice imag part over 
            // the entire box
            // Multiplication by 0.5 is EXACT (if no underflow or overflow)
            check(box.nearer.parabolic.im > 0.5 * box.further.lattice.im, where);
            break; }
        case '5': {
            check(box.nearer.parabolic.re > 0.5, where);
            break; }
        case '6': {
            double area = areaLB(box.nearer, where);
            check(area > g_max_area, where);
            break;
        }
    }
}

// Check that the matrix is NOT of the forms
// 1 b  OR  -1  b
// 0 1       0 -1
// anywhere in the box
const bool not_parabolic_at_inf(const SL2ACJ& x) {
    return absLB(x.c) > 0
        || ((absLB(x.a - 1) > 0 ||  absLB(x.d - 1) > 0)
            && (absLB(x.a + 1) > 0 || absLB(x.d + 1) > 0));
}

// Check that the matrix is NOT of the forms
// 1  OR  -1  0
// 0 1       0 -1
// anywhere in the box
const bool not_identity(const SL2ACJ& x) {
    return absLB(x.b) > 0 || not_parabolic_at_inf(x);
}

// The infinity horoball has height t = 1/|loxodromic_sqrt|. An SL2C matrix
// a b
// c d
// Takes an infinity horoball of height t to a horoball of height 1/(t |c|^2)
// We want 1/(t |c|^2) > t. With t = 1/|loxodromic_sqrt|, this gives
// |c / loxodromic_sqrt| < 1.
const bool large_horoball(const SL2ACJ& x, const ACJParams& p) {
    return absUB( x.c / p.loxodromic_sqrt ) < 1;
}

// Conditions checked:
//  1) word is not a parabolic fixing infinity anywhere in the box
//  2) word(infinity_horoball) intersects infinity_horoball
void verify_killed(char* where, char* word)
{
    Box box = build_box(where);
    SL2ACJ w = construct_word(box.cover, word);

    check(large_horoball(w, box.cover), where);
    check(not_parabolic_at_inf(w), where);
}

// Conditions checked:
//  1) word has non-zero g-length at most g_len
//  2) word(infinity_horoball) intersects infinity_horoball
void verify_len(char* where, char* word, int g_len)
{
    Box box = build_box(where);
    SL2ACJ w = construct_word(box.cover, word);

    check(g_length(word) > 0, where);
    check(g_length(word) <= g_len, where);
    check(large_horoball(w, box.cover), where);
}

// Conditions checked:
//  1) the box is inside the variety neighborhood for given word
void verify_variety(char* where, char* word)
{
    Box box = build_box(where);
    SL2ACJ w = construct_word(box.cover, word);

    check((absUB(w.c) < 1) && (absUB(w.b) < 1 || absLB(w.c) > 0), where);
}

void verify_only_bad_parabolics(char* where, char* word)
{
    Box box = build_box(where);
    SL2ACJ w = construct_word(box.cover, word);

    // Tests if w hits any lattice points (when w is parabolic).
    // This test is inconclusive is w has large transtalion (i.e. translate
    // w's center into the first postive quad of the lattice.)

    double one = 1; // Exact
    // We check the box is small enough to determine the sign of translation
    check((absUB(w.d - one) < 2 || absUB(w.d + one) < 2 || 
           absUB(w.a - one) < 2 || absUB(w.a + one) < 2 ), where);

    // For all parabolic points in the box, we want verify
    // that none of them are lattice points. At such a point w.a = +/- 1, so
    //      translation = +/- w.b
    // Away from the parabolic locus, everything is assumed elimanited.

    // There are now 4 equations to check corresponding to the intersection
    // of 4 cirles :
    // |translation - 0          | < |1 + lattice|
    // |translation - (1+lattice)| < |1 + lattice|
    // |translation - 1          | < |1 - lattice|
    // |translation - lattice    | < |1 - lattice|
    // These inequailties show that transltion is not on the lattice (assuming
    // parameterd space constraitns). See proof in text.
    // 
    // To make the computation efficient, rearange and
    // take absolute values at the end.

    ACJ T = (absUB(w.d - one) < 2 || absUB(w.a - one) < 2) ? w.b : -w.b;
    ACJ L = p.lattice;

    ACJ d1 = T / (L + one);
    ACJ d2 = d1 - one;
    ACJ d3 = (T - one) / (L - one);
    ACJ d4 = d3 - one;

    check(absUB(d1) < 1, where);
    check(absUB(d2) < 1, where);
    check(absUB(d3) < 1, where);
    check(absUB(d4) < 1, where);
}

#define ONE_OVER_E2_MIN 0.92593

// Returns true if image of the infinity horoball under w
// is smaller than a required e2 ball
const bool smaller_than_e2_horoball(SL2ACJ& x, const ACJParams& p) {
  return absUB(p.loxodromic_sqrt / w.c) < ONE_OVER_E2_MIN;
}

const bool disk_killed_by_word(ACJ& center, ACJ& radius, const SL2ACJ& x) {
  // Returns true if the disk is contained in the "kill radius"
  // of the horoball w(H_infty). A horoball of height h and center q
  // will interesect w(H_infty) if:
  //       h |S|/|c|^2 > dist(a/c, q)^2
  // We know that h >= 1/(|S| e_2^2), so an intersection is guaranteed if
  //      1/e_2 > abs(a - c q)
  // Since the disk has a radius, we need to make sure
  //      ONE_OVER_E2_MIN > (1+EPS)*(absUB(a - c center) + absUB(c radius))
  return (1 + EPS) * (absUB(x.a - x.c * center) 
                    + absUB(x.c * radius)) < ONE_OVER_E2_MIN; 
}

void verify_no_e2_disks(char* where) {
    Box box = build_box(where);
    ACJParams cover = box.cover;
    XComplex nL = box.nearer.lattice;
    XComplex fL = box.further.lattice;

    std::vector<SL2ACJ> e2_mats;
    for (int i = 0; i < NUM_E2_WORDS; ++i) {
        e2_mats.push(construct_word(box.cover, e2_words[i]));
    }

    std::queue lattice_cover = {initial_lattice_cover()};
    while (!lattice_cover.empty()) {
        bool rect_killed = false;
        Rect current = lattice_cover.front();
        ACJ c = current.c;
        ACJ r = current.r;
        if (// Check rect is below x-axis
            (c.f.im < 0 && (1 + EPS) * (absUB(r) + c.f.im) < 0)
         || // Check rect is above y = im L
            (fL.im > 0 && c.f.im > fL.im &&
            (1 + 2 * EPS) * (absUB(r) + (fL.im - c.f.im)) < 0)
         || // Check if re L > 0 and rect is left of x = 0
            (nL.re > 0 && c.f.re < 0 &&
            (1 + EPS) * (absUB(r) + c.f.re) < 0)
         || // Check if re L < 0 and rect is right of x = 1
            (nL.re < 0 && c.f.re > 1 &&
            (1 + 2 * EPS) * (absUB(r) + (1 - c.f.re)) < 0)
         || // Check if re L < 0 and rect is left of x = re L
            (fL.re < 0 && c.f.re < fL.re &&
            (1 + 2 * EPS) * ((absUB(r) - fL.re) + c.f.re) < 0)
         || // Chek if re L > 0 and rect is right of x = 1 + re L 
            (fL.re > 0 && c.f.re > 1 + fL.re &&
            (1 + 2 * EPS) * ((fL.re + 1) + (absUB(r) - c.f.re)) < 0)) {
          rect_killed = true;
        } else { 
            double area_lb = areaLB(box.nearer);
            if (area_lb >= g_min_e2_area) {
                for (int i = 0; i < NUM_E2_WORDS; ++i) {
                    int g_len = g_length(e2_words[i]);
                    SL2ACJ w = construct_word(box.cover, e2_words[i]);
                    if (g_len > 1 && smaller_than_e2_horoball(w, box.cover) &&
                        disk_killed_by_word(c, r, w)) {
                        rect_killed = true;
                        break;
                    }
                }
            }
        }
        if (!rect_killed) {
            lattice_cover.push(child(current, 0));
            lattice_cover.push(child(current, 1));
        }
        lattice_cover.pop();
    }
}
