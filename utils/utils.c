#include "utils.h"
#include <stdlib.h>

Tuple* init_tuple_for_types(int matched, char* type1_name, char* type2_name, int pos) {
    Tuple* tuple = malloc(sizeof(Tuple));
    tuple->matched = matched;
    tuple->same_name = 1;
    tuple->same_count = 1;
    tuple->pos = pos;
    tuple->type1_name = type1_name;
    tuple->type2_name = type2_name;

    return tuple;
}

Tuple* init_tuple_for_count(int matched, int arg1_count, int arg2_count) {
    Tuple* tuple = malloc(sizeof(Tuple));
    tuple->same_name = 1;
    tuple->arg1_count = arg1_count;
    tuple->arg2_count = arg2_count;
    tuple->matched = matched;
    tuple->same_count = matched;

    return tuple;
}

IntList* add_int_list(IntList* list, int number) {
    IntList* element = (IntList*)malloc(sizeof(IntList));
    element->value = number;
    element->next = list;

    return element;
}

void free_int_list(IntList* list) {
    if (list && list->next)
        free_int_list(list->next);
    free(list);
}


void free_tuple(Tuple* tuple) {
    free(tuple);
}