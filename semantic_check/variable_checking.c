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
    int free_type = 0;

    if (strcmp(var_node->static_type, "") && !defined_type) {
        ContextItem* item = find_context_item(
            node->context, var_node->static_type, 1, 0
        );

        if (item) {
            if (!item->declaration->checked) {
                accept(v, item->declaration);
                defined_type = find_defined_type(node->scope, var_node->static_type);
            } else if (item->return_type) {
                defined_type = (Symbol*)malloc(sizeof(Symbol));
                defined_type->name = item->return_type->name;
                defined_type->type = item->return_type;
                free_type = 1;
            }

            if (!defined_type) {
                report_error(
                    v, "Variable '%s' was defined as '%s', but that type produces a "
                    "cirular reference. Line: %d.", var_node->data.variable_name, 
                    var_node->static_type, node->line
                );
            } 
        } else {
            report_error(
                v, "Variable '%s' was defined as '%s', which is not a valid"
                " type. Line: %d.", var_node->data.variable_name, 
                var_node->static_type, node->line
            );
        }
    } else if (strcmp(var_node->static_type, "") && type_equals(defined_type->type, &TYPE_VOID)) {
        report_error(
            v, "Variable '%s' was defined as 'Void', which is not a valid type. Line: %d.", 
            var_node->data.variable_name, node->line
        );
    }

    accept(v, val_node);
    Type* inferried_type = find_type(val_node);

    if (type_equals(inferried_type, &TYPE_ANY) && 
        defined_type && unify_member(v, val_node, defined_type->type)
    ) {
        accept(v, val_node);
        inferried_type = find_type(val_node);
    }

    if (defined_type && !is_ancestor_type(defined_type->type, inferried_type)) {
        report_error(
            v, "Variable '%s' was defined as '%s', but inferred as '%s'. Line: %d.", 
            var_node->data.variable_name, var_node->static_type, 
            inferried_type->name, node->line
        );
    } else if (type_equals(inferried_type, &TYPE_VOID) && !defined_type) {
        report_error(
            v, "Variable '%s' was inferred as 'Void', which is not a valid target. Line: %d.", 
            var_node->data.variable_name, node->line
        );
    }

    if (strcmp(var_node->static_type, "") && node->type == NODE_D_ASSIGNMENT) {
        report_error(
            v, "Variable '%s' can not be type annotated when it "
            "is reassigned. Line: %d.", var_node->data.variable_name, node->line
        );
    }
    
    if (defined_type)
        inferried_type = defined_type->type;

    Symbol* sym = find_symbol(node->scope, var_node->data.variable_name);
    
    if (!sym && node->type == NODE_D_ASSIGNMENT) {
        report_error(
            v, "Variable '%s' needs to be initializated in a "
            "'let' definition before being reassigned. Line: %d.",
            var_node->data.variable_name, node->line
        );
    } else if (node->type == NODE_ASSIGNMENT) {
        declare_symbol(
            node->scope->parent, 
            var_node->data.variable_name, inferried_type,
            0, val_node
        );
        // Symbol* s = find_symbol(node->scope->parent, var_node->data.variable_name);
        // s->derivations = add_value_list(val_node, s->derivations);
        // val_node->return_type = inferried_type;
    } else if (
        is_ancestor_type(sym->type, inferried_type) ||
        type_equals(inferried_type, &TYPE_ANY) ||
        type_equals(sym->type, &TYPE_ANY)
    ) {
        if (type_equals(inferried_type, &TYPE_ANY) &&
            !type_equals(sym->type, &TYPE_ANY)
        ) {
            if (unify_member(v, val_node, sym->type)) {
                accept(v, val_node);
                inferried_type = find_type(val_node);
            }
        } else if (
            type_equals(sym->type, &TYPE_ANY) &&
            !type_equals(inferried_type, &TYPE_ANY)
        ) {
            sym->type = inferried_type;
            for (int i = 0; i < sym->derivations->count; i++)
            {
                ASTNode* value = at(i, sym->derivations);
                if (value && type_equals(value->return_type, &TYPE_ANY)) {
                    unify_member(v, value, inferried_type);
                }
            }
            val_node->return_type = inferried_type;
        }

        sym->derivations = add_value_list(val_node, sym->derivations);
        node->derivations = add_value_list(var_node, node->derivations);
    } else {
        report_error(
            v, "Variable '%s' was initializated as "
            "'%s', but reassigned as '%s'. Line: %d.",
            sym->name, sym->type->name, inferried_type->name, node->line
        );
    }

    var_node->return_type = inferried_type;

    if (node->type == NODE_D_ASSIGNMENT) {
        node->return_type = inferried_type;
    }

    if (free_type) {
        free(defined_type);
    }
}

void visit_variable(Visitor* v, ASTNode* node) {
    Symbol* sym = find_symbol(node->scope, node->data.variable_name);

    if (sym) {
        if (!sym->is_type_param) {
            node->return_type = sym->type;
            node->is_param = sym->is_param;
            node->derivations = sym->derivations;
        } else {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "The use of constructor parameter '%s' is not allowed. Line: %d", 
                node->data.variable_name, node->line
            );
        }
    } else if (!type_equals(node->return_type, &TYPE_ERROR)) {
        ContextItem* item = find_context_item(
            node->context, node->data.variable_name, 0, 1
        );
        if (!item) {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "Undefined variable '%s'. Line: %d", 
                node->data.variable_name, node->line
            );
        } else {
            accept(v, item->declaration);
            item->declaration->checked = 0;
            sym = find_symbol(node->scope, node->data.variable_name);
            node->return_type = sym->type;
            node->is_param = sym->is_param;
            node->derivations = sym->derivations;
        }
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
    node->return_type = find_type(body);
}