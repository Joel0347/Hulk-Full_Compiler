#define _GNU_SOURCE
#include "../type/type.h"
#include "../scope/scope.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>

#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"

// main method in semantic check
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
        .visit_conditional = visit_conditional,
        .visit_q_conditional = visit_q_conditional,
        .visit_loop = visit_loop,
        .visit_for_loop = visit_for_loop,
        .visit_casting_type = visit_casting_type,
        .visit_test_type = visit_test_type,
        .visit_type_dec = visit_type_dec,
        .visit_type_instance = visit_type_instance,
        .visit_attr_getter = visit_attr_getter,
        .visit_attr_setter = visit_attr_setter,
        .visit_base_func = visit_base_func,
        .error_count = 0,
        .errors = NULL,
        .current_function = NULL,
        .current_type = NULL,
        .type_id = 0
    };
    // starts visiting program node
    accept(&visitor, node);
    // removing duplicates (as a result of re-visiting some nodes)
    StrList* errors = to_set(visitor.errors, visitor.error_count);
    
    while (errors)
    {
        printf(RED "!!SEMANTIC ERROR: %s\n" RESET, errors->value);
        errors = errors->next;
    }

    free_error(visitor.errors, visitor.error_count);
    free_str_list(errors);

    return visitor.error_count;
}

// method to visit program node
void visit_program(Visitor* v, ASTNode* node) {
    init_builtins(node->scope);
    get_context(v, node);

    for(int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* child =  node->data.program_node.statements[i];
        child->scope->parent = node->scope;
        child->context->parent = node->context;
        accept(v, child);

        if (child->type == NODE_ASSIGNMENT) {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "Variable '%s' must be initializated in a 'let' definition. Line: %d.", 
                child->data.op_node.left->data.variable_name, child->line
            );
        }
    }
}

// <----------KEYWORDS---------->

// keywords
char* keywords[] = { 
    "Number", "String", "Boolean", "Object", "Void",
    "true", "false", "PI", "E", "function", "let", "in",
    "is", "as", "type", "inherits", "new", "base"
};
int keyword_count = sizeof(keywords) / sizeof(char*);

// method to check whether or not a name is a keyword
int match_as_keyword(char* name) {
    for (int i = 0; i < keyword_count; i++)
    {
        if (!strcmp(keywords[i], name))
            return 1;
    }

    return 0;
}

//<----------SCAPES---------->

char scape_chars[] = { 'n', 't', '\\', '\"' };
int scapes_count = sizeof(scape_chars) / sizeof(char);

// method to check whether or not a char is a scape sequence
int is_scape_char(char c) {
    for (int i = 0; i < scapes_count; i++)
    {
        if (scape_chars[i] == c)
            return 1;
    }

    return 0;
}