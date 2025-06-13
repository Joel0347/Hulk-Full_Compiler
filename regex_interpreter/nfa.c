#include "nfa.h"

NFA_State* create_state(int state) {
    NFA_State* nfa_state = (NFA_State*)malloc(sizeof(NFA_State));
    nfa_state->state = state;

    return nfa_state;
}

NFA nfa_symbol(char symbol) {
    NFA nfa = {0};
    nfa.states = 2;
    nfa.start = *create_state(0);
    nfa.finals = (NFA_State*)malloc(sizeof(NFA_State));
    nfa.finals[0] = *create_state(1);
    nfa.finals_count = 1;
    Transition transition = (Transition){*create_state(0), symbol, *create_state(1)};
    nfa.transitions = (Transition*)malloc(sizeof(Transition));
    nfa.transitions[0] = transition;
    nfa.transitions_count = 1;
    return nfa;
}

NFA nfa_epsilon() {
    NFA nfa = {0};
    nfa.states = 2;
    nfa.start = *create_state(0);
    nfa.finals = (NFA_State*)malloc(sizeof(NFA_State));
    nfa.finals[0] = *create_state(1);
    nfa.finals_count = 1;
    Transition transition = (Transition){*create_state(1), EPSILON, *create_state(1)};
    nfa.transitions = (Transition*)malloc(sizeof(Transition));
    nfa.transitions[0] = transition;
    nfa.transitions_count = 1;
    return nfa;
}

NFA nfa_union(NFA a, NFA b) {
    NFA nfa = {0};
    int offset = a.states + 1;
    nfa.states = a.states + b.states + 2;
    nfa.start = *create_state(0);
    nfa.finals = (NFA_State*)malloc(sizeof(NFA_State));
    nfa.finals[0] = *create_state(nfa.states - 1);
    nfa.finals_count = 1;
    nfa.transitions_count = 2 + a.transitions_count + b.transitions_count + a.finals_count + b.finals_count;
    nfa.transitions = (Transition*)malloc(sizeof(Transition) * (nfa.transitions_count));
    int index = 0;

    nfa.transitions[index++] = (Transition){*create_state(0), EPSILON, *create_state(a.start.state + 1)};
    nfa.transitions[index++] = (Transition){*create_state(0), EPSILON, *create_state(b.start.state + offset)};

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];
        nfa.transitions[index++] = (Transition){*create_state(t.from.state + 1), t.symbol, *create_state(t.to.state + 1)};
    }

    for (int i = 0; i < b.transitions_count; ++i) {
        Transition t = b.transitions[i];
        nfa.transitions[index++] = (Transition){*create_state(t.from.state + offset), t.symbol, *create_state(t.to.state + offset)};
    }

    for (int i = 0; i < a.finals_count; ++i)
        nfa.transitions[index++] = (Transition){*create_state(a.finals[i].state + 1), EPSILON, *create_state(nfa.states - 1)};

    for (int i = 0; i < b.finals_count; ++i)
        nfa.transitions[index++] = (Transition){*create_state(b.finals[i].state + offset), EPSILON, *create_state(nfa.states - 1)};
    return nfa;
}

NFA nfa_concat(NFA a, NFA b) {
    NFA nfa = {0};
    int offset = a.states;
    nfa.states = a.states + b.states;
    nfa.start = a.start;
    nfa.transitions_count = a.transitions_count + a.finals_count + b.transitions_count;
    nfa.transitions = (Transition*)malloc(sizeof(Transition) * nfa.transitions_count);
    nfa.finals_count = b.finals_count;
    nfa.finals = (NFA_State*)malloc(sizeof(NFA_State) * nfa.finals_count);
    int index = 0;

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];
        nfa.transitions[index++] = (Transition){t.from, t.symbol, t.to};
    }

    for (int i = 0; i < a.finals_count; ++i) {
        nfa.transitions[index++] = (Transition){a.finals[i], EPSILON, *create_state(b.start.state + offset)};
    }

    for (int i = 0; i < b.transitions_count; ++i) {
        Transition t = b.transitions[i];
        nfa.transitions[index++] = (Transition){*create_state(t.from.state + offset), t.symbol, *create_state(t.to.state + offset)};
    }

    index = 0;
    for (int i = 0; i < b.finals_count; ++i)
        nfa.finals[index++] = *create_state(b.finals[i].state + offset);
    return nfa;
}

