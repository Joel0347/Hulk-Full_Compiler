#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode* create_program_node(ASTNode** statements, int count, NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = type;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.program_node.statements = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        node->data.program_node.statements[i] = statements[i];
    }
    node->data.program_node.count = count;
    return node;
}

ASTNode* create_number_node(double value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_NUMBER;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_NUMBER_INST;
    node->data.number_value = value;
    return node;
}

ASTNode* create_string_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_STRING;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_STRING_INST;
    node->data.string_value = value;
    return node;
}

ASTNode* create_boolean_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_BOOLEAN;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_BOOLEAN_INST;
    node->data.string_value = value;
    return node;
}

ASTNode* create_variable_node(char* name, char* type, int is_param) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->is_param = is_param;
    node->type = NODE_VARIABLE;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_OBJECT_INST;
    node->data.variable_name = name;

    if (type)
        node->static_type = type;

    return node;
}

ASTNode* create_binary_op_node(Operator op, char* op_name, ASTNode* left, ASTNode* right, Type* return_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_BINARY_OP;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = return_type;
    node->data.op_node.op_name = op_name;
    node->data.op_node.op = op;
    node->data.op_node.left = left;
    node->data.op_node.right = right;
    return node;
}

ASTNode* create_unary_op_node(Operator op, char* op_name, ASTNode* operand, Type* return_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_UNARY_OP;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = return_type;
    node->data.op_node.op_name = op_name;
    node->data.op_node.op = op;
    node->data.op_node.left = operand;
    node->data.op_node.right = NULL;
    return node;
}

ASTNode* create_assignment_node(char* var, ASTNode* value, char* type_name, NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = type;
    node->return_type = &TYPE_VOID_INST;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.op_node.left = create_variable_node(var, NULL, 0);
    node->data.op_node.left->static_type = type_name;
    node->data.op_node.right = value;
    node->value = value;
    return node;
}

ASTNode* create_func_call_node(char* name, ASTNode** args, int arg_count) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->line = line_num;
    node->type = NODE_FUNC_CALL;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->return_type = &TYPE_OBJECT_INST;
    node->data.func_node.name = name;
    node->data.func_node.checked = 0;
    node->data.func_node.args = malloc(sizeof(ASTNode*) * arg_count);
    for (int i = 0; i < arg_count; i++) {
        node->data.func_node.args[i] = args[i];
    }
    node->data.func_node.arg_count = arg_count;
    return node;
}

ASTNode* create_func_dec_node(char* name, ASTNode** args, int arg_count, ASTNode* body, char* ret_type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_FUNC_DEC;
    node->return_type = &TYPE_VOID_INST;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->static_type = ret_type;
    node->data.func_node.name = name;
    node->data.func_node.checked = 0;
    node->data.func_node.args = malloc(sizeof(ASTNode*) * arg_count);
    for (int i = 0; i < arg_count; i++) {
        node->data.func_node.args[i] = args[i];
    }
    node->data.func_node.arg_count = arg_count;
    node->data.func_node.body = body;
    return node;
}

ASTNode* create_let_in_node(ASTNode** declarations, int dec_count, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->line = line_num;
    node->type = NODE_LET_IN;
    node->return_type = &TYPE_OBJECT_INST;
    node->scope = create_scope(NULL);
    node->context = create_context(NULL);
    node->data.func_node.name = "";
    node->data.func_node.args = malloc(sizeof(ASTNode*) * dec_count);
    for (int i = 0; i < dec_count; i++) {
        node->data.func_node.args[i] = declarations[i];
    }
    node->data.func_node.arg_count = dec_count;
    node->data.func_node.body = body;
    node->value = body;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case NODE_VARIABLE:
            free(node->data.variable_name);
            break;
        case NODE_BINARY_OP:
        case NODE_UNARY_OP:
        case NODE_ASSIGNMENT:
        case NODE_D_ASSIGNMENT:
            free_ast(node->data.op_node.left);
            free_ast(node->data.op_node.right);
            break;
        case NODE_PROGRAM:
        case NODE_BLOCK:
            for (int i = 0; i < node->data.program_node.count; i++) {
                free_ast(node->data.program_node.statements[i]);
            }
            free(node->data.program_node.statements);
            break;
        case NODE_FUNC_CALL:
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                free_ast(node->data.func_node.args[i]);
            }
            break;
        case NODE_FUNC_DEC:
        case NODE_LET_IN:
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                free_ast(node->data.func_node.args[i]);
            }
            free_ast(node->data.func_node.body);
            break;
        default:
            break;
    }
    destroy_scope(node->scope);
    destroy_context(node->context);
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
        case NODE_BLOCK:
            printf("Block:\n");
            for (int i = 0; i < node->data.program_node.count; i++) {
                print_ast(node->data.program_node.statements[i], indent + 1);
            }
            break;
        case NODE_FUNC_CALL:
            printf("Function_call: %s, receives:\n", node->data.func_node.name);
            int arg_count = node->data.func_node.arg_count;
            for (int i = 0; i < arg_count; i++) {
                print_ast(node->data.func_node.args[i], indent + 1);
            }
            break;
        case NODE_FUNC_DEC:
            printf("Function_Declaration: %s, returns:\n", node->data.func_node.name);
            print_ast(node->data.func_node.body, indent + 1);
            break;
        case NODE_LET_IN:
            printf("Let-in: %s\n", node->data.func_node.name);
            print_ast(node->data.func_node.body, indent + 1);
            break;
        case NODE_NUMBER:
            printf("Number: %g\n", node->data.number_value);
            break;
        case NODE_STRING:
            printf("String: %s\n", node->data.string_value);
            break;
        case NODE_BOOLEAN:
            printf("Boolean: %s\n", node->data.string_value);
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
                case OP_MOD: op_str = "%"; break;
                case OP_POW: op_str = "^"; break;
                case OP_AND: op_str = "&"; break;
                case OP_OR: op_str = "|"; break;
                case OP_CONCAT: op_str = "@"; break;
                case OP_DCONCAT: op_str = "@@"; break;
                case OP_EQ: op_str = "=="; break;
                case OP_NEQ: op_str = "!="; break;
                case OP_GRE: op_str = ">="; break;
                case OP_GR: op_str = ">"; break;
                case OP_LSE: op_str = "<="; break;
                case OP_LS: op_str = "<"; break;
                default: op_str = "?"; break;
            }
            printf("Binary Op: %s\n", op_str);
            print_ast(node->data.op_node.left, indent + 1);
            print_ast(node->data.op_node.right, indent + 1);
            break;
        }
        case NODE_UNARY_OP:
            printf("Unary Op: %s\n", node->data.op_node.op_name);
            print_ast(node->data.op_node.left, indent + 1);
            break;
        case NODE_ASSIGNMENT:
        case NODE_D_ASSIGNMENT:
            printf("Assignment:\n");
            print_ast(node->data.op_node.left, indent + 1);
            print_ast(node->data.op_node.right, indent + 1);
            break;
    }
}