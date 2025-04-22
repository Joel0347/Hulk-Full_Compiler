#include "semantic.h"


void visit_number(Visitor* v, ASTNode* node) {
    return;
}

void visit_string(Visitor* v, ASTNode* node) {
    return;
}

void visit_boolean(Visitor* v, ASTNode* node) {
    return;
}

void visit_binary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;
    ASTNode* right = node->data.op_node.right;

    left->scope->parent = node->scope;
    right->scope->parent = node->scope;

    accept(v, left);
    accept(v, right);

    Type* left_type = find_type(v, left);
    Type* right_type = find_type(v, right);

    OperatorTypeRule rule = create_op_rule ( 
        left_type, right_type, 
        node->return_type, 
        node->data.op_node.op 
    );

    if (!find_op_match(&rule)) {
        if (left->return_type == &TYPE_ERROR_INST || 
            right->return_type == &TYPE_ERROR_INST) {
                return;
        }
        char* str = NULL;
        asprintf(&str, "Operator '%s' can not be used between '%s' and '%s'. Line: %d.",
            node->data.op_node.op_name, left_type->name, right_type->name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

void visit_unary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;

    left->scope->parent = node->scope;

    accept(v, left);
    Type* left_type = find_type(v, left);

    OperatorTypeRule rule = create_op_rule( 
        left_type, NULL, 
        node->return_type, 
        node->data.op_node.op 
    );

    if (!find_op_match(&rule)) {
        if (left->return_type == &TYPE_ERROR_INST)
            return;

        char* str = NULL;
        asprintf(&str, "Operator '%s' can not be used with '%s'. Line: %d.",
            node->data.op_node.op_name, left_type->name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}

void visit_block(Visitor* v, ASTNode* node) {
    ASTNode* current = NULL;
    for(int i = 0; i < node->data.program_node.count; i++) {
        current =  node->data.program_node.statements[i];
        current->scope->parent = node->scope;
        accept(v, current);
    }

    if (current)
        node->return_type = find_type(v, current);
    else
        node->return_type = &TYPE_VOID_INST;
}