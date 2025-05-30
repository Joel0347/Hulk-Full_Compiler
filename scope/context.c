#include "ast/ast.h"
#include <string.h>
#include <stdlib.h>

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
    int type = item->type==NODE_TYPE_DEC;
    char* name = !type ? 
        item->data.func_node.name : item->data.type_node.name;

    ContextItem* c_item = find_context_item(context, name, type, 0);

    if (c_item) {
        c_item->return_type = !type ? &TYPE_ANY : &TYPE_VOID;
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

struct ContextItem* find_context_item(Context* context, char* name, int type, int var) {
    if (!context) {
        return NULL;
    }

    ContextItem* current = context->first;
    while (current) {
        if ((!type && !var && current->declaration->type == NODE_FUNC_DEC &&
            !strcmp(current->declaration->data.func_node.name, name)) ||
            (type && !var && current->declaration->type == NODE_TYPE_DEC &&
            !strcmp(current->declaration->data.type_node.name, name)) ||
            (var && current->declaration->type == NODE_ASSIGNMENT &&
            !strcmp(current->declaration->data.op_node.left->data.variable_name, name))
        ) {
            return current;
        }
        current = current->next;
    }
    
    if (context->parent) {
        return find_context_item(context->parent, name, type, var);
    }
    
    return NULL;
}

struct ContextItem* find_item_in_type_context(Context* context, char* name, int func_dec) {
    if (!context) {
        return NULL;
    }
    
    ContextItem* current = context->first;

    while (current) {
        if ((func_dec && current->declaration->type == NODE_FUNC_DEC &&
            !strcmp(current->declaration->data.func_node.name, name)) ||
            (!func_dec && current->declaration->type == NODE_ASSIGNMENT &&
            !strcmp(current->declaration->data.op_node.left->data.variable_name, name))
        ) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int save_context_for_type(Context* context, struct ASTNode* item, char* type_name) {
    int func_dec = item->type == NODE_FUNC_DEC;
    char* name = func_dec ? 
        item->data.func_node.name : item->data.op_node.left->data.variable_name;

    char* new_name = concat_str_with_underscore(type_name, name);
    ContextItem* c_item = find_item_in_type_context(context, new_name, func_dec);

    if (c_item) {
        c_item->return_type = func_dec ? &TYPE_ANY : &TYPE_VOID;
        return 0;
    }
    
    ContextItem* new = (ContextItem*)malloc(sizeof(ContextItem));
    new->declaration = item;

    if (func_dec)
        new->declaration->data.func_node.name = new_name;
    else
        new->declaration->data.op_node.left->data.variable_name = new_name;

    Symbol* defined_type = find_defined_type(item->scope, item->static_type);

    if (defined_type)
        new->return_type = defined_type->type;

    new->next = context->first;
    context->first = new;

    return 1;
}

struct ContextItem* find_item_in_type(Context* context, char* name, Type* type, int func_dec) {
    ContextItem* item = find_item_in_type_context(context, name, func_dec);
    if (!item && type->parent && type->parent->context) {
        name = delete_underscore_from_str(name, type->name);
        name = concat_str_with_underscore(type->parent->name, name);
        return find_item_in_type(type->parent->context, name, type->parent, func_dec);
    }
    
    return item;
}
