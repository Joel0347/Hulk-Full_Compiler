#ifndef DFA_H
#define DFA_H

#include <stdint.h>
#include "nfa.h"

#define MAX_DFA_STATES 1024 // Ajustar según necesidad

typedef struct {
    int set[MAX_STATES];
    int count;
} State;

typedef struct {
    State from;
    char symbol;
    State to;
} DFA_Transition;

typedef struct {
    int states_count;
    State start;
    State finals[MAX_DFA_STATES];
    int finals_count;
    DFA_Transition transitions[MAX_DFA_STATES];
    int transitions_count;
    State state_masks[MAX_DFA_STATES];
} DFA;

// typedef struct {
//     int* states;
//     int count;
// } StateClosure;

State epsilon_clousure(NFA* nfa, int* states, int count);
void copy_state_set(State* dest, State* src);
int set_equals(State* x, State* y);
DFA* nfa_to_dfa(NFA* nfa);

#endif
