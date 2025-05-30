#include "semantic.h"

void visit_conditional(Visitor* v, ASTNode* node) {
    ASTNode* condition = node->data.cond_node.cond;
    ASTNode* true_body = node->data.cond_node.body_true;
    ASTNode* false_body = node->data.cond_node.body_false;

    condition->context->parent = node->context;
    true_body->context->parent = node->context;
    condition->scope->parent = node->scope;
    true_body->scope->parent = node->scope;

    if (false_body) {
        false_body->context->parent = node->context;
        false_body->scope->parent = node->scope;
    }

    accept(v, condition);

    if (unify_member(v, condition, &TYPE_BOOLEAN)) {
        accept(v, condition);
    }

    Type* condition_type = find_type(condition);

    if (!type_equals(condition_type, &TYPE_ERROR) &&
        !type_equals(condition_type, &TYPE_BOOLEAN)
    ) {
        report_error(
            v, "Condition in 'if' expression must return "
            "'Boolean', not '%s'. Line: %d", 
            condition_type->name, condition->line
        );
    }

    accept(v, true_body);
    Type* true_type = find_type(true_body);
    Type* false_type = NULL;

    if (false_body) {
        accept(v, false_body);
        accept(v, true_body);
        false_type = find_type(false_body);
        true_type = find_type(true_body);
    } else {
        false_type = find_type(true_body);
    }

    node->return_type = get_lca(true_type, false_type);
}

void visit_loop(Visitor* v, ASTNode* node) {
    ASTNode* condition = node->data.op_node.left;
    ASTNode* body = node->data.op_node.right;

    condition->context->parent = node->context;
    body->context->parent = node->context;
    condition->scope->parent = node->scope;
    body->scope->parent = node->scope;

    accept(v, condition);

    if (unify_member(v, condition, &TYPE_BOOLEAN)) {
        accept(v, condition);
    }

    Type* condition_type = find_type(condition);

    if (!type_equals(condition_type, &TYPE_ERROR) &&
        !type_equals(condition_type, &TYPE_BOOLEAN)
    ) {
        report_error(
            v, "Condition in 'while' expression must return "
            "'Boolean', not '%s'. Line: %d", 
            condition_type->name, condition->line
        );
    }

    accept(v, body);
    node->return_type = find_type(body);
}