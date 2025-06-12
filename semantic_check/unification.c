#include "semantic.h"

// main method of the unification engine
int unify(Visitor* v, ASTNode* node, Type* type) {
    int unified = 0;
    
    // If it is parameter, tries to unify it directly
    if (node->type == NODE_VARIABLE && node->is_param == 1) {
        Symbol* sym = find_parameter(node->scope, node->data.variable_name);
        
        if (!sym)
            return 0;

        if (type_equals(sym->type, &TYPE_ANY) ||
            is_ancestor_type(sym->type, type)
        ) {
            sym->type = type;
            node->return_type = type;

            for (int i = 0; i < sym->derivations->count; i++)
            {
                ASTNode* value = at(i, sym->derivations);
                if (value && type_equals(value->return_type, &TYPE_ANY)) {
                    unify(v, value, type);
                }
            }
        } else if (!is_ancestor_type(type, sym->type)) {
            report_error(
                v, "Parameter '%s' behaves both as '%s' and '%s'. Line: %d.",
                node->data.variable_name, sym->type->name, type->name, node->line
            );
            return 0;
        }

        unified = 1;
    } else if (node->type == NODE_FUNC_CALL && // If it is a function call unify it directly
        type_equals(node->return_type, &TYPE_ANY)
    ) {
        ContextItem* item = find_context_item(node->context, node->data.func_node.name, 0, 0);
        if (item) {
            item->return_type = type;
            node->return_type = type;
            unified = 1;
        }
    } else if (node->type == NODE_CONDITIONAL || node->type == NODE_Q_CONDITIONAL) {
        unified = unify_conditional(v, node, type);
    } else if (node->derivations) {
        // trying to unify every derivation
        for (int i = 0; i < node->derivations->count; i++)
        {
            ASTNode* value = at(i, node->derivations);
            if (value && type_equals(value->return_type, &TYPE_ANY)) {
                int u_member = unify(v, value, type);

                if (u_member) {
                    value->return_type = type;
                }

                unified |= u_member;
            }
        }

        if (unified) {
            if (node->type == NODE_VARIABLE) {
                Symbol* s = find_symbol(node->scope, node->data.variable_name);

                if (s) {
                    s->type = type;
                }
            }
            node->return_type = type;
        }
    }

    return unified;
}

// method to unify binary and unary operation nodes
int unify_op(Visitor* v, ASTNode* left, ASTNode* right, Operator op, char* op_name) {
    int count = 0;
    int unified = 0; // 0: not unfied, 1: unified right, 2: unified left, 3: unified both
    OperatorTypeRule* rule;

    if (right && type_equals(left->return_type, &TYPE_ANY) && 
        type_equals(right->return_type, &TYPE_ANY)) {
        for (int i = 0; i < op_rules_count; i++)
        {
            if (operator_rules[i].op == op) {
                count++;
                if (count > 1) {
                    return 0; // more than one pair of types is possible
                }
                rule = &operator_rules[i];
            }
        }

        if (count) {
            int u_left = unify(v, left, rule->left_type);
            int u_right = unify(v, right, rule->right_type);

            unified = (u_left && u_right) ? 3 : 0;
        }
    } else if (type_equals(left->return_type, &TYPE_ANY)) {
        for (int i = 0; i < op_rules_count; i++)
        {
            if (operator_rules[i].op == op && 
                (!right || type_equals(&TYPE_ERROR, right->return_type) ||
                type_equals(right->return_type, operator_rules[i].right_type))
            ) {
                count++;
                if (count > 1) {
                    return 0; // more than one type is possible
                }
                rule = &operator_rules[i];
            }
        }

        if (count) {
            unified = unify(v, left, rule->left_type) ? 2 : 0;
        } else if (!type_equals(right->return_type, &TYPE_ERROR)) {
            report_error(
                v, "Operator '%s' can not be used with '%s' as right side. Line: %d.",
                op_name, right->return_type->name, right->line
            );
        }
    } else if (right && type_equals(right->return_type, &TYPE_ANY)) {
        for (int i = 0; i < op_rules_count; i++)
        {
            if (operator_rules[i].op == op && 
                (type_equals(&TYPE_ERROR, left->return_type) ||
                type_equals(left->return_type, operator_rules[i].left_type))
            ) {
                count++;
                if (count > 1) {
                    return 0; // more than one type is possible
                }
                rule = &operator_rules[i];
            }
        }

        if (count) {
            unified = unify(v, right, rule->right_type) ? 1 : 0;
        } else if (!type_equals(left->return_type, &TYPE_ERROR)) {
            report_error(
                v, "Operator '%s' can not be used with '%s' as left side. Line: %d.",
                op_name, left->return_type->name, left->line
            );
        }
    }

    return unified;
}

