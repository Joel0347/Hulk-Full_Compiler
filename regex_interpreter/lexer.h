#ifndef LEXER_H
#define LEXER_H

#include "nfa.h"

typedef struct 
{
    char* regex;
    char* token;
    NFA* nfa;
} Regex;


#endif