#include "match.h"
#include <string.h>

int equals(int* x, int* y, int count) {
    int equals = 1;

    for (int i = 0; i < count; i++) {
        if (x[i] != y[i]) {
            equals = 0;
            break;
        }
    }

    // printf("equals: %d\n", equals);

    return equals;
}

int match(NFA* nfa, DFA* dfa, char* str) {
    int length = strlen(str);
    State actual_state = dfa->start;
    int change = 1;

    for (int i = 0; i < length; i++) {
        if (!change) break; 
        change = 0;

        for (int j = 0; j < dfa->transitions_count; j++) {
            DFA_Transition transition = dfa->transitions[j];

            // printf("\nfrom: ");
            // for (int j = 0; j < nfa->states; j++) {
            //     printf("%d, ", transition.from[j]);
            // }
            // printf("\n");

            // printf("symbol:%c\n", transition.symbol);

            // printf("to: ");
            // for (int j = 0; j < nfa->states; j++) {
            //     printf("%d, ", transition.to[j]);
            // }

            // printf("\n actual state: ");
            // for (int j = 0; j < nfa->states; j++) {
            //     printf("%d, ", actual_state[j]);
            // }
            // printf("\n");

            // printf("symbol:%c\n", str[i]);

            // printf("symbols equals: %d\n", str[i] == transition.symbol);

            if (set_equals(&actual_state, &(transition.from)) && str[i] == transition.symbol) {
                // printf("change\n");
                copy_state_set(&actual_state, &(transition.to));
                change = 1;
                break;
            }
        }
    }

    int is_final = 0;
    if (change) {
        for (int i = 0; i < dfa->finals_count; i++) {
            if (set_equals(&actual_state, &(dfa->finals[i]))) {
                is_final = 1;
                break;
            }
        }
    }

    return is_final;
}