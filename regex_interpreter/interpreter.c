#include "interpreter.h"

// operator [ - ]
char* expand_set(const char *set, bool is_negated) {
    char *expanded = malloc(MAX_SET_LENGTH * 2);
    int pos = 0;
    int len = strlen(set);
    
    for (int i = 0; i < len; ++i) {
        if (set[i] == '-' && i > 0 && i < len - 1) {
            char start = set[i - 1];
            char end = set[i + 1];
            if (start <= end) {
                for (char c = start + 1; c <= end; ++c) {
                    expanded[pos++] = c;
                }
                i++;
            } else {
                expanded[pos++] = '-';
            }
        } else {
            expanded[pos++] = set[i];
        }
    }
    expanded[pos] = '\0';

    if (is_negated) {
        char universe[128];
        int uni_pos = 0;
        for (char c = 32; c <= 126; ++c) universe[uni_pos++] = c;
        universe[uni_pos] = '\0';

        char *complement = malloc(256);
        int comp_pos = 0;
        for (int i = 0; universe[i] != '\0'; ++i) {
            if (!strchr(expanded, universe[i])) {
                complement[comp_pos++] = universe[i];
            }
        }
        complement[comp_pos] = '\0';
        free(expanded);
        return complement;
    }

    return expanded;
}

// tokenize a regular expression
void tokenize(const char *input, Token *tokens, int *count) {
    int i = 0, j = 0;
    while (input[i]) {
        if (input[i] == ' ' || input[i] == '\t') { i++; continue; }
        if (input[i] == '\\') {
            if (input[i+1]) {
                tokens[j].type = 0;
                tokens[j].value.symbol = input[i + 1];
                j++;
                i += 2;  
            } else {
                tokens[j].type = 0;
                tokens[j].value.symbol = input[i];
                j++;
                i++;
            }
        } else if (input[i] == '|') {
            tokens[j].type = 1;
            tokens[j].value.symbol = '|';
            j++;
            i++;
        } else if (input[i] == '*') {
            tokens[j].type = 2;
            tokens[j].value.symbol = '*';
            j++;
            i++;
        } else if (input[i] == '+') {
            tokens[j].type = 8;
            tokens[j].value.symbol = '+';
            j++;
            i++;
        } else if (input[i] == '(') {
            tokens[j].type = 3;
            tokens[j].value.symbol = '(';
            j++;
            i++;
        } else if (input[i] == ')') {
            tokens[j].type = 4;
            tokens[j].value.symbol = ')';
            j++;
            i++;
        } else if (input[i] == EPSILON) {
            tokens[j].type = 5;
            tokens[j].value.symbol = EPSILON;
            j++;
            i++;
        } else if (input[i] == '[') {
            int is_negated = 0;
            i++;
            if (input[i] == '^') {
                is_negated = 1;
                i++;
            }
            tokens[j].type = is_negated ? 8 : 7;
            int k = 0;
            while (input[i] != ']' && input[i] != '\0' && k < MAX_SET_LENGTH - 1) {
                tokens[j].value.set[k++] = input[i++];
            }
            tokens[j].value.set[k] = '\0';
            if (input[i] == ']') i++;
            j++;
        } else if (input[i] == '.') {
            tokens[j].type = 9;
            tokens[j].value.symbol = '.';
            j++;
            i++;
        } else {
            tokens[j].type = 0;
            tokens[j].value.symbol = input[i];
            j++;
            i++;
        }
    }
    tokens[j].type = 6;
    *count = j + 1;
}

/*
    Recursive Descent Parser for regex grammar
    E -> T X
    X -> | T X | ε
    T -> F Y
    Y -> F Y | ε
    F -> A Z
    Z -> * Z | + Z | . Z | ε
    A -> ( E ) | [ E ] | symbol | ε
*/

Node *parse_E(ParserState *ps) {
    Node *t = parse_T(ps);
    return parse_X(ps, t);
}

Node *parse_X(ParserState *ps, Node *left) {
    if (ps->tokens[ps->pos].type == 1) { // |
        ps->pos++;
        Node *t = parse_T(ps);
        Node *u = parse_X(ps, t);
        Node *n = malloc(sizeof(Node));
        n->type = NODE_UNION; n->left = left; n->right = u;
        return n;
    }
    return left;
}

