#include "match.h"
#include "dfa.h"
#include <string.h>

is_final_state(DFA* dfa, State* state) {
    for (int i = 0; i < dfa->finals_count; i++) {
        if (set_equals(&dfa->finals[i], state)) {
            return 1;
        }
    }

    return 0;
}

String_Match* match(DFA* dfa, char* str) {
    String_Match* match = (String_Match*)malloc(sizeof(String_Match));
    match->matched = 0;
    
    int length = strlen(str);
    State actual_state = dfa->start;
    int change = 1;

    Lexer_Token tokens[3000];
    int count = 0;
    int last_index = 0;

    Lexer_Token token = *(Lexer_Token*)malloc(sizeof(Lexer_Token));
    Lexer_Token* actual_token = (Lexer_Token*)malloc(sizeof(Lexer_Token));

    for (int i = 0; i < length; i++) {
        // printf("str[i]: %c\n", str[i]);
        if (!change) {
            if (strcmp(actual_token->lexeme, "")) {
                strcpy(token.lexeme, actual_token->lexeme);
                token.matches = actual_state.matches;
                for (int k = 0; k < actual_token->matches; k++) {
                    token.token[k] = actual_token->token[k];
                }
                tokens[count++] = token;
                actual_token = NULL;
                actual_token = (Lexer_Token*)malloc(sizeof(Lexer_Token));
                actual_state = dfa->start;
                i = last_index;
            } else {
                printf("!!LEXICAL ERROR: Unexpected caracter %c\n", str[i-1]);
                return NULL;
            }
        }

        if (str[i] == ' ' || str[i] == '\n' || str[i] == '\r' || str[i] == '\t') {
            if (strcmp(actual_token->lexeme, "")) {
                strcpy(token.lexeme, actual_token->lexeme);
                token.matches = actual_state.matches;
                for (int k = 0; k < actual_token->matches; k++) {
                    token.token[k] = actual_token->token[k];
                }
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
                change = 1;

                if (is_final_state(dfa, &transition.to)) {
                    int len_lexeme = strlen(actual_token->lexeme);
                    actual_token->lexeme[len_lexeme] = str[i];
                    actual_token->matches = transition.to.matches;
                    for (int k = 0; k < transition.to.matches; k++) {
                        actual_token->token[k] = transition.to.tokens[k];
                    }
                    last_index = i + 1; 
                }

                copy_state_set(&actual_state, &(transition.to));
                break;
            }
        }
    }

    if (change && strcmp(actual_token->lexeme, "")) {
        for (int i = 0; i < dfa->finals_count; i++) {
            if (set_equals(&actual_state, &(dfa->finals[i]))) {
                actual_token->matches = dfa->finals[i].matches;
                for (int k = 0; k < dfa->finals[i].matches; k++) {
                    strcpy(actual_token->token[k], dfa->finals[i].tokens[k]);
                }
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