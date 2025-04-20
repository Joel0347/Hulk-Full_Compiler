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

Tuple* init_tuple_for_types(int matched, char* type1_name, char* type2_name, int pos);
Tuple* init_tuple_for_count(int matched, int arg1_count, int arg2_count);
void free_tuple(Tuple* tuple);