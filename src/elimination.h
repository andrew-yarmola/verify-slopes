#ifndef _elimination_h_
#define _elimination_h_
#include "box.h"

#define MAX_DEPTH 256
#define MAX_CODE_LEN 512

// Helper functions

void check(bool inequalities, char* where);

void parse_word(char* code);

typedef struct {
    char first[MAX_CODE_LEN];
    char second[MAX_CODE_LEN];
} word_pair;

word_pair get_word_pair(char* code);

int g_length(char* word);

// Elimination functions

SL2ACJ construct_G(const ACJParams& params);

ACJ construct_T(const ACJParams& params, int M, int N);

SL2ACJ construct_word(const ACJParams& params, const char* word);

inline const double areaLB(const XParams&nearer, char* where);

void verify_out_of_bounds(char* where, char bounds_code);

const int not_parabolic_at_inf(const SL2ACJ& x);

const int not_identity(const SL2ACJ& x);

const int large_horoball(const SL2ACJ& x, const ACJParams& p);

void verify_killed(char* where, char* word);

void verify_len(char* where, char* word, int g_len);

void verify_variety(char* where, char* variety);

#endif // _elimination_h_
