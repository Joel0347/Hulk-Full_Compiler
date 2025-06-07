#include <string.h>

struct ASTNode;

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

typedef struct ListElement {
    struct ASTNode* value;
    struct ASTNode* next;
} ListElement;

typedef struct ValueList {
    ListElement* first;
    int count;
} ValueList;

typedef struct MRO {
    char* type_name;
    struct MRO* next;
} MRO;

Tuple* init_tuple_for_types(int matched, char* type1_name, char* type2_name, int pos);
Tuple* init_tuple_for_count(int matched, int arg1_count, int arg2_count);
IntList* add_int_list(IntList* list, int number);
MRO* add_type_to_mro(char* type_name, MRO* list);
MRO* empty_mro_list(MRO* list);
int find_type_in_mro(char* type_name, MRO* list);
StrList* to_set(char**list, int count);
ValueList* add_value_list(struct ASTNode* value, ValueList* list);
struct ASTNode* at(int index, ValueList* list);
char* concat_str_with_underscore(char* type, char* name);
char* delete_underscore_from_str(char* name, char* type);
char* append_question(const char *input);
void free_int_list(IntList* list);
void free_str_list(StrList* list);
void free_value_list(ValueList* list);
void free_tuple(Tuple* tuple);