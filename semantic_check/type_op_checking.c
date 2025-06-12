#include "semantic.h"

// method to visit downcasting node
void visit_casting_type(Visitor* v, ASTNode* node) {
    ASTNode* exp = node->data.cast_test.exp;
    char* type_name = node->data.cast_test.type_name;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);
    Type* dynamic_type = get_type(exp);
    Symbol* defined_type = find_defined_type(node->scope, type_name);

    if (!defined_type) {
        ContextItem* item = find_context_item(
            node->context, type_name, 1, 0
        );

        if (item) {
            accept(v, item->declaration);
            defined_type = find_defined_type(node->scope, type_name);
        }

        if (!defined_type) {
            node->data.cast_test.type = &TYPE_ERROR;
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' is not a valid type. Line: %d.",
                type_name, node->line
            );
            return;
        }
    }
    
    // Checking both types are in the same branch of type hierarchy
    if (
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

    node->data.cast_test.type = defined_type->type;
    node->return_type = defined_type->type;
}

// method to visit type-testing node
void visit_test_type(Visitor* v, ASTNode* node) {
    ASTNode* exp = node->data.cast_test.exp;
    char* type_name = node->data.cast_test.type_name;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);

    Symbol* defined_type = find_defined_type(node->scope, type_name);

    if (!defined_type) {
        ContextItem* item = find_context_item(
            node->context, type_name, 1, 0
        );

        if (item) {
            accept(v, item->declaration);
            defined_type = find_defined_type(node->scope, type_name);
        }

        if (!defined_type) {
            node->data.cast_test.type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' is not a valid type. Line: %d.",
                type_name, node->line
            );
            return;
        }
    }

    node->data.cast_test.type = defined_type->type;
}

// method to visit attribute or method getter node
void visit_attr_getter(Visitor* v, ASTNode* node) {
    ASTNode* instance = node->data.op_node.left;
    ASTNode* member = node->data.op_node.right;

    instance->scope->parent = node->scope;
    instance->context->parent = node->context;
    member->context->parent = node->context;
    member->scope->parent = node->scope;

    accept(v, instance);
    Type* instance_type = get_type(instance);

    if (type_equals(instance_type, &TYPE_ERROR)) {
        node->return_type = &TYPE_ERROR;
        return;
    } else if (type_equals(instance_type, &TYPE_ANY)) {
        if (!unify_type_by_attr(v, node)) {
            node->return_type = &TYPE_ERROR;
            return;
        } else {
            accept(v, instance); // visit again if unified
            instance_type = get_type(instance);
        }
    }

    // attributes are private, so they only can be gotten using self
    if (member->type == NODE_VARIABLE && ((
        instance->type == NODE_VARIABLE &&
        strcmp(instance->data.variable_name, "self"))
        || 
        instance->type != NODE_VARIABLE)
    ) {
        report_error(
            v, "Impossible to access to '%s' in type '%s' because all"
            " attributes are private. Line: %d.",
            member->data.variable_name, instance_type->name, node->line
        );
        node->return_type = &TYPE_ERROR;
        return;
    } else if (member->type == NODE_VARIABLE) {
        member->data.variable_name = concat_str_with_underscore(
            instance_type->name, member->data.variable_name
        );

        Symbol* sym = find_type_attr(
            instance_type,
            member->data.variable_name
        );

        if (sym) {
            member->return_type = sym->type;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        } else {
            ContextItem* item = find_item_in_type_hierarchy(
                instance_type->dec->context,
                member->data.variable_name,
                instance_type, 0
            );

            if (!item) {
                member->return_type = &TYPE_ERROR;
                report_error(
                    v, "Type '%s' does not have an attribute named '%s'. Line: %d", 
                    instance_type->name, member->data.variable_name, node->line
                );
            } else {
                accept(v, item->declaration);
                sym = find_type_attr(instance_type, member->data.variable_name);
                member->return_type = sym->type;
                member->is_param = sym->is_param;
                member->derivations = sym->derivations;
            }
        }
    }

    if (member->type == NODE_FUNC_CALL && !type_equals(instance_type, &TYPE_ERROR)) {
        Symbol* t = find_defined_type(node->scope, instance_type->name);

        if (!t) {
            ContextItem* t_item = find_context_item(node->context, instance_type->name, 1, 0);
            if (t_item) {
                accept(v, t_item->declaration);
                instance_type = t_item->return_type;
            } else {
                node->return_type = &TYPE_ERROR;
                if (!type_equals(instance_type, &TYPE_ERROR)) {
                    report_error(
                        v, "Type '%s' does not have a method named '%s'. Line: %d", 
                        instance_type->name, member->data.func_node.name, node->line
                    );
                }
                return;
            }
        } else if (is_builtin_type(instance_type)) {
            node->return_type = &TYPE_ERROR;
            if (!type_equals(instance_type, &TYPE_ERROR)) {
                report_error(
                    v, "Type '%s' does not have a method named '%s'. Line: %d", 
                    instance_type->name, member->data.func_node.name, node->line
                );
            }
            return;
        } else {
            instance_type = t->type;
        }

        member->data.func_node.name = concat_str_with_underscore(
            instance_type->name, member->data.func_node.name
        );
        check_function_call(v, member, instance_type);
    }

    node->return_type = get_type(member);
    node->derivations = add_node_list(member, node->derivations);
}

