#ifndef NFA_H
#define NFA_H

#include "interpreter.h"

#define MAX_TRANSITIONS 1024
#define MAX_STATES 256

typedef struct {
    int from;
    char symbol;
    int to;
} Transition;

typedef struct {
    int states;
    int finals[MAX_STATES];
    int finals_count;
    Transition transitions[MAX_TRANSITIONS];
    int transitions_count;
    int start;
} NFA;

NFA nfa_symbol(char symbol);
NFA nfa_epsilon();
NFA nfa_union(NFA a, NFA b);
NFA nfa_concat(NFA a, NFA b);
NFA nfa_closure(NFA a);
NFA nfa_any();
NFA eval(Node *node);

#endif
