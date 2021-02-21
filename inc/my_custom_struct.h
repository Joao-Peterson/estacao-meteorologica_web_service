#ifndef _MY_CUSTOM_STRUCT_HEADER_
#define _MY_CUSTOM_STRUCT_HEADER_

#include <stdlib.h>
#include <stdint.h>
#include "mysql.h"
#include "doc.h"

#define MY_CUSTOM_STRUCT_MAGIC_NUM 0xF0F0F0F0F0F0F0F0ULL

typedef struct{
    size_t magic_num;
    MYSQL *db;
    doc *obj;
    char *stream;
    void *data;
}my_custom_struct_t;

#endif