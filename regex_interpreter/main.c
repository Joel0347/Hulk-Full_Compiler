#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "match.h"
#include "lexer.h"

FILE *file;

// read content from file
char* read_entire_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    char buffer[4096];
    char* content = NULL;
    size_t total_size = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        size_t chunk_size = strlen(buffer);
        content = realloc(content, total_size + chunk_size + 1);
        if (!content) {
            perror("Error en realloc");
            fclose(file);
            return NULL;
        }
        strcpy(content + total_size, buffer);
        total_size += chunk_size;
    }

    fclose(file);
    return content;
}

// main function
int main() {
    Regex* regex_semicolon = converTo_regex(";", "SEMICOLON");
    Regex* regex_comma = converTo_regex(",", "COMMA");
    Regex* regex_number_decimal = converTo_regex("[0-9]+\\.[0-9]+", "DECIMAL");
    Regex* regex_number = converTo_regex("[0-9]+", "NUMBER");
    Regex* regex_dot = converTo_regex("\\.", "DOT");
    Regex* regex_plusequal = converTo_regex("\\+=", "PLUSEQUAL");
    Regex* regex_minusequal = converTo_regex("\\-=", "MINUSEQUAL");
    Regex* regex_timeequal = converTo_regex("\\*=", "TIMESEQUAL");
    Regex* regex_divequal = converTo_regex("/=", "DIVEQUAL");
    Regex* regex_modequal = converTo_regex("%=", "MODEQUAL"); 
    Regex* regex_powequal = converTo_regex("\\^=", "POWEQUAL"); 
    Regex* regex_plus = converTo_regex("\\+", "PLUS"); 
    Regex* regex_minus = converTo_regex("\\-", "MINUS"); 
    Regex* regex_times = converTo_regex("\\*", "TIMES"); 
    Regex* regex_divide = converTo_regex("/", "DIVIDE"); 
    Regex* regex_mod = converTo_regex("%", "MOD"); 
    Regex* regex_power = converTo_regex("\\^", "POWER"); 
    Regex* regex_lparen = converTo_regex("\\(", "LPAREN"); 
    Regex* regex_rparen = converTo_regex("\\)", "RPAREN"); 
    Regex* regex_lbracket = converTo_regex("{", "LBRACKET"); 
    Regex* regex_rbracket = converTo_regex("}", "RBRACKET"); 
    Regex* regex_question = converTo_regex("\\?", "QUESTION"); 
    Regex* regex_andequal = converTo_regex("&=", "ANDEQUAL"); 
    Regex* regex_orequal = converTo_regex("\\|=", "OREQUAL"); 
    Regex* regex_concatequal = converTo_regex("@=", "CONCATEQUAL"); 
    Regex* regex_dequals = converTo_regex(":=", "DEQUALS"); 
    Regex* regex_colon = converTo_regex(":", "COLON"); 
    Regex* regex_equalsequals = converTo_regex("==", "EQUALSEQUALS"); 
    Regex* regex_arrow = converTo_regex("=>", "ARROW"); 
    Regex* regex_equals = converTo_regex("=", "EQUALS"); 
    Regex* regex_dconcat = converTo_regex("@@", "DCONCAT"); 
    Regex* regex_concat = converTo_regex("@", "CONCAT"); 
    Regex* regex_nequals = converTo_regex("!=", "NEQUALS");
    Regex* regex_not = converTo_regex("!", "NOT"); 
    Regex* regex_and = converTo_regex("&", "AND"); 
    Regex* regex_or = converTo_regex("\\|", "OR"); 
    Regex* regex_egreater = converTo_regex(">=", "EGREATER"); 
    Regex* regex_greater = converTo_regex(">", "GREATER"); 
    Regex* regex_eless = converTo_regex("<=", "ELESS");
    Regex* regex_less = converTo_regex("<", "LESS");
    Regex* regex_pi = converTo_regex("PI", "PI"); 
    Regex* regex_e = converTo_regex("E", "E"); 
    Regex* regex_boolean = converTo_regex("true|false", "BOOLEAN"); 
    Regex* regex_function = converTo_regex("function", "FUNCTION"); 
    Regex* regex_let = converTo_regex("let", "LET"); 
    Regex* regex_in = converTo_regex("in", "IN");
    Regex* regex_if = converTo_regex("if", "IF"); 
    Regex* regex_elif = converTo_regex("elif", "ELIF");
    Regex* regex_else = converTo_regex("else", "ELSE");
    Regex* regex_while = converTo_regex("while", "WHILE");
    Regex* regex_as = converTo_regex("as", "AS");
    Regex* regex_is = converTo_regex("is", "IS");
    Regex* regex_type = converTo_regex("type", "TYPE"); 
    Regex* regex_inherits = converTo_regex("inherits", "INHERITS");
    Regex* regex_new = converTo_regex("new", "NEW"); 
    Regex* regex_base = converTo_regex("base", "BASE");
    Regex* regex_for = converTo_regex("for", "FOR"); 
    Regex* regex_range = converTo_regex("range", "RANGE");
    Regex* regex_id = converTo_regex("[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*", "ID");

    Regex* regex_list[] = {
        regex_semicolon, regex_comma, regex_number, regex_number_decimal, regex_dot, regex_plus, regex_minus,
        regex_times, regex_divide, regex_mod, regex_power, regex_lparen, regex_rparen, regex_lbracket,
        regex_rbracket, regex_question, regex_plusequal, regex_minusequal, regex_timeequal, regex_divequal,
        regex_modequal, regex_powequal, regex_andequal, regex_orequal, regex_concatequal, regex_dequals,
        regex_colon, regex_equalsequals, regex_arrow, regex_equals, regex_dconcat, regex_concat, regex_nequals,
        regex_not, regex_and, regex_or, regex_egreater, regex_greater, regex_eless, regex_less, regex_pi,
        regex_e, regex_boolean, regex_function, regex_let, regex_in, regex_if, regex_elif, regex_else, regex_while,
        regex_as, regex_is, regex_type, regex_inherits, regex_new, regex_base, regex_for, regex_range, regex_id
    };

    int token_count = sizeof(regex_list) / sizeof(regex_list[0]);

    NFA nfa = regex_list[0]->nfa;
    regex_list[0]->priority = 0;

    for (int i = 1; i < token_count; i++) {
        regex_list[i]->priority = i;
        nfa = nfa_union_BIG(regex_list[i]->nfa, nfa);
    }

    DFA* dfa = nfa_to_dfa(&nfa);

    char* file_info = read_entire_file("script.txt");
    char* content = strcat(file_info, " ");
    String_Match* matched = match(dfa, content);

    if (matched) {
        Lexer_Token* tokens = (Lexer_Token*)malloc(sizeof(Lexer_Token) * matched->count);
        int index = 0;

        for (int i = 0; i < matched->count; i++) {
            int priority = token_count + 1;
            Lexer_Token lexer_token = *(Lexer_Token*)malloc(sizeof(Lexer_Token));
            lexer_token.matches = 1;
            strcpy(lexer_token.lexeme, matched->tokens[i].lexeme);
            
            if (matched->tokens[i].matches > 1) {
                for (int j = 0; j < matched->tokens[i].matches; j++) {
                    char* token = strdup(matched->tokens[i].token[j]);
                    Regex* regex = find_regex_by_token(regex_list, token, token_count);

                    if (match_nfa(&regex->nfa, &regex->nfa.start, lexer_token.lexeme, 0) && regex->priority < priority) {
                        lexer_token.token[0] = strdup(token);
                        priority = regex->priority;
                    }
                }
            } else {
                lexer_token.token[0] = strdup(matched->tokens[i].token[0]);
            }

            tokens[index++] = lexer_token;
        }

        for (int i = 0; i < matched->count; i++) {
            printf("lexeme: %s, token: %s\n", tokens[i].lexeme, tokens[i].token[0]);
        }
    }

    return 0;
}