#include "semantic.h"


void visit_function_call(Visitor* v, ASTNode* node) {
    ASTNode** args = node->data.func_node.args;

    for (int i = 0; i < node->data.func_node.arg_count; i++)
    {
        args[i]->scope->parent = node->scope;
        accept(v, args[i]);
    }

    Type** args_types = find_types(args, node->data.func_node.arg_count);

    Function* f = (Function*)malloc(sizeof(Function));
    f->name = node->data.func_node.name;
    f->arg_count = node->data.func_node.arg_count;
    f->args_types = args_types;

    FuncData* funcData = find_function(node->scope, f);

    if (funcData->func)
        node->return_type = funcData->func->result_type;

    if (!funcData->state->matched) {
        if (!funcData->state->same_name) {
            node->return_type = &TYPE_UNKNOWN_INST;
            char* str = NULL;
            asprintf(&str, "Undefined function '%s'. Line: %d.",
                node->data.func_node.name, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        } else if (!funcData->state->same_count) {
            char* str = NULL;
            asprintf(&str, "Function '%s' receives %d argument(s), but %d was(were) given. Line: %d.",
                node->data.func_node.name, funcData->state->arg1_count, 
                funcData->state->arg2_count, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        } else {
            if (!strcmp(funcData->state->type2_name, "error"))
                return;

            char* str = NULL;
            asprintf(&str, "Function '%s' receives '%s', not '%s' as argument %d. Line: %d.",
                node->data.func_node.name, funcData->state->type1_name, 
                funcData->state->type2_name, funcData->state->pos, node->line
            );
            add_error(&(v->errors), &(v->error_count), str);
        }
    }

    free_tuple(funcData->state);
    free(args_types);
    free(f);
}