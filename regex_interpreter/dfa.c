#include "dfa.h"

int* epsilon_clousure(NFA* nfa, int* states) {
    int queue[nfa->states];
    int* set_states = (int*)malloc(sizeof(int) * nfa->states);
    bool visited[nfa->states];
    int index = 0;

    for (int i = 0; i < nfa->states; i++) {
        if (states[i]) {
            queue[index++] = i;
            visited[i] = true;
        }
    }

    while (index > 0) {
        int s = queue[--index];

        for (int i = 0; i < nfa->transitions_count; i++) {
            Transition t = nfa->transitions[i];

            if (t.from == s && t.symbol == EPSILON && !visited[t.to]) {
                queue[index++] = t.to;
                visited[t.to] = 1;
            }

            if (t.from == s && t.symbol != EPSILON) {
                set_states[s] = 1;
            }
        }

        for (int i = 0; i < nfa->finals_count; i++) {
            if (nfa->finals[i] == s) {
                set_states[s] = 1;
                break;
            }
        }
    }

    for (int i = 0; i < nfa->states; i++) {
        if (!set_states[i]) {
            set_states[i] = 0;
        }
    }

    return set_states;
}

void copy_state_set(int* dest, int* src, int size) {
    for (int i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

int dfa_contains_state(NFA* nfa, DFA* dfa, int* state) {
    if (!dfa)
        return 0;

    for (int i = 0; i < dfa->states_count; i++) {
        int count = 0;
        int empty = 0;
        for (int j = 0; j < nfa->states; j++) {
            if (state[j] == dfa->state_masks[i][j]) {
                count++;
            }

            if (state[j] == 0) {
                empty++;
            }
        }

        if (count == nfa->states || empty == nfa->states) {
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
    int* start = (int*)malloc(sizeof(int) * nfa->states);
    start[0] = 1;
    dfa->start = epsilon_clousure(nfa, start);

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

    int* queue[MAX_DFA_STATES];
    int index = 0;
    queue[index++] = dfa->start;
    copy_state_set(dfa->state_masks[dfa->states_count++], dfa->start, nfa->states);
    
    while (index>0)
    {
        int* Q = queue[--index];
        for (int s = 0; s < symbols_count; s++) { // for symbol in symbols
            int sub_set[nfa->states];
            int states_count = 0;
            for (int q = 0; q < nfa->states; q++) { // for q in Q 
                for (int t = 0; t < nfa->transitions_count; t++) { // for transition in nfa.transitions
                    Transition transition = nfa->transitions[t];

                    if (transition.from == q && Q[q] == 1 && transition.symbol == symbols[s]) {
                        sub_set[transition.to] = 1;
                        states_count++;
                    }
                }
            }

            for (int i = 0; i < nfa->states; i++) {
                if (sub_set[i] != 1) {
                    sub_set[i] = 0;
                }
            }

            if (states_count) {
                int* new_Q = epsilon_clousure(nfa, sub_set);

                DFA_Transition* new_transition = (DFA_Transition*)malloc(sizeof(DFA_Transition));
                copy_state_set(new_transition->from, Q, nfa->states);
                new_transition->symbol = symbols[s];
                copy_state_set(new_transition->to, new_Q, nfa->states);

                dfa->transitions[dfa->transitions_count++] = *new_transition;

                if (!dfa_contains_state(nfa, dfa, new_Q)) {
                    copy_state_set(dfa->state_masks[dfa->states_count++], new_Q, nfa->states);
                    queue[index++] = new_Q;
                }
            }

            for (int i = 0; i < nfa->states; i++) {
                sub_set[i] = 0;
            }
        }
    }

    for (int i = 0; i < nfa->finals_count; i++) { // for final in nfa
        for (int j = 0; j < dfa->states_count; j++) { // for estados in dfa
            for (int k = 0; k < nfa->states; k++) {      // for estado en estados
                if (nfa->finals[i] == k && dfa->state_masks[j][k] == 1) {
                    copy_state_set(dfa->finals[dfa->finals_count++], dfa->state_masks[j], nfa->states);
                    continue;
                }                    
            }
        }
    }   
    
    printf("finales: %d\n", dfa->finals_count);
    printf("estados: %d\n", dfa->states_count);

    for (int i = 0; i < dfa->states_count; i++) {
        printf("estado: %d: ", i);
        for (int j = 0; j < nfa->states; j++) {
            printf("%d, ", dfa->state_masks[i][j]);
        }
        printf("\n");
    }

    printf("transiciones:\n");

    for (int i = 0; i < dfa->transitions_count; i++) {
        DFA_Transition t = dfa->transitions[i];

        printf("from:");
        for (int j = 0; j < nfa->states; j++) {
            printf("%d, ", t.from[j]);
        }
        printf("\n");

        printf("symbol: %c\n", t.symbol);

        printf("to:");
        for (int j = 0; j < nfa->states; j++) {
            printf("%d, ", t.to[j]);
        }
        printf("\n");
    }

    return dfa;
}