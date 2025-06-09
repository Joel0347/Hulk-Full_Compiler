#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

Regex* converTo_regex(char* regex, char* token) {
    Regex* _regex = (Regex*)malloc(sizeof(Regex));
    strcpy(_regex->token, token);
    strcpy(_regex->regex, regex);

    Token tokens[1000];
    int token_count = 0;
    tokenize(_regex->regex, tokens, &token_count);

    ParserState ps = {tokens, 0};
    Node *ast = parse_E(&ps);
    NFA nfa = eval(ast);

    _regex->nfa = nfa;
    _regex->nfa.start.token = _regex->token;

    for (int i = 0; i < nfa.transitions_count; i++) {
        _regex->nfa.transitions[i].from.token = _regex->token;
        _regex->nfa.transitions[i].to.token = _regex->token;
    }

    for (int i = 0; i < nfa.finals_count; i++) {
        _regex->nfa.finals[i].token = _regex->token;
    }

    return _regex;
}

int main() {
    Regex* regex_number = converTo_regex("[0-9]+\\.[0-9]+", "number");
    Regex* regex_id = converTo_regex("[a-z]+", "ID");

    NFA nfa_un = nfa_union_BIG(regex_number->nfa, regex_id->nfa);

    // printf("\nNFA:\nStates: %d\nStart: %d\nFinals:", regex_number->nfa.states, regex_number->nfa.start);
    // for (int i = 0; i < regex_number->nfa.finals_count; ++i) printf(" %d", regex_number->nfa.finals[i]);
    // printf("\nTransitions:\n");
    // for (int i = 0; i < regex_number->nfa.transitions_count; ++i) {
    //     printf("  %d --%c--> %d\n", regex_number->nfa.transitions[i].from.state, regex_number->nfa.transitions[i].symbol, regex_number->nfa.transitions[i].to.state);
    // }

    // for (int i = 0; i < regex_number->nfa.transitions_count; ++i) {
    //     printf("  %s --%c--> %s\n", regex_number->nfa.transitions[i].from.token, regex_number->nfa.transitions[i].symbol, regex_number->nfa.transitions[i].to.token);
    // }

    // printf("\n");

    DFA* dfa = nfa_to_dfa(&nfa_un);

    String_Match* matched = match(dfa, "gscfcx 9.2 jii 3.0  jeje");

    if (matched) {
        for (int i = 0; i < matched->count; i++) {
            printf("lexeme: %s, token: %s\n", matched->tokens[i].lexeme, matched->tokens[i].token);

        }
    }

    return 0;
}