Node *parse_T(ParserState *ps) {
    Node *f = parse_F(ps);
    return parse_Y(ps, f);
}

Node *parse_Y(ParserState *ps, Node *left) {
    if (ps->tokens[ps->pos].type == 0 ||
        ps->tokens[ps->pos].type == 3 ||
        ps->tokens[ps->pos].type == 5 ||
        ps->tokens[ps->pos].type == 7 ||
        ps->tokens[ps->pos].type == 8 ||
        ps->tokens[ps->pos].type == 9
    ) {
        Node *f = parse_F(ps);
        Node *c = malloc(sizeof(Node));
        c->type = NODE_CONCAT; c->left = left; c->right = parse_Y(ps, f);
        return c;
    }
    
    return left;
}

Node *parse_F(ParserState *ps) {
    Node *a = parse_A(ps);
    return parse_Z(ps, a);
}

Node *parse_Z(ParserState *ps, Node *left) {
    if (ps->tokens[ps->pos].type == 2) { // '*'
        ps->pos++;
        Node *n = malloc(sizeof(Node));
        n->type = NODE_CLOSURE; n->left = left; n->right = NULL;
        return parse_Z(ps, n);
    }
    if (ps->tokens[ps->pos].type == 8) { // '+'
        ps->pos++;
        Node *closure_node = malloc(sizeof(Node));
        closure_node->type = NODE_CLOSURE;
        closure_node->left = left;
        closure_node->right = NULL;
        Node *concat_node = malloc(sizeof(Node));
        concat_node->type = NODE_CONCAT;
        concat_node->left = left;
        concat_node->right = closure_node;
        return parse_Z(ps, concat_node);
    }
    if (ps->tokens[ps->pos].type == 9) {
        ps->pos++;
        Node *n = malloc(sizeof(Node));
        n->type = NODE_ANY;
        n->left = n->right = NULL;
        return n;
    }

    return left;
}

Node *parse_A(ParserState *ps) {
    if (ps->tokens[ps->pos].type == 3) { // '('
        ps->pos++;
        Node *e = parse_E(ps);
        if (ps->tokens[ps->pos].type == 4) ps->pos++; // ')'
        return e;
    }
    if (ps->tokens[ps->pos].type == 0) { // symbol
        Node *n = malloc(sizeof(Node));
        n->type = NODE_SYMBOL; n->symbol = ps->tokens[ps->pos].value.symbol;
        n->left = n->right = NULL;
        ps->pos++;
        return n;
    }
    if (ps->tokens[ps->pos].type == 5) { // epsilon
        Node *n = malloc(sizeof(Node));
        n->type = NODE_EPSILON; n->left = n->right = NULL;
        ps->pos++;
        return n;
    }
    if (ps->tokens[ps->pos].type == 7 || ps->tokens[ps->pos].type == 8) { // Set o negated set
        bool is_negated = (ps->tokens[ps->pos].type == 8);
        char *set = ps->tokens[ps->pos].value.set;
        ps->pos++;
        char *expanded = expand_set(set, is_negated);
        Node *union_node = NULL;
        for (int i = 0; expanded[i] != '\0'; ++i) {
            Node *symbol_node = malloc(sizeof(Node));
            symbol_node->type = NODE_SYMBOL;
            symbol_node->symbol = expanded[i];
            symbol_node->left = symbol_node->right = NULL;
            if (!union_node) {
                union_node = symbol_node;
            } else {
                Node *temp = malloc(sizeof(Node));
                temp->type = NODE_UNION;
                temp->left = union_node;
                temp->right = symbol_node;
                union_node = temp;
            }
        }
        free(expanded);
        return union_node;
    }

    return NULL;
}

// Print AST (preorder)
void print_ast(Node *n, int depth) {
    if (!n) return;
    for (int i = 0; i < depth; ++i) printf("  ");
    switch (n->type) {
        case NODE_SYMBOL: printf("Symbol(%c)\n", n->symbol); break;
        case NODE_EPSILON: printf("Epsilon\n"); break;
        case NODE_UNION: printf("Union\n"); break;
        case NODE_CONCAT: printf("Concat\n"); break;
        case NODE_CLOSURE: printf("Closure\n"); break;
        case NODE_ANY: printf("AnyChar\n"); break; 
    }
    print_ast(n->left, depth + 1);
    print_ast(n->right, depth + 1);
}