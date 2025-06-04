#ifndef DFA_H
#define DFA_H

#include <stdint.h>
#include "nfa.h"

#define MAX_DFA_STATES 1024 // Ajustar según necesidad

typedef struct {
    int from[MAX_STATES];
    char symbol;
    int to[MAX_STATES];
} DFA_Transition;

typedef struct {
    int states_count;
    int* start;
    int finals[MAX_DFA_STATES][MAX_STATES];
    int finals_count;
    DFA_Transition transitions[MAX_DFA_STATES];
    int transitions_count;
    int state_masks[MAX_DFA_STATES][MAX_STATES]; // Máscaras de bits para cada estado DFA
} DFA;

typedef struct {
    int states[MAX_STATES];
    int count;
} StateClosure;

int* epsilon_clousure(NFA* nfa, int* states);
DFA* nfa_to_dfa(NFA* nfa);

#endif
