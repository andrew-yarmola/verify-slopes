#include <stdlib.h>
#include "elimination.h"

double g_max_area=3.465;

void identify(char* where, size_t depth, size_t* count_ptr)
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
            identify(where, depth + 1, count_ptr);
            where[depth] = '1';
            where[depth + 1] = '\0';
            identify(where, depth + 1, count_ptr);
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
        case 'K': { // Line has format  K(word) - killer word
            parse_word(code);
            verify_killed(where, code);
            break; }
        case 'T': { // Line has format T(word) - g-length 3 word
            parse_word(code);
            verify_len(where, code, 3);
            break; }
        case 'I': { // Line has format I(word,word) - variety intersection
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

    printf("Begin identify %s - {\n", where);
    initialize_roundoff();
    size_t count = 0;
    identify(where, depth, &count);
    if(!roundoff_ok()){
        printf(". underflow may have occurred\n");
        exit(1);
    }
    printf("Successfully identified varieties above and verified %lu nodes\n", count);
    printf("}.\n");
    exit(0);
}
