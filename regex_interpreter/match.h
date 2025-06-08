#ifndef MATCH_H
#define MATCH_H

#include "dfa.h"
#include "token.h"

typedef struct {
    int matched;
    Lexer_Token* tokens;
    int count;
} String_Match;

String_Match* match(DFA* dfa, char* str);

#endif
