#include <stdio.h>
#include "./ast/ast.h"
#include "./code_generation/llvm_gen.h"

// Añade estas declaraciones externas
extern int yyparse(void);
extern FILE *yyin;
extern ASTNode* root;  // Definido en parser.y

int main() {
    
    yyin = fopen("script.hulk", "r");
    if (!yyin) {
        perror("Error opening script.hulk");
        return 1;
    }

    if (yyparse() == 0) {
        fclose(yyin);
        printf("\nGenerando código LLVM...\n");
        generate_llvm_code(root, "output.ll");  // Cambio aquí
        printf("✅ Código LLVM generado en output.ll\n");
        
        free_ast(root);
        root = NULL;
    }
    
    return 0;
}