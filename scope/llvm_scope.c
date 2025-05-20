#include "llvm_scope.h"
#include <stdlib.h>
#include <string.h>

static LLVMScope* current_scope = NULL;
void print_scope() {
    LLVMScope* scope = current_scope;
    int depth = 0;
    
    printf("Current Scope Hierarchy:\n");
    printf("------------------------\n");
    
    while (scope != NULL) {
        printf("Scope %d (Level: %d)\n", depth, depth);
        printf("-------------------\n");
        
        ScopeVarEntry* entry = scope->variables;
        if (entry == NULL) {
            printf("  (empty)\n");
        } else {
            while (entry != NULL) {
                printf("  Variable: %-15s Alloca: %p\n", 
                      entry->name, (void*)entry->alloca);
                entry = entry->next;
            }
        }
        
        scope = scope->parent;
        depth++;
        printf("\n");
    }
    
    printf("End of Scope Chain\n");
    printf("============\n");
}

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