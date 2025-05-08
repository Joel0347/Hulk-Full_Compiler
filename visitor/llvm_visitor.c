#include "llvm_visitor.h"
#include <stdlib.h>

LLVMValueRef accept_gen(LLVM_Visitor* visitor, ASTNode* node) {
    if (!node) {
        return 0;
    }
    
    switch(node->type) {
        case NODE_PROGRAM:
            return visitor->visit_program(visitor, node);
        case NODE_NUMBER:
            return visitor->visit_number(visitor, node);
        case NODE_STRING:
            return visitor->visit_string(visitor, node);
        case NODE_BOOLEAN:
            return visitor->visit_boolean(visitor, node);
        case NODE_UNARY_OP:
            return visitor->visit_unary_op(visitor, node);
        case NODE_BINARY_OP:
            return visitor->visit_binary_op(visitor, node);
        case NODE_ASSIGNMENT:
            return visitor->visit_assignment(visitor, node);
        case NODE_VARIABLE:
            return visitor->visit_variable(visitor, node);
        case NODE_FUNC_CALL:
            return visitor->visit_function_call(visitor, node);
        case NODE_BLOCK:
            return visitor->visit_block(visitor, node);
        case NODE_FUNC_DEC:
            return visitor->visit_function_dec(visitor, node);
        default:
            exit(1);
    }
}