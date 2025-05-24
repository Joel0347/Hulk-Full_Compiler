#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "nfa.c"

int main() {
    char input[256] = "a+";
    Token tokens[256];
    int token_count = 0;
    tokenize(input, tokens, &token_count);

    ParserState ps = {tokens, 0};
    Node *ast = parse_E(&ps);

    printf("AST:\n");
    print_ast(ast, 0);

    NFA nfa = eval(ast);

    printf("\nNFA:\nStates: %d\nStart: %d\nFinals:", nfa.states, nfa.start);
    for (int i = 0; i < nfa.finals_count; ++i) printf(" %d", nfa.finals[i]);
    printf("\nTransitions:\n");
    for (int i = 0; i < nfa.transitions_count; ++i) {
        printf("  %d --%c--> %d\n", nfa.transitions[i].from, nfa.transitions[i].symbol, nfa.transitions[i].to);
    }

    return 0;
}