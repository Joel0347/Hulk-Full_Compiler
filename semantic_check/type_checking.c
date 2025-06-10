#include "semantic.h"

MRO* mro_list;

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
    Type* instance_type = find_type(instance);

    if (type_equals(instance_type, &TYPE_ERROR)) {
        node->return_type = &TYPE_ERROR;
        return;
    }

    // if (instance_type->dec) {
    //     member->context->parent = instance_type->dec->context;
    //     member->scope->parent = instance_type->dec->scope; 
    // }

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

    Symbol* sym = get_type_attr(
        instance_type,
        member->data.variable_name
    );

    if (sym) {
        member->return_type = sym->type;
        member->is_param = sym->is_param;
        member->derivations = sym->derivations;
    } else {
        ContextItem* item = find_item_in_type(
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
            sym = get_type_attr(instance_type, member->data.variable_name);
            member->return_type = sym->type;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        }
    }

    accept(v, value);
    Type* inferried_type = find_type(value);

    if (sym) {
        if (
            is_ancestor_type(sym->type, inferried_type) ||
            type_equals(inferried_type, &TYPE_ANY) ||
            type_equals(sym->type, &TYPE_ANY)
        ) {
            if (type_equals(inferried_type, &TYPE_ANY) &&
                !type_equals(sym->type, &TYPE_ANY)
            ) {
                if (unify_member(v, value, sym->type)) {
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
                        unify_member(v, value, inferried_type);
                    }
                }
            }

            sym->derivations = add_value_list(value, sym->derivations);
        } else {
            report_error(
                v, "Variable '%s' was initializated as "
                "'%s', but reassigned as '%s'. Line: %d.",
                sym->name, sym->type->name, inferried_type->name, node->line
            );
        }
    }

    node->return_type = find_type(value);
    node->derivations = add_value_list(value, node->derivations);
    node->derivations = add_value_list(member, node->derivations);
}

void check_params(Visitor* v, ASTNode* node) {
    
}

