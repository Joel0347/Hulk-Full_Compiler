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

    Type* condition_type = find_type(v, condition);

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
    Type* true_type = find_type(v, true_body);
    Type* false_type = NULL;

    if (false_body) {
        accept(v, false_body);
        false_type = find_type(v, false_body);
    } else {
        false_type = find_type(v, true_body);
    }

    if (type_equals(true_type, &TYPE_ANY) &&
        !type_equals(false_type, &TYPE_ANY)
    ) {
        if (unify_member(v, true_body, false_type)) {
            accept(v, true_body);
        }
    } else if (
        type_equals(false_type, &TYPE_ANY) &&
        !type_equals(true_type, &TYPE_ANY)
    ) {
        if (unify_member(v, false_body, true_type)) {
            accept(v, false_body);
        }
    }

    node->return_type = get_common_ancestor(true_type, false_type);
}