// method to visit attribute setter node
void visit_attr_setter(Visitor* v, ASTNode* node) {
    ASTNode* instance = node->data.cond_node.cond;
    ASTNode* member = node->data.cond_node.body_true;
    ASTNode* value = node->data.cond_node.body_false;

    value->scope->parent = node->scope;
    value->context->parent = node->context;
    instance->scope->parent = node->scope;
    instance->context->parent = node->context;
    member->context->parent = node->context;
    member->scope->parent = node->scope;

    accept(v, instance);
    Type* instance_type = get_type(instance);

    if (type_equals(instance_type, &TYPE_ERROR)) {
        node->return_type = &TYPE_ERROR;
        return;
    }

    // attributes can only be set using self
    if ((instance->type == NODE_VARIABLE &&
        strcmp(instance->data.variable_name, "self"))
        ||
        instance->type != NODE_VARIABLE
    ) {
        report_error(
            v, "Impossible to access to '%s'  in type '%s' because all"
            " attributes are private. Line: %d.",
            member->data.variable_name, instance_type->name, node->line
        );
        node->return_type = &TYPE_ERROR;
        return;
    }

    member->data.variable_name = concat_str_with_underscore(
        instance_type->name, member->data.variable_name
    );

    Symbol* sym = find_type_attr(
        instance_type,
        member->data.variable_name
    );

    if (sym) {
        member->return_type = sym->type;
        member->is_param = sym->is_param;
        member->derivations = sym->derivations;
    } else {
        ContextItem* item = find_item_in_type_hierarchy(
            instance_type->dec->context,
            member->data.variable_name,
            instance_type, 0
        );

        if (!item) {
            member->return_type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' does not have an attribute named '%s'. Line: %d", 
                instance_type->name, member->data.variable_name, node->line
            );
        } else {
            accept(v, item->declaration);
            sym = find_type_attr(instance_type, member->data.variable_name);
            member->return_type = sym->type;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        }
    }

    accept(v, value);
    Type* inferried_type = get_type(value);

    // checking whether or not the new type is consistent with the initialization type
    if (sym) {
        if (
            is_ancestor_type(sym->type, inferried_type) ||
            type_equals(inferried_type, &TYPE_ANY) ||
            type_equals(sym->type, &TYPE_ANY)
        ) {
            if (type_equals(inferried_type, &TYPE_ANY) &&
                !type_equals(sym->type, &TYPE_ANY)
            ) {
                if (unify(v, value, sym->type)) {
                    accept(v, value);
                    inferried_type = sym->type;
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
                        unify(v, value, inferried_type);
                    }
                }
            }

            sym->derivations = add_node_list(value, sym->derivations);
        } else {
            report_error(
                v, "Variable '%s' was initializated as "
                "'%s', but reassigned as '%s'. Line: %d.",
                sym->name, sym->type->name, inferried_type->name, node->line
            );
        }
    }

    node->return_type = get_type(value);
    node->derivations = add_node_list(value, node->derivations);
    node->derivations = add_node_list(member, node->derivations);
}

// method to visit base function
void visit_base_func(Visitor* v, ASTNode* node) {
    ASTNode* args = node->data.func_node.args;
    char* current_func = v->current_function;
    Type* current_type = v->current_type;

    if (!current_func) {
        node->return_type = &TYPE_ERROR;
        report_error(
            v, "Keyword 'base' only can be used when referring to an ancestor"
            " implementation of a function. Line: %d.", node->line
        );
        return;
    }

    char* f_name = find_base_func_dec(current_type, current_func);

    if (!f_name) {
        if (!is_builtin_type(current_type->parent)) {
            ContextItem* item = find_item_in_type_hierarchy(
                current_type->parent->dec->context, current_func, current_type->parent, 1
            );

            if (item) {
                accept(v, item->declaration);
                f_name = item->declaration->data.func_node.name;
            }
        }

        if (!f_name) {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "No ancestor of type '%s' has a definition for '%s'. Line: %d.",
                current_type->name,
                delete_underscore_from_str(current_func, current_type->name), node->line
            );
            return;
        }
    }

    // helper node that can be checked as a function call
    ASTNode* call = create_func_call_node(f_name, args, node->data.func_node.arg_count);
    call->context->parent = node->context;
    call->scope->parent = node->scope;
    call->line = node->line;

    check_function_call(v, call, current_type->parent);
    node->derivations = add_node_list(call, node->derivations);
    node->return_type = get_type(call);
    node->data.func_node.name = f_name;
}