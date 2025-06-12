#include "semantic.h"

// method to visit number literal node
void visit_number(Visitor* v, ASTNode* node) { }

// method to visit string literal node
void visit_string(Visitor* v, ASTNode* node) {
    char* string = node->data.string_value;
    int count = strlen(string);

    for (int i = 0; i < count; i++) // check all scapes sequences
    {
        if (string[i] == '\\') {
            if (!is_scape_char(string[i + 1])) {
                report_error(
                    v, "Invalid scape sequence '\\%c'. Line: %d.", 
                    string[i + 1], node->line
                );
            }
            i++;
        }
    }
}

// method to visit boolean literal node 
void visit_boolean(Visitor* v, ASTNode* node) { }

// method to visit binary operation node
void visit_binary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;
    ASTNode* right = node->data.op_node.right;

    left->scope->parent = node->scope;
    left->context->parent = node->context;
    right->scope->parent = node->scope;
    right->context->parent = node->context;

    accept(v, left);
    accept(v, right);

    int unified = unify_op(
        v, left, right, node->data.op_node.op, 
        node->data.op_node.op_name
    );

    if (unified == 3) { // visit again if unified
        accept(v, left);
        accept(v, right);
    } else if (unified == 2) {
        accept(v, left);
    } else if (unified == 1) {
        accept(v, right);
    }

    Type* left_type = get_type(left);
    Type* right_type = get_type(right);

    OperatorTypeRule rule = create_op_rule( 
        left_type, right_type, 
        node->return_type, 
        node->data.op_node.op 
    );

    if (!find_op_match(&rule)) { // check type matching in operation
        report_error(
            v, "Operator '%s' can not be used between '%s' and '%s'. Line: %d.",
            node->data.op_node.op_name, left_type->name, right_type->name, node->line
        );
    }
}

// method to visit unary operation node
void visit_unary_op(Visitor* v, ASTNode* node) {
    ASTNode* left = node->data.op_node.left;

    left->scope->parent = node->scope;
    left->context->parent = node->context;

    accept(v, left);

    if (unify_op(v, left, NULL, node->data.op_node.op, node->data.op_node.op_name)) {
        accept(v, left); // visit again if unified
    }

    Type* left_type = get_type(left);

    OperatorTypeRule rule = create_op_rule( 
        left_type, NULL, 
        node->return_type, 
        node->data.op_node.op 
    );

    if (!find_op_match(&rule)) { // check type matching in operation
        report_error(
            v, "Operator '%s' can not be used with '%s'. Line: %d.",
            node->data.op_node.op_name, left_type->name, node->line
        );
    }
}

// method to visit block
void visit_block(Visitor* v, ASTNode* node) {
    if (!node->checked)
        get_context(v, node);

    node->checked = 1;
    ASTNode* current = NULL;
    
    for (int i = 0; i < node->data.program_node.count; i++) {
        current =  node->data.program_node.statements[i];
        current->scope->parent = node->scope;
        current->context->parent = node->context;
        accept(v, current); // check all expressions inside

        if (current->type == NODE_ASSIGNMENT) {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "Variable '%s' must be initializated in a 'let' definition. Line: %d.", 
                current->data.op_node.left->data.variable_name, current->line
            );
        }
    }

    if (current) {
        node->return_type = get_type(current); // keep the last expression type
        node->derivations = add_node_list(current, node->derivations);
    } else {
        node->return_type = &TYPE_VOID;
    }
}