// method to unify function arguments
IntList* unify_func(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* f_name, ContextItem* item) {
    int count = 0;
    IntList* unified = NULL;
    Function* f;

    // trying to find the function
    while (scope)
    {
            Function* current = scope->functions->first;
            int i = 0;
            while (i < scope->functions->count)
            {
                if (!strcmp(f_name, current->name) &&
                    arg_count == current->arg_count
                ) {
                    count ++;
                    if (count > 1) {
                        return NULL;
                    }

                    f = current;
                }

                current = current->next;
                i++;
            }

        scope = scope->parent;
    }
    
    if (count) {
        for (int i = 0; i < arg_count; i++)
        {
            if (type_equals(&TYPE_ANY, args[i]->return_type)) {
                if (unify(v, args[i], f->args_types[i])) {
                    unified = add_int_list(unified, i);
                } else {
                    return NULL;
                }
            }
        }
    } else if (item && item->declaration->data.func_node.arg_count == arg_count) {
        // trying with the declaration if it is not available in the scope
        for (int i = 0; i < item->declaration->data.func_node.arg_count; i++)
        {
            if (unify(
                v, item->declaration->data.func_node.args[i],
                args[i]->return_type
            )) {
                unified = add_int_list(unified, i);
            } else {
                return NULL;
            }
        }
    }
    
    return unified;
}

// method to unify type arguments
IntList* unify_type(Visitor* v, ASTNode** args, Scope* scope, int arg_count, char* t_name, ContextItem* item) {
    int count = 0;
    IntList* unified = NULL;
    Type* t;

    // trying to find the type
    while (scope)
    {
        if (scope->defined_types) {
            Symbol* current = scope->defined_types;
            int i = 0;
            while (i < scope->t_count)
            {
                if (!strcmp(t_name, current->name) &&
                    arg_count == current->type->arg_count
                ) {
                    count ++;
                    if (count > 1) {
                        return NULL;
                    }

                    t = current->type;
                }

                current = current->next;
                i++;
            }
        }

        scope = scope->parent;
    }

    if (count) {
        for (int i = 0; i < arg_count; i++)
        {
            if (type_equals(&TYPE_ANY, args[i]->return_type)) {
                if (unify(v, args[i], t->param_types[i])) {
                    unified = add_int_list(unified, i);
                } else {
                    return NULL;
                }
            }
        }
    } else if (item && item->declaration->data.type_node.arg_count == arg_count) {
        // trying with the declaration if it is not available in the scope
        for (int i = 0; i < item->declaration->data.type_node.arg_count; i++)
        {
            if (unify(
                v, item->declaration->data.type_node.args[i],
                args[i]->return_type
            )) {
                unified = add_int_list(unified, i);
            } else {
                return NULL;
            }
        }
    }
    
    return unified;
}

// method to unify bodys of conditionals
int unify_conditional(Visitor* v, ASTNode* node, Type* type) {
    if (type_equals(type, &TYPE_ANY)) {
        node->return_type = &TYPE_OBJECT;
        return 0;
    }

    return (
        unify(v, node->data.cond_node.body_true, type) ||
        (node->data.cond_node.body_false &&
            unify(v, node->data.cond_node.body_false, type)
        )
    );
}

// method to unify a type knowing a method name
int unify_type_by_attr(Visitor* v, ASTNode* node) {
    ASTNode* instance = node->data.op_node.left;
    ASTNode* member = node->data.op_node.right;
    int unified = 0;

    if (member->type == NODE_VARIABLE)
        return 1;
    
    NodeList* types = find_types_by_method(node->context, member->data.func_node.name);
    Type* type = NULL;
    Symbol* defined_type = NULL;

    for (int i = 0; i < types->count; i++) {
        ASTNode* dec = at(i, types);
        accept(v, dec);
        defined_type = find_defined_type(node->scope, dec->data.type_node.name);

        if (defined_type) {
            if (!type)
                type = defined_type->type;
            else { // trying to get the lowest common ancestor that has that method
                Type* tmp = get_lca(type, defined_type->type);
                
                if (type_contains_method_in_scope(tmp, member->data.func_node.name, 1))
                    type = tmp;
                else
                    return 0;
            }
        } 
    }
    

    if (type) {
        unified = unify(v, instance, type);

        if (unified) {
            instance->return_type = type;
        }
    }

    return unified;
}