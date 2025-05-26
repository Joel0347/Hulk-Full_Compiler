#ifndef DFA_H
#define DFA_H

#include <stdint.h>
#include "nfa.h"

#define MAX_DFA_STATES 1024 // Ajustar según necesidad

typedef struct {
    int states_count;
    int start;
    int finals[MAX_DFA_STATES];
    int finals_count;
    Transition transitions[MAX_DFA_STATES];
    int transitions_count;
    uint32_t state_masks[MAX_DFA_STATES]; // Máscaras de bits para cada estado DFA
} DFA;

typedef struct {
    int states[MAX_STATES];
    int count;
} StateClosure;

int *epsilon_closure(NFA* nfa, int state);

#endif
