#include "ast.h"
#include <fstream>
#include <cstring>

// Variables globales para Flex/Bison
extern FILE* yyin;
extern int yyparse();
extern ASTNode* ast_root;

// Función para leer el contenido de un archivo
std::string read_file(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)), 
                      std::istreambuf_iterator<char>());
}

int main(int argc, char* argv[]) {
    // Configurar la entrada
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "Uso: " << argv[0] << " [archivo.hulk]\n";
            std::cout << "Si no se proporciona archivo, se lee de stdin.\n";
            return 0;
        }
        
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            std::cerr << "Error: No se pudo abrir el archivo " << argv[1] << std::endl;
            return 1;
        }
    } else {
        yyin = stdin;
    }

    // Parsear el input
    int parse_result = yyparse();
    if (parse_result != 0) {
        std::cerr << "Error durante el parsing." << std::endl;
        return 1;
    }

    // Mostrar el AST (para depuración)
    if (ast_root) {
        std::cout << "=== AST ===" << std::endl;
        print_ast(ast_root);
        std::cout << "===========" << std::endl;
        
        // Aquí iría la evaluación o compilación del AST
        // evaluate(ast_root);
        
        // Liberar memoria
        free_ast(ast_root);
    }

    return 0;
}