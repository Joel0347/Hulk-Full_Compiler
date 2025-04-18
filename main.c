#include <stdio.h>
#include "./ast/ast.h"
#include "./code_generation/llvm_gen.h"
#include "./semantic_check/semantic.h"

extern int yyparse(void);
extern FILE *yyin;
extern ASTNode* root;

int main() {
    yyin = fopen("script.hulk", "r");
    if (!yyin) {
        perror("Error opening script.hulk");
        return 1;
    }

    if (!yyparse() && !analyze_semantics(root)) {
        fclose(yyin);
        
        // Añadir esta línea para imprimir el AST
        printf("\n🌳 Abstract Syntax Tree:\n");
        print_ast(root, 0);
        
        printf("\nGenerando código LLVM...\n");
        generate_llvm_code(root, "output.ll");
        printf("✅ Código LLVM generado en output.ll\n");
        
        free_ast(root);
        root = NULL;
    }
    
    return 0;
}