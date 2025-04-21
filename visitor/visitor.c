#include "visitor.h"
#include <stdlib.h>

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
            visitor->visit_assignment(visitor, node);
            break;
        case NODE_VARIABLE:
            visitor->visit_variable(visitor, node);
            break;
        case NODE_BUILTIN_FUNC:
            visitor->visit_builtin_func_call(visitor, node);
            break;
        case NODE_BLOCK:
            visitor->visit_block(visitor, node);
            break;
    }
}

void add_error(char*** array, int* count, const char* str) {
    *array = realloc(*array, (*count + 1) * sizeof(char*));
    (*array)[*count] = strdup(str);
    (*count)++;
}

// Liberar memoria
void free_error(char** array, int count) {
    for (int i = 0; i < count; i++) {
        free(array[i]);
    }
    free(array);
}