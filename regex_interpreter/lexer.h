#ifndef LEXER_H
#define LEXER_H

#include "nfa.h"

typedef struct 
{
    char regex[50];
    char token[50];
    NFA nfa;
} Regex;


#endif