#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>

// Tipos de nodos del AST
typedef enum {
    // Estructuras principales
    NODE_PROGRAM,
    NODE_FUNCTION_DEF,
    NODE_FUNCTION_CALL,
    NODE_BLOCK,
    NODE_EXPRESSION_LIST,
    
    // Expresiones
    NODE_LET_IN,
    NODE_ASSIGNMENT,
    NODE_VAR_DECLARATION,
    NODE_IF_EXPR,
    NODE_ELIF_EXPR,
    
    // Operaciones
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    
    // Valores
    NODE_NUMBER,
    NODE_STRING,
    NODE_IDENTIFIER,
    NODE_BOOLEAN,
    NODE_PI,
    NODE_E,
    
    // Listas
    NODE_ARG_LIST,
    NODE_PARAM_LIST,
    
    // Terminales
    NODE_EMPTY
} NodeType;

// Operadores disponibles
typedef enum {
    // Aritméticos
    OP_PLUS,       // +
    OP_MINUS,      // -
    OP_MULTIPLY,   // *
    OP_DIVIDE,     // /
    OP_POWER,      // **
    OP_MODULO,     // %
    OP_XOR,        // ^
    OP_CONCAT,     // @
    OP_DCONCAT,    // @@
    
    // Comparación
    OP_EQ,         // ==
    OP_NEQ,        // !=
    OP_LT,         // <
    OP_GT,         // >
    OP_LEQ,        // <=
    OP_GEQ,        // >=
    
    // Lógicos
    OP_AND,        // &
    OP_OR,         // |
    OP_NOT,        // !
    
    // Asignación
    OP_ASSIGN,     // :=
    OP_ARROW,      // =>
    
    OP_NONE
} Operator;

// Estructura base del nodo AST
struct ASTNode {
    NodeType type;
    int line;  // Línea en el código fuente
    
    union {
        // Para valores literales
        double number_value;
        char* string_value;
        bool boolean_value;
        
        // Para operadores
        Operator op;
        
        // Para nodos con hijos
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
        } children;
        
        // Para listas (usando vector para flexibilidad)
        std::vector<struct ASTNode*>* list;
        
        // Para declaraciones de funciones
        struct {
            char* function_name;
            struct ASTNode* params;
            struct ASTNode* body;
        } function_def;
        
        // Para let-in
        struct {
            struct ASTNode* declarations;
            struct ASTNode* body;
        } let_in;
        
        // Para if-elif-else
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* elif_branches;
            struct ASTNode* else_branch;
        } conditional;
        
        // Para llamadas a funciones
        struct {
            char* function_name;
            struct ASTNode* arguments;
        } function_call;
    } data;
};

// Prototipos de funciones para crear nodos

// Programa y bloques
ASTNode* create_program_node(ASTNode* functions, ASTNode* main_expr);
ASTNode* create_block_node(ASTNode* expression_list);
ASTNode* create_expression_list_node(ASTNode* first, ASTNode* rest);

// Definición de funciones
ASTNode* create_function_def_node(const char* name, ASTNode* params, ASTNode* body);
ASTNode* create_param_list_node(ASTNode* first, ASTNode* rest);

// Expresiones let-in
ASTNode* create_let_in_node(ASTNode* declarations, ASTNode* body);
ASTNode* create_declaration_list_node(ASTNode* first, ASTNode* rest);
ASTNode* create_declaration_node(const char* id, ASTNode* expr);

// Expresiones condicionales
ASTNode* create_if_node(ASTNode* condition, ASTNode* then_branch, ASTNode* elif_branches, ASTNode* else_branch);
ASTNode* create_elif_node(ASTNode* condition, ASTNode* then_branch, ASTNode* next_elif);

// Llamadas a funciones
ASTNode* create_function_call_node(const char* name, ASTNode* args);
ASTNode* create_arg_list_node(ASTNode* first, ASTNode* rest);

// Operaciones
ASTNode* create_binary_op_node(Operator op, ASTNode* left, ASTNode* right);
ASTNode* create_unary_op_node(Operator op, ASTNode* operand);

// Valores terminales
ASTNode* create_number_node(double value);
ASTNode* create_string_node(const char* value);
ASTNode* create_identifier_node(const char* name);
ASTNode* create_boolean_node(bool value);
ASTNode* create_pi_node();
ASTNode* create_e_node();
ASTNode* create_empty_node();

// Asignaciones
ASTNode* create_assignment_node(const char* id, ASTNode* expr);

// Funciones de utilidad
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent = 0);
const char* operator_to_string(Operator op);

#endif