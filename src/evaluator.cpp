#include "ast.h"
#include <unordered_map>
#include <cmath>
#include <stdexcept>

// Tabla de símbolos
std::unordered_map<std::string, ASTNode*> symbol_table;

// Funciones matemáticas predefinidas
double eval_math_function(const std::string& name, const std::vector<double>& args) {
    if (name == "sin") return sin(args[0]);
    if (name == "cos") return cos(args[0]);
    if (name == "sqrt") return sqrt(args[0]);
    if (name == "log") return log(args[0]);
    throw std::runtime_error("Función no definida: " + name);
}

// Evaluar un nodo del AST
ASTNode* evaluate(ASTNode* node) {
    if (!node) return nullptr;

    switch(node->type) {
        case NODE_NUMBER:
        case NODE_STRING:
        case NODE_BOOLEAN:
        case NODE_PI:
        case NODE_E:
            // Estos nodos se evalúan a sí mismos
            return node;
            
        case NODE_IDENTIFIER: {
            // Buscar en la tabla de símbolos
            auto it = symbol_table.find(node->data.string_value);
            if (it == symbol_table.end()) {
                throw std::runtime_error("Variable no definida: " + std::string(node->data.string_value));
            }
            return it->second;
        }
            
        case NODE_BINARY_OP: {
            ASTNode* left = evaluate(node->data.children.left);
            ASTNode* right = evaluate(node->data.children.right);
            
            if (left->type != NODE_NUMBER || right->type != NODE_NUMBER) {
                throw std::runtime_error("Operación binaria aplicada a tipos no numéricos");
            }
            
            double result;
            switch(node->data.op) {
                case OP_PLUS: result = left->data.number_value + right->data.number_value; break;
                case OP_MINUS: result = left->data.number_value - right->data.number_value; break;
                case OP_MULTIPLY: result = left->data.number_value * right->data.number_value; break;
                case OP_DIVIDE: result = left->data.number_value / right->data.number_value; break;
                default: throw std::runtime_error("Operador no implementado");
            }
            
            return create_number_node(result);
        }
            
        // Implementar otros casos...
            
        default:
            throw std::runtime_error("Tipo de nodo no implementado en el evaluador");
    }
}

// Función principal de evaluación
void evaluate_program(ASTNode* program) {
    try {
        ASTNode* result = evaluate(program);
        if (result) {
            print_ast(result); // O mostrar el resultado de otra forma
        }
    } catch (const std::exception& e) {
        std::cerr << "Error de evaluación: " << e.what() << std::endl;
    }
}