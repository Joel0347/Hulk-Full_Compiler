#include "ast/ast.h"

Context* create_context(Context* parent) {
    Context* context = (Context*)malloc(sizeof(Context));
    context->first = NULL;
    context->parent = parent;

    return context;
}

void free_context_item(ContextItem* item) {
    if (item) {
        free_context_item(item->next);
    }

    free(item);
}

void destroy_context(Context* context) {
    if (context) {
        free_context_item(context->first);
    }

    free(context);
}

int save_context_item(Context* context, struct ASTNode* item) {
    ContextItem* func = find_context_item(context, item->data.func_node.name);

    if (func) {
        func->return_type = &TYPE_ANY;
        return 0;
    }
    
    ContextItem* new = (ContextItem*)malloc(sizeof(ContextItem));
    new->declaration = item;

    Symbol* defined_type = find_defined_type(item->scope, item->static_type);

    if (defined_type)
        new->return_type = defined_type->type;

    new->next = context->first;
    context->first = new;

    return 1;
}

struct ContextItem* find_context_item(Context* context, char* name) {
    if (!context) {
        return NULL;
    }

    ContextItem* current = context->first;
    while (current) {
        if (!strcmp(current->declaration->data.func_node.name, name)) {
            return current;
        }
        current = current->next;
    }
    
    if (context->parent) {
        return find_context_item(context->parent, name);
    }
    
    return NULL;
}