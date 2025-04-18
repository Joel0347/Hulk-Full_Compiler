#define _GNU_SOURCE
#include "../type/type.h"
#include "../scope/scope.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>

#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"

int analyze_semantics(ASTNode* node) {
    Visitor visitor = {
        .visit_program = visit_program,
        .visit_assignment = visit_assignment,
        .visit_binary_op = visit_binary_op,
        .visit_number = visit_number,
        .visit_print = visit_print,
        .visit_string = visit_string,
        .visit_boolean = visit_boolean,
        .visit_unary_op = visit_unary_op,
        .visit_variable = visit_variable,
        .error_count = 0,
        .errors = NULL
    };
    
    accept(&visitor, node);
    
    // Mostrar errores
    for (int i = 0; i < visitor.error_count; i++) {
        printf(RED "!!SEMANTIC ERROR: %s\n" RESET, visitor.errors[i]);
    }

    // Liberar recursos
    free_error(visitor.errors, visitor.error_count);

    return visitor.error_count;
}

Type* find_type(Visitor* v, ASTNode* node) {

    // if (node->return_type) {
    //     return node->return_type;
    // }
    return node->return_type;
}

int compatibility_type_binary_op(Operator op, Type* left, Type* right) {

    for (int i = 0; i < rules_count; i++)
    {
        OperatorTypeRule rule = operator_rules[i];
        if (rule.op == op && 
            rule.left_type->kind == left->kind &&
            rule.right_type->kind == right->kind)  
            return 1;
    }

    return 0;
}

int compatibility_type_unary_op(Operator op, Type* left) {

    for (int i = 0; i < rules_count; i++)
    {
        OperatorTypeRule rule = operator_rules[i];
        if (rule.op == op && 
            rule.left_type->kind == left->kind)  
            return 1;
    }

    return 0;
}

// Funciones de visita para cada nodo:
static void visit_program(Visitor* v, ASTNode* node) { 
    for(int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* child =  node->data.program_node.statements[i];
        child->scope->parent = node->scope;
        accept(v, child);
    }
}

static void visit_assignment(Visitor* v, ASTNode* node) {
    ASTNode* var_node = node->data.op_node.left;
    ASTNode* val_node = node->data.op_node.right;

    if(var_node->type != NODE_VARIABLE) {
        char* str = NULL;
        asprintf(&str, "Left side of assigment must be a variable in line: %d.", node->line);
        add_error(&(v->errors), &(v->error_count), str);
        return;
    }
    
    // Chequear expresión
    var_node->scope->parent = node->scope;
    val_node->scope->parent = node->scope;

    accept(v, val_node);
    
    // Insertar en tabla de símbolos
    Symbol* sym = find_symbol(node->scope, var_node->data.variable_name);
    Type* inferried_type = find_type(v, val_node);
    
    if(!sym) {
        declare_symbol(
            node->scope->parent, 
            var_node->data.variable_name, inferried_type
        );
    } else {
        sym->type = inferried_type;
    }

    var_node->return_type = inferried_type;
}

static void visit_variable(Visitor* v, ASTNode* node) {
    Symbol* sym = find_symbol(node->scope, node->data.variable_name);

    if(sym) {
        node->return_type = sym->type;
    } else {
        char* str = NULL;
        asprintf(&str, "Undefined variable '%s' in line: %d", node->data.variable_name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

static void visit_number(Visitor* v, ASTNode* node) {
    return;
}

static void visit_string(Visitor* v, ASTNode* node) {
    return;
}

static void visit_boolean(Visitor* v, ASTNode* node) {
    return;
}

static void visit_binary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;
    ASTNode* right = node->data.op_node.right;

    left->scope->parent = node->scope;
    right->scope->parent = node->scope;

    accept(v, left);
    accept(v, right);

    Type* left_type = find_type(v, left);
    Type* right_type = find_type(v, right);

    if (!compatibility_type_binary_op(node->data.op_node.op, left_type, right_type)) {
        char* str = NULL;
        asprintf(&str, "Operator '%s' can not be used between '%s' and '%s' in line: %d.",
            node->data.op_node.op_name, left_type->name, right_type->name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

static void visit_unary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;

    left->scope->parent = node->scope;

    accept(v, left);
    Type* left_type = find_type(v, left);

    if (!compatibility_type_unary_op(node->data.op_node.op, left_type)) {
        char* str = NULL;
        asprintf(&str, "Operator '%s' can not be used with '%s' in line: %d.",
            node->data.op_node.op_name, left_type->name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

static void visit_print(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;
    left->scope->parent = node->scope;
    accept(v, left);
}