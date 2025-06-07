#ifndef MATCH_H
#define MATCH_H

#include "dfa.h"

typedef struct {
    int matched;
    char** tokens;
    int matches;
} String_Match;

String_Match* match(DFA* dfa, char* str);

#endif
