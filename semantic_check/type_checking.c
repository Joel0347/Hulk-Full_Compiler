#include "semantic.h"


void visit_test_type(Visitor* v, ASTNode* node) {
    ASTNode* exp = node->data.op_node.left;
    char* type_name = node->static_type;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);

    Symbol* defined_type = find_defined_type(node->scope, type_name);

    if (!defined_type) {
        report_error(
            v, "Type '%s' is not a valid type. Line: %d.",
            type_name, node->line
        );
    }
}

void visit_casting_type(Visitor* v, ASTNode* node) {
    ASTNode* exp = node->data.op_node.left;
    char* type_name = node->static_type;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);
    Type* dynamic_type = find_type(v, exp);
    Symbol* defined_type = find_defined_type(node->scope, type_name);

    if (!defined_type) {
        report_error(
            v, "Type '%s' is not a valid type. Line: %d.",
            type_name, node->line
        );
    } else if (
        !type_equals(dynamic_type, &TYPE_ERROR) &&
        !type_equals(dynamic_type, &TYPE_ANY) &&
        !same_branch_in_type_hierarchy(
            dynamic_type, defined_type->type
        )
    ) {
        report_error(
            v, "Type '%s' can not be downcasted to type '%s'. Line: %d.",
            dynamic_type->name, type_name, node->line
        );
    }

    node->return_type = defined_type? defined_type->type : &TYPE_ERROR;
}