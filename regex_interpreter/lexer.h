#ifndef LEXER_H
#define LEXER_H

#include "nfa.h"

typedef struct 
{
    char regex[50];
    char token[50];
    NFA nfa;
    int priority;
} Regex;

Regex* converTo_regex(char* regex, char* token);
Regex* find_regex_by_token(Regex** list, char* token, int len);

#endif