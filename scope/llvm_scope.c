#include "llvm_scope.h"
#include <stdlib.h>
#include <string.h>

static LLVMScope* current_scope = NULL;

void push_scope(void) {
    LLVMScope* new_scope = malloc(sizeof(LLVMScope));
    new_scope->variables = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

void pop_scope(void) {
    if (!current_scope) return;
    
    // Liberar variables del scope actual
    ScopeVarEntry* current = current_scope->variables;
    while (current) {
        ScopeVarEntry* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    
    LLVMScope* parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
}

LLVMValueRef lookup_variable(const char* name) {
    LLVMScope* scope = current_scope;
    while (scope) {
        ScopeVarEntry* entry = scope->variables;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return entry->alloca;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

void declare_variable(const char* name, LLVMValueRef alloca) {
    ScopeVarEntry* entry = malloc(sizeof(ScopeVarEntry));
    entry->name = strdup(name);
    entry->alloca = alloca;
    entry->next = current_scope->variables;
    current_scope->variables = entry;
}

void update_variable(const char* name, LLVMValueRef new_alloca) {
    LLVMScope* scope = current_scope;
    while (scope) {
        ScopeVarEntry* entry = scope->variables;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                entry->alloca = new_alloca;
                return;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
}