void visit_type_dec(Visitor* v, ASTNode* node) {
    if (find_type_in_mro(node->data.type_node.parent_name, mro_list)) {
        report_error(
            v, "Circular inheritance detected. Line: %d.", node->line
        );
        mro_list = empty_mro_list(mro_list);
        return;
    }

    if (node->checked) {
        return;
    }

    node->checked = 1;
    Type* visitor_type = v->current_type;
    Scope* parent_scope = node->scope->parent;

    mro_list = add_type_to_mro(node->data.type_node.parent_name, mro_list);

    ASTNode** params = node->data.type_node.args;
    ASTNode** definitions = node->data.type_node.definitions;

    for (int i = 0; i < node->data.type_node.arg_count - 1; i++) {
        for (int j = i + 1; j < node->data.type_node.arg_count; j++) {
            if (!strcmp(
                params[i]->data.variable_name,
                params[j]->data.variable_name
            )) {
                report_error(
                    v, "Symbol '%s' is used for both argument '%d' and argument '%d' in "
                    "type '%s' declaration. Line: %d.", params[i]->data.variable_name,
                    i + 1, j + 1, node->data.type_node.name, node->line
                );
                return;
            }
        }
    }

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        params[i]->scope->parent = node->scope;
        params[i]->context->parent = node->context;
        Symbol* param_type = find_defined_type(node->scope, params[i]->static_type);
        int free_type = 0;

        if (strcmp(params[i]->static_type, "") && !param_type) {
             ContextItem* item = find_context_item(
                node->context, params[i]->static_type, 1, 0
            );
    
            if (item) {
                if (!item->declaration->checked) {
                    accept(v, item->declaration);
                    param_type = find_defined_type(node->scope, params[i]->static_type);
                } else if (item->return_type) {
                    param_type = (Symbol*)malloc(sizeof(Symbol));
                    param_type->name = item->return_type->name;
                    param_type->type = item->return_type;
                    free_type = 1;
                }
    
                if (!param_type) {
                    report_error(
                        v, "Parameter '%s' was defined as '%s', but that type produces a "
                        "cirular reference. Line: %d.", params[i]->data.variable_name, 
                        params[i]->static_type, node->line
                    );
                }
            } else {
                report_error(
                    v, "Parameter '%s' was defined as '%s', which is not a valid type. Line: %d.", 
                    params[i]->data.variable_name, params[i]->static_type, node->line
                );
                params[i]->return_type = &TYPE_ERROR;
            }
        } else if (strcmp(params[i]->static_type, "") && type_equals(param_type->type, &TYPE_VOID)) {
            report_error(
                v, "Parameter '%s' was defined as 'Void', which is not a valid type. Line: %d.", 
                params[i]->data.variable_name, node->line
            );
            params[i]->return_type = &TYPE_ERROR;
        }

        if (!strcmp(params[i]->static_type, "")) {
            params[i]->return_type = &TYPE_ANY;
        } else if (param_type) {
            params[i]->return_type = param_type->type;
        }

        declare_symbol(
            node->scope, params[i]->data.variable_name,
            params[i]->return_type, 1, NULL
        );

        if (free_type)
            free(param_type);
    }

    Symbol* parent_info = find_defined_type(node->scope, node->data.type_node.parent_name);
    Type* parent_type = &TYPE_OBJECT;

    if (strcmp(node->data.type_node.parent_name, "") && !parent_info) {
        ContextItem* item = find_context_item(
            node->context, 
            node->data.type_node.parent_name, 1, 0
        );
        if (!item) {
            report_error(
                v, "Type '%s' inherits from '%s', which is not a valid type. Line: %d.", 
                node->data.type_node.name, node->data.type_node.parent_name, node->line
            );
        } else {
            int found = 0;
            if (!item->declaration->checked) {
                accept(v, item->declaration);
                parent_info = find_defined_type(node->scope, node->data.type_node.parent_name);
            } else if (item->return_type) {
                parent_type = item->return_type;
                found = 1;
            }
            
            if (!parent_info && !found) {
                report_error(
                    v, "Type '%s' inherits from '%s', but that type produces a "
                    "cirular reference that can not be solved. Line: %d.", node->data.type_node.name, 
                    node->data.type_node.parent_name, node->line
                );
                node->checked = 0;
                mro_list = empty_mro_list(mro_list);
                return;
            } else if (!found) {
                parent_type = parent_info->type;
            }

            if (!is_builtin_type(parent_type)) {
                // node->scope->parent = parent_type->dec->scope;
                // node->context->parent = parent_type->dec->context;
                // node->scope = copy_scope_symbols(parent_type->dec->scope, node->scope);
            }
            // node->scope = copy_scope_symbols(parent_type->dec->scope, node->scope);
        }
    } else if (
        strcmp(node->data.type_node.parent_name, "") && 
        is_builtin_type(parent_info->type)
    ) {
        report_error(
            v, "Type '%s' can not inherit from '%s'. Line: %d.", 
            node->data.type_node.name, node->data.type_node.parent_name, node->line
        );
    } else if (strcmp(node->data.type_node.parent_name, "")) {
        parent_type = parent_info->type;
        // node->scope->parent = parent_type->dec->scope;
        // node->context->parent = parent_type->dec->context;
        // node->scope = copy_scope_symbols(parent_type->dec->scope, node->scope);
    }

    if (!is_builtin_type(parent_type) && !node->data.type_node.arg_count) {
        node->data.type_node.arg_count = parent_type->arg_count;
        node->data.type_node.args = malloc(sizeof(ASTNode*) * parent_type->arg_count);

        for (int i = 0; i < parent_type->arg_count; i++) {
            node->data.type_node.args[i] = parent_type->dec->data.type_node.args[i];
            node->data.type_node.args[i]->line = node->line;
        }

        params = node->data.type_node.args;

        for (int i = 0; i < node->data.type_node.arg_count; i++) {
            params[i]->scope->parent = node->scope;
            params[i]->context->parent = node->context;

            declare_symbol(
                node->scope, params[i]->data.variable_name,
                params[i]->return_type, 1, NULL
            );
        }
        
    } else if (!is_builtin_type(parent_type)) {
        ASTNode* parent = create_type_instance_node(
            parent_type->name,
            node->data.type_node.p_args,
            node->data.type_node.p_arg_count
        );

        parent->scope->parent = node->scope;
        parent->context->parent = node->context;
        parent->line = node->line;
        accept(v, parent);
    }

    // if (parent_type)
    //     node->data.type_node.parent = parent_type;
    mro_list = empty_mro_list(mro_list);

    Type* this = create_new_type(node->data.type_node.name, parent_type, NULL, 0, node);
    // this->context = node->context;
    // this->scope = create_scope(node->scope);
    // this->scope = node->scope;
    ContextItem* item = find_context_item(node->context->parent, this->name, 1, 0);
    item->return_type = this;
    v->current_type = this;

    for (int i = 0; i < node->data.type_node.def_count; i++) {
        ASTNode* child =  definitions[i];
        child->context->parent = node->context;
        child->scope->parent = node->scope;

        if (!save_context_for_type(node->context, child, this->name)) {
            char* name = child->type == NODE_FUNC_DEC ?
                child->data.func_node.name :
                child->data.op_node.left->data.variable_name;
            report_error(
                v, "Member '%s' already exists in type '%s'. Line: %d.", 
                name, this->name, child->line
            );
        }
    }

    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = definitions[i];
        if (current->type == NODE_ASSIGNMENT) {
            accept(v, current);
        }
    }
    
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* sym = find_parameter(
            node->scope, node->data.type_node.args[i]->data.variable_name
        );
        sym->is_type_param = 1;
    }

    declare_symbol(node->scope, "self", this, 0, NULL);
    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = definitions[i];
        if (current->type == NODE_FUNC_DEC) {
            check_function_dec(v, current, this);
        }
    }

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* sym = find_parameter(
            node->scope, node->data.type_node.args[i]->data.variable_name
        );
        sym->is_type_param = 0;
    }

    Type** param_types = find_types(params, node->data.type_node.arg_count);

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* param = find_parameter(node->scope, params[i]->data.variable_name);

        if (type_equals(param->type, &TYPE_ANY)) {
            
            for (int i = 0; i < node->data.type_node.def_count; i++) {
                accept(v, definitions[i]);
            }

            if (type_equals(param->type, &TYPE_ANY)) {
                report_error(
                    v, "Impossible to infer type of parameter '%s' in type '%s'."
                    " It must be type annotated. Line: %d.", param->name,
                    node->data.type_node.name, node->line
                );
            }
        }

        Symbol* def_type = find_defined_type(node->scope, params[i]->static_type);
        param_types[i] = def_type?  def_type->type : param->type;
        node->data.type_node.args[i]->return_type = param_types[i];
    }

    this->dec = node;
    this->arg_count = node->data.type_node.arg_count;
    this->param_types = param_types;
    declare_type(node->scope->parent, this);
    mro_list = empty_mro_list(mro_list);
    v->current_type = visitor_type;
}

