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

void save_context_item(Context* context, struct ASTNode* item) {
    ContextItem* new = (ContextItem*)malloc(sizeof(ContextItem));
    new->declaration = item;
    new->next = context->first;
    context->first = new;
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