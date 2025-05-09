#include <string.h>

typedef struct Tuple {
    int matched;
    int same_name;
    int same_count;
    int arg1_count;
    int arg2_count;
    char* type1_name;
    char* type2_name;
    int pos;
} Tuple;

typedef struct IntList {
    int value;
    struct IntList* next;
} IntList;

typedef struct StrList {
    char* value;
    struct StrList* next;
} StrList;

Tuple* init_tuple_for_types(int matched, char* type1_name, char* type2_name, int pos);
Tuple* init_tuple_for_count(int matched, int arg1_count, int arg2_count);
IntList* add_int_list(IntList* list, int number);
StrList* to_set(char**list, int count);
void free_int_list(IntList* list);
void free_str_list(StrList* list);
void free_tuple(Tuple* tuple);