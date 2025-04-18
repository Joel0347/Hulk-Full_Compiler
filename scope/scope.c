#include "scope.h"
#include <stdlib.h>
#include <string.h>

Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->symbols = NULL;
    scope->parent = parent;
    return scope;
}

void destroy_scope(Scope* scope) {
    if (scope == NULL) {
        return;
    }

    Symbol* current = scope->symbols;
    while (current != NULL) {
        Symbol* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    free(scope);
}

void declare_symbol(Scope* scope, const char* name, Type* type) {    
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->next = scope->symbols;
    scope->symbols = symbol;
}

Symbol* find_symbol(Scope* scope, const char* name) {
    if (!scope) {
        return NULL;
    }

    Symbol* current = scope->symbols;
    while (current) {
        if (!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }
    
    if (scope->parent) {
        return find_symbol(scope->parent, name);
    }
    
    return NULL;
}