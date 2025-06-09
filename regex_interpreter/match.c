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
    
    int length = strlen(str);
    State actual_state = dfa->start;
    int change = 1;

    Lexer_Token tokens[1000];
    int count = 0;

    Lexer_Token token = *(Lexer_Token*)malloc(sizeof(Lexer_Token));
    Lexer_Token* actual_token = (Lexer_Token*)malloc(sizeof(Lexer_Token));

    for (int i = 0; i < length; i++) {
        if (!change) {
            if (strcmp(actual_token->lexeme, "")) {
                strcpy(token.lexeme, actual_token->lexeme);
                strcpy(token.token, actual_token->token);
                tokens[count++] = token;
                actual_token = NULL;
                actual_token = (Lexer_Token*)malloc(sizeof(Lexer_Token));
                actual_state = dfa->start;
                i--;
            } else {
                printf("!!LEXICAL ERROR: Unexpected caracter %c\n", str[i-1]);
                return NULL;
            }
        }

        if (str[i] == ' ') {
            if (strcmp(actual_token->lexeme, "")) {
                strcpy(token.lexeme, actual_token->lexeme);
                strcpy(token.token, actual_token->token);
                tokens[count++] = token;
                actual_token = NULL;
                actual_token = (Lexer_Token*)malloc(sizeof(Lexer_Token));
                actual_state = dfa->start;
                change = 1;
            }

            continue;
        }

        change = 0;

        for (int j = 0; j < dfa->transitions_count; j++) {
            DFA_Transition transition = dfa->transitions[j];

            if (set_equals(&actual_state, &(transition.from)) && str[i] == transition.symbol) {
                copy_state_set(&actual_state, &(transition.to));
                change = 1;
                int len_lexeme = strlen(actual_token->lexeme);
                actual_token->lexeme[len_lexeme] = str[i];
                strcpy(actual_token->token, actual_state.tokens[0]);
                break;
            }
        }
    }

    if (change) {
        for (int i = 0; i < dfa->finals_count; i++) {
            if (set_equals(&actual_state, &(dfa->finals[i]))) {
                match->matched = 1;

                strcpy(actual_token->token, dfa->finals[i].tokens[0]);
                tokens[count++] = *actual_token;
                break;
            }
        }
    }

    match->matched = count > 0;
    match->tokens = tokens;
    match->count = count;

    return match;
}