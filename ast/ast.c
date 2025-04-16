#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

ASTNode* create_program_node(ASTNode** statements, int count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_PROGRAM;
    node->data.program_node.statements = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        node->data.program_node.statements[i] = statements[i];
    }
    node->data.program_node.count = count;
    return node;
}

ASTNode* create_number_node(double value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->data.number_value = value;
    return node;
}

ASTNode* create_string_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING;
    node->data.string_value = value;
    return node;
}

ASTNode* create_variable_node(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE;
    node->data.variable_name = name;
    return node;
}

ASTNode* create_binary_op_node(Operator op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_OP;
    node->data.op_node.op = op;
    node->data.op_node.left = left;
    node->data.op_node.right = right;
    return node;
}

ASTNode* create_unary_op_node(Operator op, ASTNode* operand) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_OP;
    node->data.op_node.op = op;
    node->data.op_node.left = operand;
    node->data.op_node.right = NULL;
    return node;
}

ASTNode* create_assignment_node(char* var, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;
    node->data.op_node.op = OP_ADD; // No se usa realmente
    node->data.op_node.left = create_variable_node(var);
    node->data.op_node.right = value;
    return node;
}

ASTNode* create_print_node(ASTNode* expr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_PRINT;
    node->data.op_node.left = expr;  // Reusing op_node structure to store the expression to print
    node->data.op_node.right = NULL;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_VARIABLE:
            free(node->data.variable_name);
            break;
        case NODE_BINARY_OP:
        case NODE_UNARY_OP:
        case NODE_ASSIGNMENT:
        case NODE_PRINT:
            free_ast(node->data.op_node.left);
            free_ast(node->data.op_node.right);
            break;
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program_node.count; i++) {
                free_ast(node->data.program_node.statements[i]);
            }
            free(node->data.program_node.statements);
            break;
        default:
            break;
    }
    free(node);
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program:\n");
            for (int i = 0; i < node->data.program_node.count; i++) {
                print_ast(node->data.program_node.statements[i], indent + 1);
            }
            break;
        case NODE_NUMBER:
            printf("Number: %g\n", node->data.number_value);
            break;
        case NODE_STRING:
            printf("String: %s\n", node->data.string_value);
            break;
        case NODE_VARIABLE:
            printf("Variable: %s\n", node->data.variable_name);
            break;
        case NODE_BINARY_OP: {
            const char* op_str;
            switch (node->data.op_node.op) {
                case OP_ADD: op_str = "+"; break;
                case OP_SUB: op_str = "-"; break;
                case OP_MUL: op_str = "*"; break;
                case OP_DIV: op_str = "/"; break;
                case OP_POW: op_str = "^"; break;
                default: op_str = "?"; break;
            }
            printf("Binary Op: %s\n", op_str);
            print_ast(node->data.op_node.left, indent + 1);
            print_ast(node->data.op_node.right, indent + 1);
            break;
        }
        case NODE_UNARY_OP:
            printf("Unary Op: negate\n");
            print_ast(node->data.op_node.left, indent + 1);
            break;
        case NODE_ASSIGNMENT:
            printf("Assignment:\n");
            print_ast(node->data.op_node.left, indent + 1);
            print_ast(node->data.op_node.right, indent + 1);
            break;
        case NODE_PRINT:
            printf("Print:\n");
            print_ast(node->data.op_node.left, indent + 1);
            break;
    }
}