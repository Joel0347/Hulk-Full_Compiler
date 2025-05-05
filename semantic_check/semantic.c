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
        .visit_function_call = visit_function_call,
        .visit_string = visit_string,
        .visit_boolean = visit_boolean,
        .visit_unary_op = visit_unary_op,
        .visit_variable = visit_variable,
        .visit_block = visit_block,
        .visit_function_dec = visit_function_dec,
        .visit_let_in = visit_let_in,
        .error_count = 0,
        .errors = NULL
    };
    
    accept(&visitor, node);
    
    for (int i = 0; i < visitor.error_count; i++) {
        printf(RED "!!SEMANTIC ERROR: %s\n" RESET, visitor.errors[i]);
    }

    free_error(visitor.errors, visitor.error_count);

    return visitor.error_count;
}

Type* find_type(Visitor* v, ASTNode* node) {

    // if (node->return_type) {
    //     return node->return_type;
    // }
    return node->return_type;
}

Type** find_types(ASTNode** args, int args_count) {
    Type** types = (Type**)malloc(args_count * sizeof(Type*));
    for (int i = 0; i < args_count; i++)
    {
        types[i] = args[i]->return_type;
    }
    
    return types;
}

void visit_program(Visitor* v, ASTNode* node) {
    init_builtins(node->scope);
    get_context(v, node);

    for(int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* child =  node->data.program_node.statements[i];
        child->scope->parent = node->scope;
        child->context->parent = node->context;
        accept(v, child);

        if (child->type == NODE_ASSIGNMENT) {
            node->return_type = &TYPE_ERROR_INST;
            report_error(
                v, "Variable '%s' must be initializated in a 'let' definition. Line: %d", 
                child->data.op_node.left->data.variable_name, child->line
            );
        }
    }
}