void visit_type_instance(Visitor* v, ASTNode* node) {
    ASTNode** args = node->data.type_node.args;

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        args[i]->context->parent = node->context;
        accept(v, args[i]);
    }

    if (match_as_keyword(node->data.type_node.name)) {
        node->return_type = &TYPE_ERROR;
        report_error(
            v, "Keyword '%s' can not be used in type instance. Line: %d.",
            node->data.type_node.name, node->line
        );
        return;
    }

    ContextItem* item = find_context_item(
        node->context, node->data.type_node.name, 1, 0
    );

    if (item) {
        accept(v, item->declaration);
    }

    IntList* unified = unify_type(
        v, args, node->scope, node->data.type_node.arg_count, 
        node->data.type_node.name, item
    );

    for (IntList* current = unified; current != NULL; current = current->next)
    {
        accept(v, args[current->value]);
    }

    free_int_list(unified);
    

    Type** args_types = find_types(args, node->data.type_node.arg_count);

    Function* f = (Function*)malloc(sizeof(Function));
    f->name = node->data.type_node.name;
    f->arg_count = node->data.type_node.arg_count;
    f->args_types = args_types;

    Function* dec = NULL;

    if (item && item->return_type) {
        Type** dec_args_types = find_types(
            item->declaration->data.type_node.args, 
            item->declaration->data.type_node.arg_count
        );

        dec = (Function*)malloc(sizeof(Function));
        dec->name = item->declaration->data.type_node.name;
        dec->arg_count = item->declaration->data.type_node.arg_count;
        dec->args_types = dec_args_types;
        dec->result_type = item->return_type;
    } else if (item) {
        node->return_type = &TYPE_ERROR;
        report_error(
            v, "Type '%s' is inaccesible. Line: %d.",
            node->data.type_node.name, node->line
        );
        return;
    }

    FuncData* funcData = find_type_data(node->scope, f, dec);

    if (funcData->func && funcData->func->result_type) {
        node->return_type = funcData->func->result_type;
    } else if (funcData->func) {
        node->return_type = create_new_type(
            node->data.type_node.name, NULL, NULL, 0, item->declaration
        );
    }

    if (!funcData->state->matched) {
        if (!funcData->state->same_name) {
            node->return_type = &TYPE_ERROR;
            report_error(
                v, "Undefined type '%s'. Line: %d.",
                node->data.type_node.name, node->line
            );

        } else if (!funcData->state->same_count) {
            report_error(
                v, "Constructor of type '%s' receives %d argument(s),"
                " but %d was(were) given. Line: %d.",
                node->data.type_node.name, funcData->state->arg1_count, 
                funcData->state->arg2_count, node->line
            );
        } else {
            if (!strcmp(funcData->state->type2_name, "Error"))
                return;

            report_error(
                v, "Constructor of type '%s' receives '%s', not '%s' as argument %d. Line: %d.",
                node->data.type_node.name, funcData->state->type1_name, 
                funcData->state->type2_name, funcData->state->pos, node->line
            );
        }
    }

    free_tuple(funcData->state);
    free(args_types);
    free(f);
}

