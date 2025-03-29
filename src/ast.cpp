#include "ast.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// =============================================
// Funciones para creación de nodos del AST
// =============================================

ASTNode* create_program_node(ASTNode* functions, ASTNode* main_expr) {
    ASTNode* node = new ASTNode;
    node->type = NODE_PROGRAM;
    node->data.children.left = functions;
    node->data.children.right = main_expr;
    return node;
}

ASTNode* create_block_node(ASTNode* expression_list) {
    ASTNode* node = new ASTNode;
    node->type = NODE_BLOCK;
    node->data.list = new std::vector<ASTNode*>();
    if (expression_list) {
        // Flatten the expression list into the block
        ASTNode* current = expression_list;
        while (current) {
            if (current->type == NODE_EXPRESSION_LIST) {
                node->data.list->push_back(current->data.children.left);
                current = current->data.children.right;
            } else {
                node->data.list->push_back(current);
                break;
            }
        }
    }
    return node;
}

ASTNode* create_expression_list_node(ASTNode* first, ASTNode* rest) {
    ASTNode* node = new ASTNode;
    node->type = NODE_EXPRESSION_LIST;
    node->data.children.left = first;
    node->data.children.right = rest;
    return node;
}

ASTNode* create_function_def_node(const char* name, ASTNode* params, ASTNode* body) {
    ASTNode* node = new ASTNode;
    node->type = NODE_FUNCTION_DEF;
    node->data.function_def.function_name = strdup(name);
    node->data.function_def.params = params;
    node->data.function_def.body = body;
    return node;
}

ASTNode* create_param_list_node(ASTNode* first, ASTNode* rest) {
    ASTNode* node = new ASTNode;
    node->type = NODE_PARAM_LIST;
    node->data.children.left = first;
    node->data.children.right = rest;
    return node;
}

ASTNode* create_let_in_node(ASTNode* declarations, ASTNode* body) {
    ASTNode* node = new ASTNode;
    node->type = NODE_LET_IN;
    node->data.let_in.declarations = declarations;
    node->data.let_in.body = body;
    return node;
}

ASTNode* create_declaration_list_node(ASTNode* first, ASTNode* rest) {
    ASTNode* node = new ASTNode;
    node->type = NODE_ARG_LIST; // Reutilizamos el tipo para listas de declaraciones
    node->data.children.left = first;
    node->data.children.right = rest;
    return node;
}

ASTNode* create_declaration_node(const char* id, ASTNode* expr) {
    ASTNode* node = new ASTNode;
    node->type = NODE_VAR_DECLARATION;
    node->data.function_call.function_name = strdup(id);
    node->data.function_call.arguments = expr;
    return node;
}

ASTNode* create_if_node(ASTNode* condition, ASTNode* then_branch, ASTNode* elif_branches, ASTNode* else_branch) {
    ASTNode* node = new ASTNode;
    node->type = NODE_IF_EXPR;
    node->data.conditional.condition = condition;
    node->data.conditional.then_branch = then_branch;
    node->data.conditional.elif_branches = elif_branches;
    node->data.conditional.else_branch = else_branch;
    return node;
}

ASTNode* create_elif_node(ASTNode* condition, ASTNode* then_branch, ASTNode* next_elif) {
    ASTNode* node = new ASTNode;
    node->type = NODE_ELIF_EXPR;
    node->data.conditional.condition = condition;
    node->data.conditional.then_branch = then_branch;
    node->data.conditional.elif_branches = next_elif;
    return node;
}

ASTNode* create_function_call_node(const char* name, ASTNode* args) {
    ASTNode* node = new ASTNode;
    node->type = NODE_FUNCTION_CALL;
    node->data.function_call.function_name = strdup(name);
    node->data.function_call.arguments = args;
    return node;
}

ASTNode* create_arg_list_node(ASTNode* first, ASTNode* rest) {
    ASTNode* node = new ASTNode;
    node->type = NODE_ARG_LIST;
    node->data.children.left = first;
    node->data.children.right = rest;
    return node;
}

