#include "dfa.h"
#include <stdlib.h>
#include <string.h>

State epsilon_clousure(NFA* nfa, int* states, int count) {
    State* set = (State*)malloc(sizeof(State));
    int queue[nfa->states];
    int* set_states = (int*)malloc(sizeof(int) * nfa->states);
    bool visited[nfa->states];
    int index = 0;
    int count_set = 0;

    for (int i = 0; i < nfa->states; i++) {
        visited[i] = false;
    }

    for (int i = 0; i < count; i++) {
        queue[index++] = states[i];
        visited[states[i]] = true;
    }

    while (index > 0) {
        int s = queue[--index];

        for (int i = 0; i < nfa->transitions_count; i++) {
            Transition t = nfa->transitions[i];

            if (t.from.state == s && t.symbol == EPSILON && !visited[t.to.state]) {
                queue[index++] = t.to.state;
                visited[t.to.state] = 1;
            }

            if (t.from.state == s && t.symbol != EPSILON) {
                set_states[count_set++] = s;

                if (!state_contains_token(set, t.from.token)) {
                    set->tokens[set->matches++] = t.from.token;
                }
            }
        }

        for (int i = 0; i < nfa->finals_count; i++) {
            if (nfa->finals[i].state == s) {
                set_states[count_set++] = s;
            
                if (!state_contains_token(set, nfa->finals[i].token)) {
                    set->tokens[set->matches++] = nfa->finals[i].token;
                }
                break;
            }
        }
    }

    set->count = count_set;
    for (int i = 0; i < set->count; i++) {
        set->set[i] = set_states[i];
    }

    return *set;
}

int state_contains_token(State* state, char* token) {
    if (!strcmp("error", token)) return 1;
    
    for (int i = 0; i < state->matches; i++) {
        if (!strcmp(state->tokens[i], token)) {
            return 1;
        }
    }

    return 0;
}

void copy_state_set(State* dest, State* src) {
    dest->count = src->count;
    dest->matches = src->matches;

    for (int i = 0; i < src->count; i++) {
        dest->set[i] = src->set[i];
    }

    for (int i = 0; i < src->matches; i++) {
        dest->tokens[i] = src->tokens[i];
    }
}

int set_equals(State* x, State* y) {
    for (int i = 0; i < x->count; i++) {
        if (!state_contains_element(y, x->set[i])) {
            return 0;
        }
    }

    for (int i = 0; i < y->count; i++) {
        if (!state_contains_element(x, y->set[i])) {
            return 0;
        }
    }

    return 1;
}

int dfa_contains_state(DFA* dfa, State* state) {
    if (!dfa)
        return 0;

    for (int i = 0; i < dfa->states_count; i++) {
        State actual_state = dfa->state_masks[i];

        if (state->count != actual_state.count) {
            continue;
        }

        if (set_equals(state, &actual_state)) {
            return 1;
        }
    }

    return 0;
}

int state_contains_element(State* state, int element) {
    for (int i = 0; i < state->count; i++) {
        if (state->set[i] == element) {
            return 1;
        }
    }

    return 0;
}

DFA* nfa_to_dfa(NFA* nfa) {
    DFA* dfa = (DFA*)malloc(sizeof(DFA));
    dfa->finals_count = 0;
    dfa->states_count = 0;
    dfa->transitions_count = 0;
    dfa->start = epsilon_clousure(nfa, &(nfa->start), 1);

    bool symbol_used[256] = {false};
    char symbols[MAX_SYMBOLS];
    int symbols_count = 0;

    for (int i = 0; i < nfa->transitions_count; i++) {
        char sym = nfa->transitions[i].symbol;
        if (sym != EPSILON && !symbol_used[(unsigned char)sym]) {
            symbols[symbols_count++] = sym;
            symbol_used[(unsigned char)sym] = true;
        }
    }

    State queue[MAX_STATES];
    int index = 0;
    queue[index++] = dfa->start;
    copy_state_set(&(dfa->state_masks[dfa->states_count++]), &(dfa->start));
    
    while (index>0)
    {
        State Q = queue[--index];
        for (int s = 0; s < symbols_count; s++) { // for symbol in symbols
            int sub_set[MAX_DFA_STATES];
            int states_count = 0;
            for (int q = 0; q < Q.count; q++) { // for q in Q 
                for (int t = 0; t < nfa->transitions_count; t++) { // for transition in nfa.transitions
                    Transition transition = nfa->transitions[t];

                    if (transition.from.state == Q.set[q] && transition.symbol == symbols[s]) {
                        sub_set[states_count++] = transition.to.state;
                    }
                }
            }

            if (states_count) {
                State new_Q = epsilon_clousure(nfa, sub_set, states_count);

                DFA_Transition* new_transition = (DFA_Transition*)malloc(sizeof(DFA_Transition));
                copy_state_set(&(new_transition->from), &Q);
                new_transition->symbol = symbols[s];
                copy_state_set(&(new_transition->to), &new_Q);

                dfa->transitions[dfa->transitions_count++] = *new_transition;

                if (!dfa_contains_state(dfa, &new_Q)) {
                    copy_state_set(&(dfa->state_masks[dfa->states_count++]), &new_Q);
                    queue[index++] = new_Q;
                }
            }
        }
    }

    for (int i = 0; i < nfa->finals_count; i++) { // for final in nfa
        for (int j = 0; j < dfa->states_count; j++) { // for estados in dfa
            for (int k = 0; k < nfa->states; k++) {      // for estado en estados
                if (nfa->finals[i].state == dfa->state_masks[j].set[k]) {
                    copy_state_set(&(dfa->finals[dfa->finals_count++]), &(dfa->state_masks[j]));
                    continue;
                }                    
            }
        }
    }   
    
    // printf("finales: %d\n", dfa->finals_count);
    // printf("estados: %d\n", dfa->states_count);

    // for (int i = 0; i < dfa->states_count; i++) {
    //     printf("estado: %d: ", i);
    //     for (int j = 0; j < dfa->state_masks[i].count; j++) {
    //         printf("%d, ", dfa->state_masks[i].set[j]);
    //     }
    //     printf("\n");
    // }

    // printf("transiciones:\n");

    // for (int i = 0; i < dfa->transitions_count; i++) {
    //     DFA_Transition t = dfa->transitions[i];

    //     printf("from:");
    //     for (int j = 0; j < t.from.count; j++) {
    //         printf("%d, ", t.from.set[j]);
    //     }
    //     printf("tokens: %d\n", t.from.matches);
    //     for (int j = 0; j < t.from.matches; j++) {
    //         printf("%s, ", t.from.tokens[j]);
    //     }

    //     printf("symbol: %c\n", t.symbol);

    //     printf("to:");
    //     for (int j = 0; j < t.to.count; j++) {
    //         printf("%d, ", t.to.set[j]);
    //     }
    //     printf("tokens: %d\n", t.to.matches);
    //     for (int j = 0; j < t.to.matches; j++) {
    //         printf("%s, ", t.to.tokens[j]);
    //     }
    // }

    return dfa;
}