void visit_attr_getter(Visitor* v, ASTNode* node) {
    ASTNode* instance = node->data.op_node.left;
    ASTNode* member = node->data.op_node.right;

    instance->scope->parent = node->scope;
    instance->context->parent = node->context;
    member->context->parent = node->context;
    member->scope->parent = node->scope;

    accept(v, instance);
    Type* instance_type = find_type(instance);

    if (type_equals(instance_type, &TYPE_ERROR)) {
        node->return_type = &TYPE_ERROR;
        return;
    } else if (type_equals(instance_type, &TYPE_ANY)) {
        if (!unify_type_by_attr(v, node)) {
            node->return_type = &TYPE_ERROR;
            return;
        } else {
            accept(v, instance);
            instance_type = find_type(instance);
        }
    }

    // if (instance_type->dec) {
    //     member->context->parent = instance_type->dec->context;
    //     member->scope->parent = instance_type->dec->scope; 
    // }

    if (member->type == NODE_VARIABLE && ((
        instance->type == NODE_VARIABLE &&
        strcmp(instance->data.variable_name, "self"))
        || 
        instance->type != NODE_VARIABLE)
    ) {
        // instance_type->name = type_equals(instance_type, &TYPE_ANY)? 
        //     "(not matter)" : instance_type->name;
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

        Symbol* sym = get_type_attr(
            instance_type,
            member->data.variable_name
        );

        if (sym) {
            member->return_type = sym->type;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        } else {
            ContextItem* item = find_item_in_type(
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
                sym = get_type_attr(instance_type, member->data.variable_name);
                member->return_type = sym->type;
                member->is_param = sym->is_param;
                member->derivations = sym->derivations;
                // member->derivations = add_value_list(
                //     item->declaration->data.op_node.left,
                //     member->derivations
                // );
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

    node->return_type = find_type(member);
    node->derivations = add_value_list(member, node->derivations);
}

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

void visit_casting_type(Visitor* v, ASTNode* node) {
    ASTNode* exp = node->data.cast_test.exp;
    char* type_name = node->data.cast_test.type_name;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);
    Type* dynamic_type = find_type(exp);
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