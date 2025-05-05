#include "semantic.h"


void visit_assignment(Visitor* v, ASTNode* node) {
    ASTNode* var_node = node->data.op_node.left;
    ASTNode* val_node = node->data.op_node.right;

    if (match_as_keyword(var_node->data.variable_name)) {
        char* str = NULL;
        asprintf(&str, "Keyword '%s' can not be used as a variable name. Line: %d.", 
            var_node->data.variable_name, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }
    
    var_node->scope->parent = node->scope;
    var_node->context->parent = node->context;
    val_node->scope->parent = node->scope;
    val_node->context->parent = node->context;

    Symbol* defined_type = find_defined_type(node->scope, var_node->static_type);

    if (strcmp(var_node->static_type, "") && !defined_type) {
        char* str = NULL;
        asprintf(&str, "Variable '%s' was defined as '%s', which is not a valid type. Line: %d.", 
            var_node->data.variable_name, var_node->static_type, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }

    accept(v, val_node);
    Type* inferried_type = find_type(v, val_node);

    if (defined_type && !is_ancestor_type(defined_type->type, inferried_type)) {
        char* str = NULL;
        asprintf(&str, "Variable '%s' was defined as '%s', but inferred as '%s'. Line: %d.", 
            var_node->data.variable_name, var_node->static_type, 
            inferried_type->name, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
    }
    
    if (defined_type)
        inferried_type = defined_type->type;

    Symbol* sym = find_symbol(node->scope, var_node->data.variable_name);
    
    if(!sym) {
        declare_symbol(
            node->scope->parent, 
            var_node->data.variable_name, inferried_type,
            0, val_node
        );
    } else if (sym->is_param) {
        char* str = NULL;
        asprintf(&str, "Parameter '%s' can not be redefined. Line: %d.", 
            var_node->data.variable_name, node->line
        );
        add_error(&(v->errors), &(v->error_count), str);
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
        char* str = NULL;
        asprintf(&str, "Undefined variable '%s'. Line: %d", node->data.variable_name, node->line);
        add_error(&(v->errors), &(v->error_count), str);
    }
}