ASTNode* create_binary_op_node(Operator op, ASTNode* left, ASTNode* right) {
    ASTNode* node = new ASTNode;
    node->type = NODE_BINARY_OP;
    node->data.op = op;
    node->data.children.left = left;
    node->data.children.right = right;
    return node;
}

ASTNode* create_unary_op_node(Operator op, ASTNode* operand) {
    ASTNode* node = new ASTNode;
    node->type = NODE_UNARY_OP;
    node->data.op = op;
    node->data.children.left = operand;
    return node;
}

ASTNode* create_number_node(double value) {
    ASTNode* node = new ASTNode;
    node->type = NODE_NUMBER;
    node->data.number_value = value;
    return node;
}

ASTNode* create_string_node(const char* value) {
    ASTNode* node = new ASTNode;
    node->type = NODE_STRING;
    node->data.string_value = strdup(value);
    return node;
}

ASTNode* create_identifier_node(const char* name) {
    ASTNode* node = new ASTNode;
    node->type = NODE_IDENTIFIER;
    node->data.string_value = strdup(name);
    return node;
}

ASTNode* create_boolean_node(bool value) {
    ASTNode* node = new ASTNode;
    node->type = NODE_BOOLEAN;
    node->data.boolean_value = value;
    return node;
}

ASTNode* create_pi_node() {
    ASTNode* node = new ASTNode;
    node->type = NODE_PI;
    return node;
}

ASTNode* create_e_node() {
    ASTNode* node = new ASTNode;
    node->type = NODE_E;
    return node;
}

ASTNode* create_empty_node() {
    ASTNode* node = new ASTNode;
    node->type = NODE_EMPTY;
    return node;
}

ASTNode* create_assignment_node(const char* id, ASTNode* expr) {
    ASTNode* node = new ASTNode;
    node->type = NODE_ASSIGNMENT;
    node->data.function_call.function_name = strdup(id);
    node->data.function_call.arguments = expr;
    return node;
}

// =============================================
// Funciones de utilidad para el AST
// =============================================

