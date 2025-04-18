#ifndef AST_H
#define AST_H

#include "../type/type.h"
#include "../scope/scope.h"

extern int line_num;
typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGNMENT,
    NODE_PRINT,
    NODE_STRING,
    NODE_BOOLEAN,
    NODE_PROGRAM
} NodeType;

typedef struct ASTNode {
    int line;
    NodeType type;
    Type* return_type;
    Scope* scope;
    union {
        double number_value;
        char* string_value;
        char* variable_name;
        struct {
            Operator op;
            char* op_name;
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
ASTNode* create_boolean_node(char* value);
ASTNode* create_variable_node(char* name);
ASTNode* create_binary_op_node(Operator op, char* op_name, ASTNode* left, ASTNode* right, Type* return_type);
ASTNode* create_unary_op_node(Operator op, char* op_name, ASTNode* operand, Type* return_type);
ASTNode* create_assignment_node(char* var, ASTNode* value);
ASTNode* create_print_node(ASTNode* expr);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

#endif