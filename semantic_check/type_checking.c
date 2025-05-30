#include "semantic.h"

MRO* mro_list;

void visit_attr_setter(Visitor* v, ASTNode* node) {
    ASTNode* instance = node->data.cond_node.cond;
    ASTNode* member = node->data.cond_node.body_true;
    ASTNode* value = node->data.cond_node.body_false;

    value->scope->parent = node->scope;
    value->context->parent = node->context;
    ASTNode* member_call = create_attr_getter_node(instance, member);
    member_call->scope->parent = node->scope;
    member_call->context->parent = node->context;

    accept(v, member_call);

    Symbol* sym = find_defined_type(
        member_call->scope,
        find_type(instance)->name
    );

    if (sym) {
        ASTNode* assig = create_assignment_node(sym->name, value, "", NODE_D_ASSIGNMENT);
        accept(v, assig);
        node->return_type = find_type(assig);
    } else {
        accept(v, value);
        node->return_type = find_type(value);
    }

    node->derivations = add_value_list(value, NULL);
}

void visit_type_dec(Visitor* v, ASTNode* node) {
    if (node->checked)
        return;

    node->checked = 1;
    mro_list = add_type_to_mro(node->data.type_node.name, mro_list);
    
    if (find_type_in_mro(node->data.type_node.parent_name, mro_list)) {
        report_error(
            v, "Circular inheritance detected. Line: %d.", node->line
        );
        mro_list = empty_mro_list(mro_list);
        return;
    }

    ASTNode** params = node->data.type_node.args;
    ASTNode** definitions = node->data.type_node.definitions;

    for (int i = 0; i < node->data.type_node.arg_count - 1; i++) {
        for (int j = 1; j < node->data.type_node.arg_count; j++) {
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

        if (strcmp(params[i]->static_type, "") && !param_type) {
            report_error(
                v, "Parameter '%s' was defined as '%s', which is not a valid type. Line: %d.", 
                params[i]->data.variable_name, params[i]->static_type, node->line
            );
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
    }

    Symbol* parent_info = find_defined_type(node->scope, node->data.type_node.parent_name);
    Type* parent_type = NULL;

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
            parent_type = &TYPE_OBJECT;
        } else {
            accept(v, item->declaration);
            parent_info = find_defined_type(node->scope, node->data.type_node.parent_name);

            if (!parent_info) {
                parent_type = create_new_type(
                    node->data.type_node.parent_name, create_new_type(
                        node->data.type_node.name, NULL, NULL, 0
                    ), NULL, 0
                );
                parent_type->context = item->declaration->context;
                parent_type->scope = item->declaration->scope;
                parent_type->dec = item->declaration;
            } else {
                parent_type = parent_info->type;
            }

            // node->scope->parent = parent_type->dec->scope;
            // node->context->parent = parent_type->dec->context;

            node->scope->symbols = parent_type->dec->scope->symbols;
        }
    } else if (
        strcmp(node->data.type_node.parent_name, "") && 
        is_builtin_type(parent_info->type)
    ) {
        report_error(
            v, "Type '%s' can not inherit from '%s'. Line: %d.", 
            node->data.type_node.name, node->data.type_node.parent_name, node->line
        );
        parent_type = &TYPE_OBJECT;
    } else if (!strcmp(node->data.type_node.parent_name, "")) {
        parent_type = &TYPE_OBJECT;
    } else {
        parent_type = parent_info->type;
        node->scope->parent = parent_type->dec->scope;
        node->context->parent = parent_type->dec->context;
    }

    if (parent_type && !is_builtin_type(parent_type) && !node->data.type_node.arg_count) {
        Type* parent = parent_info->type;
        node->data.type_node.arg_count = parent->arg_count;
        node->data.type_node.args = malloc(sizeof(ASTNode*) * parent->arg_count);
        for (int i = 0; i < parent->arg_count; i++)
        {
            node->data.type_node.args[i] = parent->dec->data.type_node.args[i];
            node->data.type_node.args[i]->line = node->line;
        }

        params = node->data.type_node.args;
    } else if (parent_type && !is_builtin_type(parent_type)) {
        ASTNode* parent = create_type_instance_node(
            parent_type->name,
            node->data.type_node.p_args,
            node->data.type_node.p_arg_count
        );
        parent->scope->parent = node->scope; // con mas de un  nivel de herencia no sirve
        parent->context->parent = node->context;
        accept(v, parent);
    }

    // if (parent_type)
    //     node->data.type_node.parent = parent_type;

    Type* this = create_new_type(node->data.type_node.name, NULL, NULL, 0);
    this->parent = parent_type? parent_type : NULL;
    this->context = node->context;
    this->scope = create_scope(node->scope);

    for(int i = 0; i < node->data.type_node.def_count; i++) {
        ASTNode* child =  definitions[i];
        child->context->parent = this->context;
        child->scope->parent = this->scope;

        if (!save_context_for_type(this->context, child, this->name)) {
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
    
    // for (int i = 0; i < node->data.type_node.arg_count; i++)
    // {
    //     Symbol* sym = find_parameter(
    //         this->scope, node->data.type_node.args[i]->data.variable_name
    //     );
    //     sym->name = concat_str_with_underscore(this->name, sym->name);
    // }

    declare_symbol(this->scope, "self", this, 0, NULL);

    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = definitions[i];
        if (current->type == NODE_FUNC_DEC) {
            check_function_dec(v, current, this);
        }
    }

    // for (int i = 0; i < node->data.type_node.arg_count; i++)
    // {
    //     Symbol* sym = find_parameter(
    //         this->scope, concat_str_with_underscore(
    //             this->name,
    //             node->data.type_node.args[i]->data.variable_name
    //         )
    //     );
    //     sym->name = delete_underscore_from_str(sym->name,this->name);
    // }
    Type** param_types = find_types(params, node->data.type_node.arg_count);

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* param = find_parameter(this->scope, params[i]->data.variable_name);

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
    Scope* priv_scope = create_scope(NULL);
    Context* priv_context = create_context(NULL);
    priv_scope->symbols = this->scope->symbols;
    priv_scope->functions = this->scope->functions;
    priv_context->first = this->context->first;
    this->context = priv_context;
    declare_type(priv_scope, this, node->scope->parent);
    mro_list = empty_mro_list(mro_list);
}

void visit_type_inst(Visitor* v, ASTNode* node) {
    ASTNode** args = node->data.type_node.args;

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        args[i]->context->parent = node->context;
        accept(v, args[i]);
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

    if (item) {
        Type** dec_args_types = find_types(
            item->declaration->data.type_node.args, 
            item->declaration->data.type_node.arg_count
        );

        dec = (Function*)malloc(sizeof(Function));
        dec->name = item->declaration->data.type_node.name;
        dec->arg_count = item->declaration->data.type_node.arg_count;
        dec->args_types = dec_args_types;
    }

    FuncData* funcData = find_type_data(node->scope, f, dec);

    if (funcData->func)
        node->return_type = create_new_type(
            node->data.type_node.name, NULL, NULL, 0
        );

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
    member->context->parent = instance->context;
    member->scope->parent = instance->scope;

    accept(v, instance);
    Type* instance_type = find_type(instance);

    if (type_equals(instance_type, &TYPE_ERROR)) {
        node->return_type = &TYPE_ERROR;
        return;
    }

    if (instance_type->dec) {
        member->context->parent = instance_type->dec->context;
        member->scope->parent = instance_type->dec->scope; 
    }

    if (member->type == NODE_VARIABLE && ((
        instance->type == NODE_VARIABLE &&
        strcmp(instance->data.variable_name, "self"))
        || 
        instance->type != NODE_VARIABLE)
    ) {
        report_error(
            v, "Impossible to access to '%s'  in type '%s' because all"
            " attributes are private. Line: %d.",
            member->data.variable_name, instance_type->name, node->line
        );
        node->return_type = &TYPE_ERROR;
    } else if (member->type == NODE_VARIABLE) {
        Symbol* sym = get_type_attr(
            instance_type,
            member->data.variable_name
        );

        if (sym) {
            member->return_type = sym->type;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        } else if (!type_equals(member->return_type, &TYPE_ERROR)) {
            member->return_type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' does not have an attribute named '%s'. Line: %d", 
                instance_type->name, member->data.variable_name, node->line
            );
        }
    }

    if (member->type == NODE_FUNC_CALL && !type_equals(instance_type, &TYPE_ERROR)) {
        Symbol* t = find_defined_type(node->scope, instance_type->name);

        if (!t) {
            ContextItem* t_item = find_context_item(node->context, instance_type->name, 1, 0);
            accept(v, t_item->declaration);
            instance_type->context = t_item->declaration->context;
            instance_type->scope = t_item->declaration->scope;
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
    Type* dynamic_type = find_type(exp);
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