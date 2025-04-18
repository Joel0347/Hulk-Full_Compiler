#ifndef VISITOR_H
#define VISITOR_H

#include "../ast/ast.h"
#include "../scope/scope.h"
#include "../type/type.h"
#include <string.h>

typedef struct Visitor Visitor;

typedef struct {
    TypeKind type;
} TypeInfo;

typedef void (*VisitProgram)(Visitor*, ASTNode*);
typedef void (*VisitNumber)(Visitor*, ASTNode*);
typedef void (*VisitString)(Visitor*, ASTNode*);
typedef void (*VisitBoolean)(Visitor*, ASTNode*);
typedef void (*VisitVariable)(Visitor*, ASTNode*);
typedef void (*VisitBinaryOp)(Visitor*, ASTNode*);
typedef void (*VisitUnaryOp)(Visitor*, ASTNode*);
typedef void (*VisitAssignment)(Visitor*, ASTNode*);
typedef void (*VisitPrint)(Visitor*, ASTNode*);

struct Visitor {
    int error_count;
    
    // Funciones de visita
    VisitProgram visit_program;
    VisitNumber visit_number;
    VisitString visit_string;
    VisitBoolean visit_boolean;
    VisitVariable visit_variable;
    VisitBinaryOp visit_binary_op;
    VisitUnaryOp visit_unary_op;
    VisitAssignment visit_assignment;
    VisitPrint visit_print;
    
    // Para inferencia de tipos
    TypeKind (*type_checker)(Visitor*, ASTNode*);

    // errores
    char** errors;
};

void accept(Visitor* visitor, ASTNode* node);
void add_error(char*** array, int* count, const char* str);
void free_error(char** array, int count);

#endif