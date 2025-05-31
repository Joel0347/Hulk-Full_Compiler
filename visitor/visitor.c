#include "visitor.h"

void accept(Visitor* visitor, ASTNode* node) {
    if (!node) {
        return;
    }
    
    switch(node->type) {
        case NODE_PROGRAM:
            visitor->visit_program(visitor, node);
            break;
        case NODE_NUMBER:
            visitor->visit_number(visitor, node);
            break;
        case NODE_STRING:
            visitor->visit_string(visitor, node);
            break;
        case NODE_BOOLEAN:
            visitor->visit_boolean(visitor, node);
            break;
        case NODE_UNARY_OP:
            visitor->visit_unary_op(visitor, node);
            break;
        case NODE_BINARY_OP:
            visitor->visit_binary_op(visitor, node);
            break;
        case NODE_ASSIGNMENT:
        case NODE_D_ASSIGNMENT:
            visitor->visit_assignment(visitor, node);
            break;
        case NODE_VARIABLE:
            visitor->visit_variable(visitor, node);
            break;
        case NODE_FUNC_CALL:
            visitor->visit_function_call(visitor, node);
            break;
        case NODE_BLOCK:
            visitor->visit_block(visitor, node);
            break;
        case NODE_FUNC_DEC:
            visitor->visit_function_dec(visitor, node);
            break;
        case NODE_LET_IN:
            visitor->visit_let_in(visitor, node);
            break;
        case NODE_CONDITIONAL:
            visitor->visit_conditional(visitor, node);
            break;
        case NODE_LOOP:
            visitor->visit_loop(visitor, node);
            break;
        case NODE_TYPE_DEC:
            visitor->visit_type_dec(visitor, node);
            break;
        case NODE_TYPE_INST:
            visitor->visit_type_instance(visitor, node);
            break;
        case NODE_TEST_TYPE:
            visitor->visit_test_type(visitor, node);
            break;
        case NODE_CAST_TYPE:
            visitor->visit_casting_type(visitor, node);
            break;
        case NODE_TYPE_GET_ATTR:
            visitor->visit_attr_getter(visitor, node);
            break;
        case NODE_TYPE_SET_ATTR:
            visitor->visit_attr_setter(visitor, node);
            break;
    }
}

void get_context(Visitor* visitor, ASTNode* node) {
    for(int i = 0; i < node->data.program_node.count; i++) {
        ASTNode* child =  node->data.program_node.statements[i];
        if (child->type == NODE_FUNC_DEC || child->type == NODE_TYPE_DEC) {
            child->context->parent = node->context;
            child->scope->parent = node->scope;
            if (!save_context_item(node->context, child)) {
                char* func_or_type = child->type == NODE_FUNC_DEC ? 
                    "Function" : "Type";
                char* name = child->type == NODE_FUNC_DEC ?
                    child->data.func_node.name : child->data.type_node.name;
                report_error(
                    visitor, "%s '%s' already exists. Line: %d.", 
                    func_or_type, name, child->line
                );
            }
        }
    }
}

void add_error(char*** array, int* count, const char* str) {
    *array = realloc(*array, (*count + 1) * sizeof(char*));
    (*array)[*count] = strdup(str);
    (*count)++;
}

void report_error(Visitor* v, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char* str = NULL;
    vasprintf(&str, fmt, args);
    va_end(args);
    add_error(&(v->errors), &(v->error_count), str);
}

// Free memory
void free_error(char** array, int count) {
    for (int i = 0; i < count; i++) {
        free(array[i]);
    }
    free(array);
}