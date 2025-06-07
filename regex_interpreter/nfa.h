#ifndef NFA_H
#define NFA_H

#include "interpreter.h"

#define MAX_TRANSITIONS 1024
#define MAX_STATES 256

typedef struct
{
    int state;
    char* token;
} NFA_State;


typedef struct {
    NFA_State from;
    char symbol;
    NFA_State to;
} Transition;

typedef struct {
    int states;
    NFA_State finals[MAX_STATES];
    int finals_count;
    Transition transitions[MAX_TRANSITIONS];
    int transitions_count;
    NFA_State start;
} NFA;

NFA_State* create_state(int state);

NFA nfa_symbol(char symbol);
NFA nfa_epsilon();
NFA nfa_union(NFA a, NFA b);
NFA nfa_concat(NFA a, NFA b);
NFA nfa_closure(NFA a);
NFA nfa_any();
NFA eval(Node *node);

#endif
