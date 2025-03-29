#pragma once

typedef enum {
    // Palabras clave
    TOK_FUNCTION = 256,
    TOK_LET,
    TOK_IN,
    TOK_IF,
    TOK_ELIF,
    TOK_ELSE,
    TOK_TRUE,
    TOK_FALSE,
    TOK_PI,
    TOK_E,
    
    // Identificadores y literales
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    
    // Operadores
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_POWER,
    TOK_MODULO,
    TOK_CONCAT,       // @
    TOK_DOUBLE_CONCAT, // @@
    TOK_ASSIGN,        // =
    TOK_REASSIGN,      // :=
    TOK_EQUAL,         // ==
    TOK_NOT_EQUAL,     // !=
    TOK_LESS,
    TOK_GREATER,
    TOK_LESS_EQUAL,
    TOK_GREATER_EQUAL,
    TOK_AND,           // &
    TOK_OR,            // |
    TOK_NOT,           // !
    
    // Símbolos
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_ARROW,         // =>
    
    TOK_END_OF_FILE
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;