#include "dfa.h"

int* epsilon_closure(NFA* nfa, int state) {
    int queue[nfa->states];
    int* set_states = (int*)malloc(sizeof(int) * nfa->states);
    bool visited[nfa->states];
    int index = 0;
    queue[index++] = state;
    visited[state] = true;

    while (index > 0) {
        int s = queue[--index];
        printf("state: %d\n", s);

        for (int i = 0; i < nfa->transitions_count; i++) {
            Transition t = nfa->transitions[i];
            if (t.from == s && t.symbol == EPSILON && !visited[t.to]) {
                queue[index++] = t.to;
                visited[t.to] = 1;
            }

            if (t.from == s && t.symbol != EPSILON && t.to < nfa->states) {
                set_states[t.from] = 1;
            }
        }
    }

    for (int i = 0; i < nfa->states; i++) {
        if (set_states[i]) {
            printf("%d\n", i);
        }
    }

    return set_states;
}