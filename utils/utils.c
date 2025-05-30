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

StrList* add_str_list(StrList* list, char* s) {
    StrList* element = (StrList*)malloc(sizeof(StrList));
    element->value = s;
    element->next = list;

    return element;
}

int contains_str(StrList* l, char* s) {
    while (l)
    {
        if (!strcmp(l->value, s))
            return 1;

        l = l->next;
    }
    
    return 0;
}

StrList* to_set(char**list, int len) {
    StrList* result = NULL;
    for (int i = len - 1; i >= 0; i--)
    {
        if (!contains_str(result, list[i]))
            result = add_str_list(result, list[i]);
    }
    
    return result;
}

ValueList* add_value_list(struct ASTNode* value, ValueList* list) {
    ValueList* new_list = (ValueList*)malloc(sizeof(ValueList));
    ListElement* element = (ListElement*)malloc(sizeof(ListElement));
    element->value = value;

    if (list) {
        element->next = list->first;
        new_list->first = element;
        new_list->count = list->count + 1;
    } else {
        new_list->first = element;
        new_list->count = 1;
    }

    return new_list;
}

struct ASTNode* at(int index, ValueList* list) {
    if (index > list->count || index < 0 || !list) {
        return NULL;
    }

    ListElement* current = list->first;
    
    while (current)
    {
        if (!index) {
            return current->value;
        }
        index--;
        current = current->next;
    }
    
    return NULL;
}

char* concat_str_with_underscore(char* s1, char* s2) {
    if (s2[0] == '_')
        return s2;
        
    int n = strlen(s1) + strlen(s2) + 3;
    char* new_s = (char*)malloc(n);
    snprintf(new_s, n, "_%s_%s", s1, s2);
    return new_s;
}

char* delete_underscore_from_str(char* s, char* sub_s) {
    int len = strlen(sub_s);
    const char *s1_ptr = s + len + 2;

    return strdup(s1_ptr);
}

MRO* add_type_to_mro(char* type_name, MRO* list) {
    if (!strcmp(type_name, ""))
        return list;

    MRO* new_list = (MRO*)malloc(sizeof(MRO));
    new_list->type_name = type_name;
    new_list->next = list;
    return new_list;
}

MRO* empty_mro_list(MRO* list) {
    if (list)
        free(list->next);
    free(list);
    return NULL;
}

int find_type_in_mro(char* type_name, MRO* list) {
    if (!list) {
        return 0;
    }

    if (!strcmp(list->type_name, type_name)) {
        return 1;
    }

    return find_type_in_mro(type_name, list->next);
}

void free_int_list(IntList* list) {
    if (list && list->next)
        free_int_list(list->next);
    free(list);
}

void free_str_list(StrList* list) {
    if (list && list->next)
        free_str_list(list->next);
    free(list);
}

void free_value_list_element(ListElement* element) {
    if (element) {
        free(element->value);
        free_value_list_element(element->next);
    }
}

void free_value_list(ValueList* list) {
    if (list) {
        free_value_list_element(list->first);
    }
    free(list);
}

void free_tuple(Tuple* tuple) {
    free(tuple);
}