#include "semantic.h"

MRO* mro_list; // list to save type names for catching circular inheritance

// method to visit type declaration node
void visit_type_dec(Visitor* v, ASTNode* node) {
    // checking circular inheritance
    if (find_type_in_mro(node->data.type_node.parent_name, mro_list)) {
        report_error(
            v, "Circular inheritance detected. Line: %d.", node->line
        );
        mro_list = empty_mro_list(mro_list);
        return;
    }

    // to prevent loops
    if (node->checked) {
        return;
    }

    node->checked = 1;
    Type* visitor_type = v->current_type;
    Scope* parent_scope = node->scope->parent;

    mro_list = add_type_to_mro(node->data.type_node.parent_name, mro_list);

    ASTNode** params = node->data.type_node.args;
    ASTNode** definitions = node->data.type_node.definitions;

    // Checking multiple uses of a symbol in parameters
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

    // Parameters checking
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        params[i]->scope->parent = node->scope;
        params[i]->context->parent = node->context;
        Symbol* param_type = find_defined_type(node->scope, params[i]->static_type);
        int free_type = 0;

        // Checking type annotation
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

        // Annotate parameters as Any if they weren't annotated
        if (!strcmp(params[i]->static_type, "")) {
            params[i]->return_type = &TYPE_ANY;
        } else if (param_type) {
            params[i]->return_type = param_type->type;
        }

        // Declare parameters in the type scope
        declare_symbol(
            node->scope, params[i]->data.variable_name,
            params[i]->return_type, 1, NULL
        );

        if (free_type)
            free(param_type);
    }

    Symbol* parent_info = find_defined_type(node->scope, node->data.type_node.parent_name);
    Type* parent_type = &TYPE_OBJECT;

    // Checking type parent
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
    }

    // Taking parent parameters if it doesn't have any
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
        // checking parent instantiation if there is a constructor defined
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

    mro_list = empty_mro_list(mro_list);

    // create actual type
    Type* this = create_new_type(node->data.type_node.name, parent_type, NULL, 0, node);
    ContextItem* item = find_context_item(node->context->parent, this->name, 1, 0);
    item->return_type = this;
    v->current_type = this; // save it as current in the visitor for 'base' node

    // Collecting context of the type
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

    // Checking attribute initializations
    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = definitions[i];
        if (current->type == NODE_ASSIGNMENT) {
            accept(v, current);
        }
    }
    
    // Hidding parameters for method declarations
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* sym = find_parameter(
            node->scope, node->data.type_node.args[i]->data.variable_name
        );
        sym->is_type_param = 1;
    }

    // Declaring 'self' symbol in order to make it available inside methods
    declare_symbol(node->scope, "self", this, 0, NULL);
    // Checking type methods
    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode* current = definitions[i];
        if (current->type == NODE_FUNC_DEC) {
            check_function_dec(v, current, this);
        }
    }

    // Returning changes made to parameters
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        Symbol* sym = find_parameter(
            node->scope, node->data.type_node.args[i]->data.variable_name
        );
        sym->is_type_param = 0;
    }

    Type** param_types = map_get_type(params, node->data.type_node.arg_count);

    // Checking that no parameter is annotated as Any
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

    // declaring the type

    if (parent_type && !is_builtin_type(parent_type)) {
        if (!node->data.type_node.p_arg_count) {
            node->data.type_node.parent_instance = create_type_instance_node(
                parent_type->name, node->data.type_node.args,
                node->data.type_node.arg_count
            );
        } else {
            node->data.type_node.parent_instance = create_type_instance_node(
                parent_type->name, node->data.type_node.p_args,
                node->data.type_node.p_arg_count
            );
        }
    }

    node->data.type_node.id = ++v->type_id;
    this->dec = node;
    this->arg_count = node->data.type_node.arg_count;
    this->param_types = param_types;
    declare_type(node->scope->parent, this);
    mro_list = empty_mro_list(mro_list);
    v->current_type = visitor_type;
}

// method to check type instance node
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
        accept(v, args[current->value]); // visit again if unified
    }

    free_int_list(unified);
    

    Type** args_types = map_get_type(args, node->data.type_node.arg_count);

    Function* f = (Function*)malloc(sizeof(Function));
    f->name = node->data.type_node.name;
    f->arg_count = node->data.type_node.arg_count;
    f->args_types = args_types;

    Function* dec = NULL;

    if (item && item->return_type) {
        Type** dec_args_types = map_get_type(
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

    // Trying to find a type signature that matches
    FuncData* funcData = find_type_data(node->scope, f, dec);

    if (funcData->func && funcData->func->result_type) {
        node->return_type = funcData->func->result_type;
        node->data.type_node.parent_instance = 
            funcData->func->result_type->dec->data.type_node.parent_instance;
    } else if (funcData->func) {
        node->return_type = create_new_type(
            node->data.type_node.name, NULL, NULL, 0, item->declaration
        );
    }

    if (!funcData->state->matched) {
        // Unpacking errors
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

    if(node->data.type_node.parent_instance) printf("%s\n", node->data.type_node.parent_instance->data.type_node.name);
    free_tuple(funcData->state);
    free(args_types);
    free(f);
}