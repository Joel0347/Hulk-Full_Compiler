#include "utils.h"
#include <stdlib.h>


//<----------STRING---------->

// method to concatenate the type name with name and starting with '_'
// Ex: type: A, name: foo -> _A_foo 
char* concat_str_with_underscore(char* type, char* name) {
    if (name[0] == '_')
        return name;
        
    int n = strlen(type) + strlen(name) + 3;
    char* new_s = (char*)malloc(n);
    snprintf(new_s, n, "_%s_%s", type, name);
    return new_s;
}

// method to delete the prefix from name that contains the type name.
// Ex: name: _A_foo, type: A -> foo
char* delete_underscore_from_str(char* name, char* type) {
    int len = strlen(type);
    const char *s1_ptr = name + len + 2;

    return strdup(s1_ptr);
}

// method to append question mark to a string
char* append_question(const char *input) {
    int len = strlen(input);
    char *result = malloc((len + 2) * sizeof(char));
    
    strcpy(result, input);
    result[len] = '?';
    result[len + 1] = '\0';
    
    return result;
}


//<----------TUPLE---------->

// method to create a tuple containing different types
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

// method to create a tuple containing different counts
Tuple* init_tuple_for_count(int matched, int arg1_count, int arg2_count) {
    Tuple* tuple = malloc(sizeof(Tuple));
    tuple->same_name = 1;
    tuple->arg1_count = arg1_count;
    tuple->arg2_count = arg2_count;
    tuple->matched = matched;
    tuple->same_count = matched;

    return tuple;
}

// method to free a tuple
void free_tuple(Tuple* tuple) {
    free(tuple);
}


//<----------INT_LIST---------->

// method to add an element to the list 
IntList* add_int_list(IntList* list, int number) {
    IntList* element = (IntList*)malloc(sizeof(IntList));
    element->value = number;
    element->next = list;

    return element;
}

// method to free an int list
void free_int_list(IntList* list) {
    if (list && list->next)
        free_int_list(list->next);
    free(list);
}


//<----------STR_LIST---------->

// method to add a string to the list
StrList* add_str_list(StrList* list, char* s) {
    StrList* element = (StrList*)malloc(sizeof(StrList));
    element->value = s;
    element->next = list;

    return element;
}

// method to check whether or not a list contains a string
int contains_str(StrList* l, char* s) {
    while (l)
    {
        if (!strcmp(l->value, s))
            return 1;

        l = l->next;
    }
    
    return 0;
}

// method to remove duplicates
StrList* to_set(char**list, int len) {
    StrList* result = NULL;
    for (int i = len - 1; i >= 0; i--)
    {
        if (!contains_str(result, list[i]))
            result = add_str_list(result, list[i]);
    }
    
    return result;
}

// method to free a string list
void free_str_list(StrList* list) {
    if (list && list->next)
        free_str_list(list->next);
    free(list);
}


//<----------NODE_LIST---------->

// method to add a new node to the list
NodeList* add_node_list(struct ASTNode* value, NodeList* list) {
    NodeList* new_list = (NodeList*)malloc(sizeof(NodeList));
    NodeElement* element = (NodeElement*)malloc(sizeof(NodeElement));
    element->value = value;
    element->next = NULL;

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

// method to get a node at a specific index
struct ASTNode* at(int index, NodeList* list) {
    if (!list || index > list->count || index < 0) {
        return NULL;
    }

    NodeElement* current = list->first;
    int i = 0;
    while (i < list->count)
    {  
        if (index == i) {
            return current->value;
        }
    
        i++;
        current = current->next;
    }
    
    return NULL;
}

// method to free a node list element
void free_node_list_element(NodeElement* element) {
    if (element) {
        element->value = NULL;
        free_node_list_element(element->next);
    }

    free(element);
}

// method to free a node list
void free_node_list(NodeList* list) {
    if (list) {
        free_node_list_element(list->first);
    }
    free(list);
}


//<----------MRO---------->

// method to add a new type to the mro list
MRO* add_type_to_mro(char* type_name, MRO* list) {
    if (!strcmp(type_name, ""))
        return list;

    MRO* new_list = (MRO*)malloc(sizeof(MRO));
    new_list->type_name = type_name;
    new_list->next = list;
    return new_list;
}

// method to empty the mro list
MRO* empty_mro_list(MRO* list) {
    if (list)
        free(list->next);
    free(list);
    return NULL;
}

// method to check whether or not a type is in the mro list
int find_type_in_mro(char* type_name, MRO* list) {
    if (!list) {
        return 0;
    }

    if (!strcmp(list->type_name, type_name)) {
        return 1;
    }

    return find_type_in_mro(type_name, list->next);
}