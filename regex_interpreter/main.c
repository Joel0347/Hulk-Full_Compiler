#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "match.h"
#include "lexer.h"

NFA nfa_union_BIG(NFA a, NFA b) {
    NFA nfa = {0};
    int offset = a.states + 1;
    nfa.states = a.states + b.states + 2;
    nfa.start = *create_state(0);
    nfa.start.token = "error";
    nfa.finals[0] = *create_state(nfa.states - 1);
    nfa.finals[0].token = "error";
    nfa.finals_count = 1;

    NFA_State* state_a = create_state(a.start.state + 1);
    state_a->token = a.start.token;

    NFA_State* state_b = create_state(b.start.state + offset);
    state_b->token = b.start.token;

    nfa.transitions[nfa.transitions_count++] = (Transition){nfa.start, EPSILON, *state_a};
    nfa.transitions[nfa.transitions_count++] = (Transition){nfa.start, EPSILON, *state_b};

    for (int i = 0; i < a.transitions_count; ++i) {
        Transition t = a.transitions[i];

        NFA_State* state_from = create_state(t.from.state + 1);
        state_from->token = t.from.token;

        NFA_State* state_to = create_state(t.to.state + 1);
        state_to->token = t.to.token;

        nfa.transitions[nfa.transitions_count++] = (Transition){*state_from, t.symbol, *state_to};
    }

    for (int i = 0; i < b.transitions_count; ++i) {
        Transition t = b.transitions[i];

        NFA_State* state_from = create_state(t.from.state + offset);
        state_from->token = t.from.token;

        NFA_State* state_to = create_state(t.to.state + offset);
        state_to->token = t.to.token;

        nfa.transitions[nfa.transitions_count++] = (Transition){*state_from, t.symbol, *state_to};
    }

    for (int i = 0; i < a.finals_count; ++i) {
        NFA_State* state_final = create_state(a.finals[i].state + 1);
        state_final->token = a.finals[i].token;

        nfa.transitions[nfa.transitions_count++] = (Transition){*state_final, EPSILON, nfa.finals[0]};
    }

    for (int i = 0; i < b.finals_count; ++i) {
        NFA_State* state_final = create_state(b.finals[i].state + offset);
        state_final->token = b.finals[i].token;

        nfa.transitions[nfa.transitions_count++] = (Transition){*state_final, EPSILON, nfa.finals[0]};
    }
    
    return nfa;
}

int main() {
    Regex* regex = (Regex*)malloc(sizeof(Regex));
    regex->token = "number";
    regex->regex = "[0-9]+";

    Token tokens[256];
    int token_count = 0;
    tokenize(regex->regex, tokens, &token_count);

    ParserState ps = {tokens, 0};
    Node *ast = parse_E(&ps);
    NFA nfa = eval(ast);

    regex->nfa = &nfa;
    regex->nfa->start.token = regex->token;

    for (int i = 0; i < nfa.transitions_count; i++) {
        regex->nfa->transitions[i].from.token = regex->token;
        regex->nfa->transitions[i].to.token = regex->token;
    }

    for (int i = 0; i < nfa.finals_count; i++) {
        regex->nfa->finals[i].token = regex->token;
    }

    Regex* regex2 = (Regex*)malloc(sizeof(Regex));
    regex2->token = "ID";
    regex2->regex = "[a-z]+";

    Token tokens2[256];
    int token_count2 = 0;
    tokenize(regex2->regex, tokens2, &token_count2);

    ParserState ps2 = {tokens2, 0};
    Node *ast2 = parse_E(&ps2);
    NFA nfa2 = eval(ast2);

    regex2->nfa = &nfa2;
    regex2->nfa->start.token = regex2->token;

    for (int i = 0; i < nfa2.transitions_count; i++) {
        regex2->nfa->transitions[i].from.token = regex2->token;
        regex2->nfa->transitions[i].to.token = regex2->token;
    }

    for (int i = 0; i < nfa2.finals_count; i++) {
        regex2->nfa->finals[i].token = regex2->token;
    }

    NFA a = *(regex->nfa);
    NFA b = *(regex2->nfa);
    NFA nfa_un = nfa_union_BIG(a, b);

    // printf("\nNFA:\nStates: %d\nStart: %d\nFinals:", nfa_un.states, nfa_un.start);
    // for (int i = 0; i < nfa_un.finals_count; ++i) printf(" %d", nfa_un.finals[i]);
    // printf("\nTransitions:\n");
    // for (int i = 0; i < nfa_un.transitions_count; ++i) {
    //     printf("  %d --%c--> %d\n", nfa_un.transitions[i].from.state, nfa_un.transitions[i].symbol, nfa_un.transitions[i].to.state);
    // }

    // for (int i = 0; i < nfa_un.transitions_count; ++i) {
    //     printf("  %s --%c--> %s\n", nfa_un.transitions[i].from.token, nfa_un.transitions[i].symbol, nfa_un.transitions[i].to.token);
    // }

    // printf("\n");

    DFA* dfa = nfa_to_dfa(&nfa_un);

    String_Match* matched = match(dfa, "gscfcx99jii3  jeje");

    if (matched) {
        for (int i = 0; i < 5; i++) {
            printf("lexeme: %s, token: %s\n", matched->tokens[i].lexeme, matched->tokens[i].token);
        }
    }

    return 0;
}