#ifndef AST_H
#define AST_H

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGNMENT,
    NODE_PRINT,
    NODE_STRING,
    NODE_PROGRAM
} NodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,
    OP_NEGATE
} Operator;

typedef struct ASTNode {
    NodeType type;
    union {
        double number_value;
        char* string_value;
        char* variable_name;
        struct {
            Operator op;
            struct ASTNode *left;
            struct ASTNode *right;
        } op_node;
        struct {
            struct ASTNode** statements;
            int count;
        } program_node;
    } data;
} ASTNode;

ASTNode* create_program_node(ASTNode** statements, int count);
ASTNode* create_number_node(double value);
ASTNode* create_string_node(char* value);
ASTNode* create_variable_node(char* name);
ASTNode* create_binary_op_node(Operator op, ASTNode* left, ASTNode* right);
ASTNode* create_unary_op_node(Operator op, ASTNode* operand);
ASTNode* create_assignment_node(char* var, ASTNode* value);
ASTNode* create_print_node(ASTNode* expr);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

#endif