const char* operator_to_string(Operator op) {
    switch(op) {
        case OP_PLUS: return "+";
        case OP_MINUS: return "-";
        case OP_MULTIPLY: return "*";
        case OP_DIVIDE: return "/";
        case OP_POWER: return "**";
        case OP_MODULO: return "%";
        case OP_XOR: return "^";
        case OP_CONCAT: return "@";
        case OP_DCONCAT: return "@@";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LEQ: return "<=";
        case OP_GEQ: return ">=";
        case OP_AND: return "&";
        case OP_OR: return "|";
        case OP_NOT: return "!";
        case OP_ASSIGN: return ":=";
        case OP_ARROW: return "=>";
        default: return "?";
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;

    switch(node->type) {
        case NODE_STRING:
        case NODE_IDENTIFIER:
            delete[] node->data.string_value;
            break;
            
        case NODE_FUNCTION_DEF:
            delete[] node->data.function_def.function_name;
            free_ast(node->data.function_def.params);
            free_ast(node->data.function_def.body);
            break;
            
        case NODE_FUNCTION_CALL:
            delete[] node->data.function_call.function_name;
            free_ast(node->data.function_call.arguments);
            break;
            
        case NODE_BLOCK:
            for (ASTNode* expr : *node->data.list) {
                free_ast(expr);
            }
            delete node->data.list;
            break;
            
        case NODE_BINARY_OP:
        case NODE_EXPRESSION_LIST:
        case NODE_ARG_LIST:
        case NODE_PARAM_LIST:
            free_ast(node->data.children.left);
            free_ast(node->data.children.right);
            break;
            
        case NODE_UNARY_OP:
        case NODE_LET_IN:
        case NODE_IF_EXPR:
        case NODE_ELIF_EXPR:
        case NODE_VAR_DECLARATION:
        case NODE_ASSIGNMENT:
            free_ast(node->data.children.left);
            break;
            
        default:
            // Tipos que no necesitan liberación especial
            break;
    }
    
    delete node;
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    std::string indent_str(indent, ' ');
    
    switch(node->type) {
        case NODE_PROGRAM:
            std::cout << indent_str << "Program:\n";
            print_ast(node->data.children.left, indent + 2);
            print_ast(node->data.children.right, indent + 2);
            break;
            
        case NODE_FUNCTION_DEF:
            std::cout << indent_str << "FunctionDef: " << node->data.function_def.function_name << "\n";
            print_ast(node->data.function_def.params, indent + 2);
            print_ast(node->data.function_def.body, indent + 2);
            break;
            
        case NODE_FUNCTION_CALL:
            std::cout << indent_str << "FunctionCall: " << node->data.function_call.function_name << "\n";
            print_ast(node->data.function_call.arguments, indent + 2);
            break;
            
        case NODE_BLOCK: {
            std::cout << indent_str << "Block:\n";
            for (ASTNode* expr : *node->data.list) {
                print_ast(expr, indent + 2);
            }
            break;
        }
            
        case NODE_EXPRESSION_LIST:
            print_ast(node->data.children.left, indent);
            print_ast(node->data.children.right, indent);
            break;
            
        case NODE_LET_IN:
            std::cout << indent_str << "LetIn:\n";
            print_ast(node->data.let_in.declarations, indent + 2);
            print_ast(node->data.let_in.body, indent + 2);
            break;
            
        case NODE_IF_EXPR:
            std::cout << indent_str << "If:\n";
            print_ast(node->data.conditional.condition, indent + 2);
            std::cout << indent_str << "Then:\n";
            print_ast(node->data.conditional.then_branch, indent + 4);
            print_ast(node->data.conditional.elif_branches, indent + 2);
            std::cout << indent_str << "Else:\n";
            print_ast(node->data.conditional.else_branch, indent + 4);
            break;
            
        case NODE_ELIF_EXPR:
            std::cout << indent_str << "Elif:\n";
            print_ast(node->data.conditional.condition, indent + 2);
            print_ast(node->data.conditional.then_branch, indent + 4);
            print_ast(node->data.conditional.elif_branches, indent + 2);
            break;
            
        case NODE_BINARY_OP:
            std::cout << indent_str << "BinaryOp: " << operator_to_string(node->data.op) << "\n";
            print_ast(node->data.children.left, indent + 2);
            print_ast(node->data.children.right, indent + 2);
            break;
            
        case NODE_UNARY_OP:
            std::cout << indent_str << "UnaryOp: " << operator_to_string(node->data.op) << "\n";
            print_ast(node->data.children.left, indent + 2);
            break;
            
        case NODE_NUMBER:
            std::cout << indent_str << "Number: " << node->data.number_value << "\n";
            break;
            
        case NODE_STRING:
            std::cout << indent_str << "String: " << node->data.string_value << "\n";
            break;
            
        case NODE_IDENTIFIER:
            std::cout << indent_str << "Identifier: " << node->data.string_value << "\n";
            break;
            
        case NODE_BOOLEAN:
            std::cout << indent_str << "Boolean: " << (node->data.boolean_value ? "true" : "false") << "\n";
            break;
            
        case NODE_PI:
            std::cout << indent_str << "PI\n";
            break;
            
        case NODE_E:
            std::cout << indent_str << "E\n";
            break;
            
        case NODE_ARG_LIST:
        case NODE_PARAM_LIST:
            print_ast(node->data.children.left, indent);
            print_ast(node->data.children.right, indent);
            break;
            
        case NODE_VAR_DECLARATION:
            std::cout << indent_str << "VarDecl: " << node->data.function_call.function_name << "\n";
            print_ast(node->data.function_call.arguments, indent + 2);
            break;
            
        case NODE_ASSIGNMENT:
            std::cout << indent_str << "Assignment: " << node->data.function_call.function_name << "\n";
            print_ast(node->data.function_call.arguments, indent + 2);
            break;
            
        case NODE_EMPTY:
            std::cout << indent_str << "Empty\n";
            break;
    }
}