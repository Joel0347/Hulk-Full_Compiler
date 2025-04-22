#include "scope.h"
#include <stdlib.h>
#include <string.h>

Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->symbols = NULL;
    scope->functions = NULL;
    scope->defined_types = NULL;
    scope->parent = parent;
    return scope;
}

void free_symbol(Symbol* current_symbol) {
    while (current_symbol != NULL) {
        Symbol* next = current_symbol->next;
        free(current_symbol->name);
        free(current_symbol);
        current_symbol = next;
    }
}

void destroy_scope(Scope* scope) {
    if (scope == NULL) {
        return;
    }

    Symbol* lists_to_free[] = { 
        scope->symbols, 
        scope->functions, 
        scope->defined_types 
    };

    for (int i = 0; i < 3; i++)
        free_symbol(lists_to_free[i]);

    free(scope);
}

void declare_symbol(Scope* scope, const char* name, Type* type) {    
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->next = scope->symbols;
    scope->symbols = symbol;
}

void declare_function(Scope* scope, const char* name, Type* type) {    
    Symbol* func = (Symbol*)malloc(sizeof(Symbol));
    func->name = strdup(name);
    func->type = type;
    func->next = scope->functions;
    scope->functions = func;
}

void declare_type(Scope* scope, Type* type) {    
    Symbol* def_type = (Symbol*)malloc(sizeof(Symbol));
    def_type->name = strdup(type->name);
    def_type->type = type;
    def_type->next = scope->defined_types;
    scope->defined_types = def_type;
}

void init_basic_types(Scope* scope) {
    declare_type(scope, &TYPE_NUMBER_INST);
    declare_type(scope, &TYPE_STRING_INST);
    declare_type(scope, &TYPE_BOOLEAN_INST);
    declare_type(scope, &TYPE_VOID_INST);
    declare_type(scope, &TYPE_OBJECT_INST);
}

Symbol* find(Scope* scope, int index, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* symbols[] = { 
        scope->symbols, 
        scope->functions, 
        scope->defined_types 
    };

    Symbol* current = symbols[index];
    while (current) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }
    
    if (scope->parent) {
        return find(scope->parent, index, name);
    }
    
    return NULL;
}

Symbol* find_symbol(Scope* scope, const char* name) {
    return find(scope, 0, name);
}

Symbol* find_function(Scope* scope, const char* name) {
    return find(scope, 1, name);
}

Symbol* find_defined_type(Scope* scope, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* current = scope->defined_types;
    while (current) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }
    
    if (scope->parent) {
        return find_defined_type(scope->parent, name);
    }
    
    return NULL;
}