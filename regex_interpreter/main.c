#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dfa.h"

int main() {
    // char input[256] = "[0-9]+\\.[0-9]+";
    char input[256] = "if";
    Token tokens[256];
    int token_count = 0;
    tokenize(input, tokens, &token_count);

    ParserState ps = {tokens, 0};
    Node *ast = parse_E(&ps);
    NFA nfa = eval(ast);

    printf("\nNFA:\nStates: %d\nStart: %d\nFinals:", nfa.states, nfa.start);
    for (int i = 0; i < nfa.finals_count; ++i) printf(" %d", nfa.finals[i]);
    printf("\nTransitions:\n");
    for (int i = 0; i < nfa.transitions_count; ++i) {
        printf("  %d --%c--> %d\n", nfa.transitions[i].from, nfa.transitions[i].symbol, nfa.transitions[i].to);
    }

    printf("\n");

    nfa_to_dfa(&nfa);

    return 0;
}