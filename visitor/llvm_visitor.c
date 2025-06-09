#include "llvm_visitor.h"
#include <stdlib.h>
#include <stdio.h>

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
        case NODE_D_ASSIGNMENT:
            return visitor->visit_assignment(visitor, node);
        case NODE_VARIABLE:
            return visitor->visit_variable(visitor, node);
        case NODE_FUNC_CALL:
            // Handle both regular function calls and method calls
            return visitor->visit_function_call(visitor, node);
        case NODE_BLOCK:
            return visitor->visit_block(visitor, node);
        case NODE_FUNC_DEC:
            return visitor->visit_function_dec(visitor, node);
        case NODE_LET_IN:
            return visitor->visit_let_in(visitor, node);
        case NODE_CONDITIONAL:
            return visitor->visit_conditional(visitor, node);
        case NODE_LOOP:
            return visitor->visit_loop(visitor, node);
        case NODE_TYPE_DEC:
            return visitor->visit_type_dec(visitor, node);
        case NODE_TYPE_INST:
            return visitor->visit_type_inst(visitor, node);
        case NODE_TYPE_GET_ATTR:
            if (node->data.op_node.right->type == NODE_FUNC_CALL) {
                return visitor->visit_type_method(visitor, node);
            }
            return visitor->visit_type_get_attr(visitor, node);
        case NODE_TEST_TYPE:  // Handle 'is' operator
            return visitor->visit_type_test(visitor, node);
        case NODE_CAST_TYPE:  // Handle 'as' operator 
            return visitor->visit_type_cast(visitor, node);
        default:
            fprintf(stderr, "Unknown node type in code generation\n");
            exit(1);
    }
}