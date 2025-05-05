#include "semantic.h"


void visit_assignment(Visitor* v, ASTNode* node) {
    ASTNode* var_node = node->data.op_node.left;
    ASTNode* val_node = node->data.op_node.right;

    if (match_as_keyword(var_node->data.variable_name)) {
        report_error(
            v, "Keyword '%s' can not be used as a variable name. Line: %d.", 
            var_node->data.variable_name, node->line
        );
    }
    
    var_node->scope->parent = node->scope;
    var_node->context->parent = node->context;
    val_node->scope->parent = node->scope;
    val_node->context->parent = node->context;

    Symbol* defined_type = find_defined_type(node->scope, var_node->static_type);

    if (strcmp(var_node->static_type, "") && !defined_type) {
        report_error(
            v, "Variable '%s' was defined as '%s', which is not a valid"
            " type. Line: %d.", var_node->data.variable_name, 
            var_node->static_type, node->line
        );
    }

    accept(v, val_node);
    Type* inferried_type = find_type(v, val_node);

    if (defined_type && !is_ancestor_type(defined_type->type, inferried_type)) {
        report_error(
            v, "Variable '%s' was defined as '%s', but inferred as '%s'. Line: %d.", 
            var_node->data.variable_name, var_node->static_type, 
            inferried_type->name, node->line
        );
    }
    
    if (defined_type)
        inferried_type = defined_type->type;

    if (node->type == NODE_D_ASSIGNMENT) {
        node->return_type = inferried_type;
    }

    Symbol* sym = find_symbol(node->scope, var_node->data.variable_name);
    
    if (!sym && node->type == NODE_D_ASSIGNMENT) {
        report_error(
            v, "Variable '%s' needs to be initializated in a "
            "'let' definition before using operator ':='. Line: %d.",
            var_node->data.variable_name, node->line
        );
    } else if (sym && sym->is_param) {
        report_error(
            v, "Parameter '%s' can not be redefined. Line: %d.", 
            var_node->data.variable_name, node->line
        );
    } else if (node->type == NODE_ASSIGNMENT) {
        declare_symbol(
            node->scope->parent, 
            var_node->data.variable_name, inferried_type,
            0, val_node
        );
    } else {
        sym->type = inferried_type;
    }

    var_node->return_type = inferried_type;
}

void visit_variable(Visitor* v, ASTNode* node) {
    Symbol* sym = find_symbol(node->scope, node->data.variable_name);

    if (sym) {
        node->return_type = sym->type;
        node->is_param = sym->is_param;
        node->value = sym->value;
    } else if (!type_equals(node->return_type, &TYPE_ERROR_INST)) {
        node->return_type = &TYPE_ERROR_INST;
        report_error(
            v, "Undefined variable '%s'. Line: %d", 
            node->data.variable_name, node->line
        );
    }
}

void visit_let_in(Visitor* v, ASTNode* node) {
    ASTNode** declarations = node->data.func_node.args;
    ASTNode* body = node->data.func_node.body;

    body->scope->parent = node->scope;
    body->context->parent = node->context;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        declarations[i]->scope->parent = node->scope;
        declarations[i]->context->parent = node->context;
        accept(v, declarations[i]);
    }

    accept(v, body);
    node->return_type = find_type(v, body);
}