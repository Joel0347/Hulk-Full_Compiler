#include "type.h"

// Definir las instancias de tipos básicos
Type TYPE_NUMBER_INST = { TYPE_NUMBER, "number", NULL };
Type TYPE_STRING_INST = { TYPE_STRING, "string", NULL };
Type TYPE_VOID_INST = { TYPE_VOID, "void", NULL };
Type TYPE_UNKNOWN_INST = { TYPE_UNKNOWN, "unknown", NULL };
Type TYPE_ERROR_INST = { TYPE_ERROR, "error", NULL };

OperatorTypeRule operator_rules[] = {
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_ADD },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_SUB },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_MUL },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_DIV },
    { &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, &TYPE_NUMBER_INST, OP_POW },
    { &TYPE_NUMBER_INST, NULL, &TYPE_NUMBER_INST, OP_NEGATE },
};

int rules_count = sizeof(operator_rules) / sizeof(OperatorTypeRule);