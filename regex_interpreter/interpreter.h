#ifndef INTERPRETER_H
#define INTERPRETER_H

#define EPSILON 'ε'
#define MAX_SYMBOLS 128
#define MAX_STACK 256
#define MAX_SET_LENGTH 128

typedef enum { 
    NODE_SYMBOL,
    NODE_EPSILON,
    NODE_UNION,
    NODE_CONCAT,
    NODE_CLOSURE,
    NODE_ANY
} NodeType;

typedef struct Node {
    NodeType type;
    char symbol;
    struct Node *left, *right;
} Node;

typedef struct {
    int type;
    union {
        char symbol;
        char set[MAX_SET_LENGTH];
    } value;
} Token;

typedef struct {
    Token *tokens;
    int pos;
} ParserState;

char* expand_set(const char *set, bool is_negated);
void tokenize(const char *input, Token *tokens, int *count);

Node *parse_E(ParserState *ps);
Node *parse_T(ParserState *ps);
Node *parse_X(ParserState *ps, Node *left);
Node *parse_Y(ParserState *ps, Node *left);
Node *parse_F(ParserState *ps);
Node *parse_Z(ParserState *ps, Node *left);
Node *parse_A(ParserState *ps);
void print_ast(Node *n, int depth);

#endif
