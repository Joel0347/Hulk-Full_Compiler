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
    nfa.finals[0] = *create_state(1);
    nfa.finals_count = 1;
    nfa.transitions[0] = (Transition){*create_state(0), symbol, *create_state(1)};
    nfa.transitions_count = 1;
    return nfa;
}

NFA nfa_epsilon() {
    NFA nfa = {0};
    nfa.states = 2;
    nfa.start = *create_state(0);
    nfa.finals[0] = *create_state(1);
    nfa.finals_count = 1;
    nfa.transitions[0] = (Transition){*create_state(1), EPSILON, *create_state(1)};
    nfa.transitions_count = 1;
    return nfa;
}

NFA nfa_union(NFA a, NFA b) {
    NFA nfa = {0};
    int offset = a.states + 1;
    nfa.states = a.states + b.states + 2;
    nfa.start = *create_state(0);
    nfa.finals[0] = *create_state(nfa.states - 1);
    nfa.finals_count = 1;

    nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(0), EPSILON, *create_state(a.start.state + 1)};
    nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(0), EPSILON, *create_state(b.start.state + offset)};

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(t.from.state + 1), t.symbol, *create_state(t.to.state + 1)};
    }

    for (int i = 0; i < b.transitions_count; ++i) {
        Transition t = b.transitions[i];
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(t.from.state + offset), t.symbol, *create_state(t.to.state + offset)};
    }

    for (int i = 0; i < a.finals_count; ++i)
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(a.finals[i].state + 1), EPSILON, *create_state(nfa.states - 1)};

    for (int i = 0; i < b.finals_count; ++i)
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(b.finals[i].state + offset), EPSILON, *create_state(nfa.states - 1)};
    return nfa;
}

NFA nfa_concat(NFA a, NFA b) {
    NFA nfa = {0};
    int offset = a.states;
    nfa.states = a.states + b.states;
    nfa.start = a.start;

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];
        nfa.transitions[nfa.transitions_count++] = (Transition){t.from, t.symbol, t.to};
    }

    for (int i = 0; i < a.finals_count; ++i)
        nfa.transitions[nfa.transitions_count++] = (Transition){a.finals[i], EPSILON, *create_state(b.start.state + offset)};

        for (int i = 0; i < b.transitions_count; ++i) {
        Transition t = b.transitions[i];
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(t.from.state + offset), t.symbol, *create_state(t.to.state + offset)};
    }

    for (int i = 0; i < b.finals_count; ++i)
        nfa.finals[nfa.finals_count++] = *create_state(b.finals[i].state + offset);
    return nfa;
}

NFA nfa_closure(NFA a) {
    NFA nfa = {0};
    nfa.states = a.states + 2;
    nfa.start = *create_state(0);
    int new_final = nfa.states - 1;
    nfa.finals[0] = *create_state(new_final);
    nfa.finals_count = 1;

    nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(0), EPSILON, *create_state(a.start.state + 1)};
    nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(0), EPSILON, *create_state(new_final)};

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(t.from.state + 1), t.symbol, *create_state(t.to.state + 1)};
    }

    for (int i = 0; i < a.finals_count; ++i) {
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(a.finals[i].state + 1), EPSILON, *create_state(a.start.state + 1)};
        nfa.transitions[nfa.transitions_count++] = (Transition){*create_state(a.finals[i].state + 1), EPSILON, *create_state(new_final)};
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