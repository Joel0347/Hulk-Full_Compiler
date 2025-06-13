#ifndef TOKEN_H
#define TOKEN_H

typedef struct {
    char lexeme[256];
    char* token[50];
    int matches;
} Lexer_Token;

#endif