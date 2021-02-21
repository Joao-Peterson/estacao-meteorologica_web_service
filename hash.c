#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct data_t data_t;

struct data_t{
    char *name;
    int value;
    data_t *next;
};

typedef struct{
    data_t **row;
    size_t size;
}data_table_t;



size_t hash_data(char *name, size_t size){
    size_t acc = 0;

    for(size_t i = 0; i < strlen(name); i++)
        acc += name[i] + i;

    acc %= size;

    return acc;
}

void data_add(data_table_t table, data_t *data){
    size_t index = hash_data(data->name, table.size);
    data_t *cursor;

    if(table.row[index] == NULL){
        table.row[index] = data;
    }
    else{
        cursor = table.row[index];

        while(cursor->next != NULL)
            cursor = cursor->next;

        cursor->next = data;
    }
}

data_t *data_get(data_table_t table, char *name){
    size_t index = hash_data(name, table.size);
    data_t *cursor;

    cursor = table.row[index];
    while(strcmp(cursor->name, name))
        cursor = cursor->next;

    return cursor;
}

data_table_t data_new(size_t size){
    data_table_t table = {.row = NULL, .size = size};

    table.row = calloc(size, sizeof(*(table.row)));
    
    return table;
}

void data_print(data_table_t table){
    data_t *cursor;
    for(size_t i = 0; i < table.size; i++){
        cursor = table.row[i];
        while(cursor != NULL){
            printf("[%i] %s : %i\n", i, cursor->name, cursor->value);
            cursor = cursor->next;
        }
    }
}

int main(void){

    data_table_t table = data_new(20);

    data_add(table, &(data_t){"Carol", 25});
    data_add(table, &(data_t){"Carol", 35});
    data_add(table, &(data_t){"Carol", 45});
    data_add(table, &(data_t){"Carol", 55});
    data_add(table, &(data_t){"john", 23});
    data_add(table, &(data_t){"paulo", 46});
    data_add(table, &(data_t){"xixo", 75});
    data_add(table, &(data_t){"niino", 32});
    data_add(table, &(data_t){"jacko", 73});
    data_add(table, &(data_t){"bruh", 63});

    data_print(table);

    return 0;
}