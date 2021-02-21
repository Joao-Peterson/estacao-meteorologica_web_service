#ifndef _FLOAD_INTO_MEM_HEADER_
#define _FLOAD_INTO_MEM_HEADER_

#include <stdio.h>
#include <stdlib.h>

// copy file to memory
char *fload_into_mem(FILE *file, size_t *size){
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file); 
    fseek(file, 0, SEEK_SET);

    if(size != NULL)
        *size = file_size;

    char *file_buff = (char *)calloc(file_size + 1, sizeof(*file_buff));
    fread(file_buff, sizeof(*file_buff), file_size, file);

    return file_buff;
}

#endif