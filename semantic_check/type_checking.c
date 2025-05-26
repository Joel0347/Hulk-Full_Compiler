#include "semantic.h"


void visit_type_dec(Visitor* v, ASTNode* node) {
    if (node->checked)
        return;

    node->checked = 1;

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
        report_error(
            v, "Type '%s' inherits from '%s', which is not a valid type. Line: %d.", 
            node->data.type_node.name, node->data.type_node.parent_name, node->line
        );
        parent_type = &TYPE_OBJECT;
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

    if (parent_info && !node->data.type_node.arg_count) {
        Type* parent = parent_info->type;
        node->data.type_node.arg_count = parent->arg_count;
        node->data.type_node.args = malloc(sizeof(ASTNode*) * parent->arg_count);
        for (int i = 0; i < parent->arg_count; i++)
        {
            node->data.type_node.args[i] = parent->dec->data.type_node.args[i];
            node->data.type_node.args[i]->line = node->line;
        }

        params = node->data.type_node.args;
    } else if (parent_info) {
        ASTNode* parent = create_type_instance(
            node->data.type_node.parent_name,
            node->data.type_node.p_args,
            node->data.type_node.p_arg_count
        );
        parent->scope->parent = node->scope;
        parent->context->parent = node->context;
        accept(v, parent);
    }

    // buscar los tipos tambien en el contexto

    for(int i = 0; i < node->data.type_node.def_count; i++) {
        ASTNode* child =  node->data.type_node.definitions[i];
        child->context->parent = node->context;
        child->scope->parent = node->scope;
        if (!save_context_for_type(node->context, child, node->data.type_node.name)) {
            char* name = child->type==NODE_FUNC_DEC ?
                child->data.func_node.name :
                child->data.op_node.left->data.variable_name;
            report_error(
                v, "Member '%s' already exists in type '%s'. Line: %d.", 
                name, node->data.type_node.name, child->line
            );
        }
    }

    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = node->data.type_node.definitions[i];
        if (current->type == NODE_ASSIGNMENT) {
            accept(v, current);
        }
    }

    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = node->data.type_node.definitions[i];
        if (current->type == NODE_FUNC_DEC) {
            accept(v, current);
        }
    }
    
    Type** param_types = find_types(params, node->data.type_node.arg_count);

    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* param = find_parameter(node->scope, params[i]->data.variable_name);

        if (type_equals(param->type, &TYPE_ANY)) {
            
            for (int i = 0; i < node->data.type_node.def_count; i++) {
                accept(v, node->data.type_node.definitions[i]);
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

    Type* new_type = create_new_type(
        node->data.type_node.name, parent_type,
        param_types, node->data.type_node.arg_count
    );
    new_type->dec = node;
    Scope* parent_scope = parent_info? 
        parent_type->dec->scope->parent : node->scope->parent;
    declare_type(node->scope, new_type, parent_scope);
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