NFA nfa_closure(NFA a) {
    NFA nfa = {0};
    nfa.states = a.states + 2;
    nfa.start = *create_state(0);
    int new_final = nfa.states - 1;
    nfa.finals = (NFA_State*)malloc(sizeof(NFA_State));
    nfa.finals[0] = *create_state(new_final);
    nfa.finals_count = 1;
    nfa.transitions_count = 2 + a.transitions_count + 2 * a.finals_count;
    nfa.transitions = (Transition*)malloc(sizeof(Transition) * nfa.transitions_count);
    int index = 0;

    nfa.transitions[index++] = (Transition){*create_state(0), EPSILON, *create_state(a.start.state + 1)};
    nfa.transitions[index++] = (Transition){*create_state(0), EPSILON, *create_state(new_final)};

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];
        nfa.transitions[index++] = (Transition){*create_state(t.from.state + 1), t.symbol, *create_state(t.to.state + 1)};
    }

    for (int i = 0; i < a.finals_count; ++i) {
        nfa.transitions[index++] = (Transition){*create_state(a.finals[i].state + 1), EPSILON, *create_state(a.start.state + 1)};
        nfa.transitions[index++] = (Transition){*create_state(a.finals[i].state + 1), EPSILON, *create_state(new_final)};
    }
    return nfa;
}

NFA nfa_any() {
    NFA nfa = nfa_epsilon();
    for (char c = 32; c <= 126; ++c) {
        if (c == '\n') continue;
        NFA char_nfa = nfa_symbol(c);
        nfa = nfa_union(nfa, char_nfa);
    }
    return nfa;
}

// AST Evaluation
NFA eval(Node *node) {
    if (!node) return nfa_epsilon();
    switch (node->type) {
        case NODE_SYMBOL: return nfa_symbol(node->symbol);
        case NODE_EPSILON: return nfa_epsilon();
        case NODE_UNION: return nfa_union(eval(node->left), eval(node->right));
        case NODE_CONCAT: return nfa_concat(eval(node->left), eval(node->right));
        case NODE_CLOSURE: return nfa_closure(eval(node->left));
    }
    return nfa_epsilon();
}

NFA nfa_union_BIG(NFA a, NFA b) {
    NFA nfa = {0};
    int offset = a.states + 1;
    nfa.states = a.states + b.states + 2;
    nfa.start = *create_state(0);
    nfa.start.token = "error";
    nfa.finals_count = a.finals_count + b.finals_count;
    nfa.finals = (NFA_State*)malloc(sizeof(NFA_State) * nfa.finals_count);
    nfa.transitions_count = 2 + a.transitions_count + b.transitions_count;
    nfa.transitions = (Transition*)malloc(sizeof(Transition) * nfa.transitions_count);
    int index = 0;

    NFA_State* state_a = create_state(a.start.state + 1);
    state_a->token = a.start.token;

    NFA_State* state_b = create_state(b.start.state + offset);
    state_b->token = b.start.token;

    nfa.transitions[index++] = (Transition){nfa.start, EPSILON, *state_a};
    nfa.transitions[index++] = (Transition){nfa.start, EPSILON, *state_b};

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];

        NFA_State* state_from = create_state(t.from.state + 1);
        state_from->token = t.from.token;

        NFA_State* state_to = create_state(t.to.state + 1);
        state_to->token = t.to.token;

        nfa.transitions[index++] = (Transition){*state_from, t.symbol, *state_to};
    }

    for (int i = 0; i < b.transitions_count; ++i) {
        Transition t = b.transitions[i];

        NFA_State* state_from = create_state(t.from.state + offset);
        state_from->token = t.from.token;

        NFA_State* state_to = create_state(t.to.state + offset);
        state_to->token = t.to.token;

        nfa.transitions[index++] = (Transition){*state_from, t.symbol, *state_to};
    }

    index = 0;
    for (int i = 0; i < a.finals_count; ++i) {
        NFA_State state_final = *create_state(a.finals[i].state + 1);
        state_final.token = a.finals[i].token;

        nfa.finals[index++] = state_final;
    }

    for (int i = 0; i < b.finals_count; ++i) {
        NFA_State state_final = *create_state(b.finals[i].state + offset);
        state_final.token = b.finals[i].token;

        nfa.finals[index++] = state_final;
    }
    
    return nfa;
}

int match_nfa(NFA* nfa, NFA_State* actual_state, char* str, int pos) {
    for (int i = 0; i < nfa->transitions_count; i++) {
        Transition t = nfa->transitions[i];
        if (t.from.state == actual_state->state) {
            if (t.symbol == EPSILON) {
                if (match_nfa(nfa, &t.to, str, pos)) {
                    return 1;
                }
            }

            if (t.symbol == str[pos]) {
                if (match_nfa(nfa, &t.to, str, pos+1)) {
                    return 1;
                }
            }
        }
    }

    if (pos >= strlen(str)) {
        for (int i = 0; i < nfa->finals_count; i++) {
            if (nfa->finals[i].state == actual_state->state) {
                return 1;
            }
        }

        return 0;
    }

    return 0;
}