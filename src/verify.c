#include <stdlib.h>
#include "elimination.h"

double max_area = 5.95;
double e2_area_min = 5.65; 
double one_over_e2_min = 0.92593;


void verify(char* where, size_t depth, size_t* count_ptr)
{
    check(depth < MAX_DEPTH, where);
    *count_ptr += 1;
    char code[MAX_CODE_LEN];
    fgets(code, MAX_CODE_LEN, stdin);
    switch(code[0]) {
        case 'X': { 
            *count_ptr -= 1; // don't count branch nodes
            where[depth] = '0';
            where[depth + 1] = '\0';
            verify(where, depth + 1, count_ptr);
            where[depth] = '1';
            where[depth + 1] = '\0';
            verify(where, depth + 1, count_ptr);
            break; }
        case '0': 
        case '1': 
        case '2': 
        case '3': 
        case '4': 
        case '5': 
        case '6': {
            verify_out_of_bounds(where, code[0]);
            break; }
        case '9': {
            verify_no_e2_horoball(where);
            break; }
        case 'K': { // Line has format  K(word) - killer word
            parse_word(code);
            verify_large_horoball(where, code);
            break; }
        case 'S': { // Line has format S(word) - g-length 7 word
            parse_word(code);
            verify_g_length(where, code, 7);
            break; }
        case 'V': { // Line has format V(word)
            parse_word(code);
            verify_variety_g_length(where, code, 7);
            break; }
        case 'Q': { // Line has format Q(word)
            parse_word(code);
            verify_short_parabolic(where, code);
            break; }
        case 'L': { // Line has format L(word)
            parse_word(code);
            verify_bad_parabolic(where, code);
            break; }
        case 'E': { // Line has format E(word)
            word_pair p = get_word_pair(code);
            verify_bad_elliptic(where, p.first, p.second);
            break; }
        case 'T': { // Line has format T(word,word) - variety intersection
            word_pair p = get_word_pair(code);
            verify_variety(where, p.first);
            verify_variety(where, p.second);
            printf("Valid variety intersection: %s and %s\n", p.first, p.second);
            break; }
        // We fail by default, guaranteeing completes on the tree
        default: {
            check(false, where);
        }
    }
    // Block below is only for printing progress. 
    //    progress bar code from: https://stackoverflow.com/a/36315819/1411737
    if (*count_ptr % (1 << 18) == 0 && code[0] != 'X') {
        #define PBSTR "++++++++++++++++++++++++++++++++++++++++++++++++++"
        #define PBWIDTH 50
        #define NUM_NODES 1394524064
        double fraction = ((double) *count_ptr) / NUM_NODES;
        int lpad = (int) (fraction * PBWIDTH);
        int rpad = PBWIDTH - lpad;
        printf("\r%6.2f%% [%.*s%*s] of %d", 100 * fraction, lpad, PBSTR, rpad, "", NUM_NODES);
        fflush(stdout);
    }
    if (depth == 0) {
        printf("\r%6.2f%% [%s] of %d\n", 100.00, PBSTR, NUM_NODES);
    }
}

int main(int argc, char**argv)
{
    if(argc != 1) {
        fprintf(stderr,"Usage: %s < data\n", argv[0]);
        exit(1);
    }
    char where[MAX_DEPTH];
    size_t depth = 0;
    where[depth] = '\0';

    printf("Begin verify %s - {\n", where);
    initialize_roundoff();
    size_t count = 0;
    verify(where, depth, &count);
    if(!roundoff_ok()){
        printf(". underflow may have occurred\n");
        exit(1);
    }
    printf("Successfully verified all %lu nodes\n", count);
    printf("}.\n");
    exit(0);
}
