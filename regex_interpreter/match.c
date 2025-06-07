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

    return equals;
}

String_Match* match(DFA* dfa, char* str) {
    String_Match* match = (String_Match*)malloc(sizeof(String_Match));
    match->matched = 0;
    match->matches = 0;
    
    int length = strlen(str);
    State actual_state = dfa->start;
    int change = 1;

    for (int i = 0; i < length; i++) {
        if (!change) break; 

        if (str[i] == ' ') {
            continue;
        }

        change = 0;

        for (int j = 0; j < dfa->transitions_count; j++) {
            DFA_Transition transition = dfa->transitions[j];

            if (set_equals(&actual_state, &(transition.from)) && str[i] == transition.symbol) {
                copy_state_set(&actual_state, &(transition.to));
                change = 1;
                break;
            }
        }
    }

    if (change) {
        for (int i = 0; i < dfa->finals_count; i++) {
            if (set_equals(&actual_state, &(dfa->finals[i]))) {
                match->matched = 1;
                match->matches = dfa->finals[i].matches;
                match->tokens = (char**)malloc(sizeof(char) * 256 * sizeof(char) * 50);

                for (int j = 0; j < dfa->finals[i].matches; j++) {
                    match->tokens[j] = dfa->finals[i].tokens[j];
                }
                break;
            }
        }
    }

    return match;
}