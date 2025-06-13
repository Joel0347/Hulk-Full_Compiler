#include <string.h>
#include "lexer.h"

Regex* converTo_regex(char* regex, char* token) {
    Regex* _regex = (Regex*)malloc(sizeof(Regex));
    strcpy(_regex->token, token);
    strcpy(_regex->regex, regex);
    _regex->priority = 0;

    Token tokens[1000];
    int token_count = 0;
    tokenize(_regex->regex, tokens, &token_count);

    ParserState ps = {tokens, 0};
    Node *ast = parse_E(&ps);
    NFA nfa = eval(ast);

    _regex->nfa = nfa;
    _regex->nfa.start.token = _regex->token;

    for (int i = 0; i < nfa.transitions_count; i++) {
        _regex->nfa.transitions[i].from.token = "error";
        _regex->nfa.transitions[i].to.token = "error";
    }

    for (int i = 0; i < nfa.finals_count; i++) {
        _regex->nfa.finals[i].token = _regex->token;
    }

    return _regex;
}

Regex* find_regex_by_token(Regex** list, char* token, int len) {
    for (int i = 0; i < len; i++) {
        if (!strcmp(list[i]->token, token)) {
            return list[i];
        }
    }

    return NULL;
}