#ifndef _HTTP_SERVER_UTILS_
#define _HTTP_SERVER_UTILS_

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "microhttpd.h"

/* ----------------------------------------- Definitions -------------------------------------- */

#define HTTP_OPTIONS_LEN_MAX 500

/* ----------------------------------------- Globals ------------------------------------------ */

const char *error_html = 
"<html><body>An internal server error has occurred!\
</body></html>";

/* ----------------------------------------- Typedef's ---------------------------------------- */

typedef struct http_options_t http_options_t;

/* ----------------------------------------- Structs ------------------------------------------ */

struct http_options_t{
    char *name;
    char *value;
    http_options_t *next;
};

/* ----------------------------------------- Functions ---------------------------------------- */

// copy file to memory
char *fload_into_mem(FILE *file, size_t *size){
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file); 
    fseek(file, 0, SEEK_SET);

    if(size != NULL)
        *size = file_size;

    char *file_buff = calloc(file_size + 1, sizeof(*file_buff));
    fread(file_buff, sizeof(*file_buff), file_size, file);

    return file_buff;
}

// parse options from a uri
http_options_t *http_parse_uri_options(const char *uri){
    if(uri == NULL)
        return NULL;

    size_t len = strlen(uri) + 1;
    char *uri_copy = (char *)calloc(len, sizeof(*uri));
    strncpy(uri_copy, uri, len);

    http_options_t *first_node = NULL;
    http_options_t *last_node = NULL;
    http_options_t *cursor = NULL;
    char *name  = uri_copy;
    size_t name_len;
    char *value = uri_copy;
    size_t value_len;


    name = strpbrk(uri_copy, "?");
    if(name == NULL)
        return NULL;

    name++;

    while(true){
        cursor = (http_options_t *)calloc(1, sizeof(*cursor));
        if(last_node == NULL)
            first_node = cursor;

        if(last_node != NULL)
            last_node->next = cursor;

        value = strpbrk(name, "=");
        *value = '\0';
        value++;

        name_len = strlen(name) + 1;
        cursor->name = calloc(name_len, sizeof(*(cursor->name)));
        strncpy(cursor->name, name, name_len);

        name = strpbrk(value, "&");                                                 // find pointer starting string
        if(name != NULL){
            *name = '\0';
            name++;

            value_len = strlen(value) + 1;
            cursor->value = calloc(value_len, sizeof(*(cursor->value)));
            strncpy(cursor->value, value, value_len);
        }
        else{
            value_len = strlen(value) + 1;
            cursor->value = calloc(value_len, sizeof(*(cursor->value)));
            strncpy(cursor->value, value, value_len);

            break;
        }

        last_node = cursor;
    }

    return first_node;
}

// calback for before a uri is parsed and handed to the response callback
void *on_uri_parsing(void *cls, const char *uri, struct MHD_Connection *con){
    //printf("URI: %s\n", uri);

    http_options_t *options = http_parse_uri_options(uri);

    return (void *)options;
}

// callback for when a client connects 
static enum MHD_Result on_client_connect(void *cls, const struct sockaddr *addr, socklen_t addr_len){
    
    struct sockaddr_in *client = (struct sockaddr_in*)addr;

    char *client_ip = inet_ntoa(client->sin_addr);
    
    printf("Client IP: %s\n",client_ip);

    return MHD_YES;
}

// iterative fucntion called on key value pairs of information from the client 
enum MHD_Result print_keys(void *cls, enum MHD_ValueKind kind, const char *key, const char *value){
    // printf("%s: %s\n", key, value);
    return MHD_YES;
}

// callback for sending a response for the client 
static enum MHD_Result on_response(
    void * cls,
    struct MHD_Connection *connection, 
    const char *url, 
    const char *method, 
    const char *version, 
    const char *upload_data, 
    size_t *upload_data_size, 
    void **ptr
){

    static int dummy = 0;

    if (strcmp(method, "GET"))
        return MHD_NO;                                                              // unexpected method
    

    if (dummy == 0){                                                                // The first time only the headers are valid, 
        dummy++;                                                                    // do not respond in the first round...
        return MHD_YES;
    }

    if (*upload_data_size != 0)                                                     // upload data in a GET!?
        return MHD_NO; 


    MHD_get_connection_values(connection, MHD_HEADER_KIND, print_keys, NULL);

    http_options_t *options = *ptr;
    printf("Options: \n");
    while(options != NULL){
        printf("    -%s: %s\n", options->name, options->value);
        options = options->next;
    }
    printf("Responding HTML. url: %s - method: %s - version: %s - data: %s\n\n", url, method, version, upload_data);

    struct MHD_Response *response;

    const char *filename = url + 1;
    FILE *file = fopen(filename, "r+b");

    if(file == NULL){
        response = MHD_create_response_from_buffer(strlen(error_html), (void *)error_html, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/html");
        MHD_add_response_header(response, "Server", "Weather_station_API/1.0");
        
        int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);

        MHD_destroy_response(response);
        return MHD_YES;
    }

    size_t file_size;
    char *file_buff = fload_into_mem(file, &file_size);
    
    fclose(file);

    response = MHD_create_response_from_buffer(file_size, (void *)file_buff, MHD_RESPMEM_MUST_FREE);

    MHD_add_response_header(response, "Content-Type", "text/html");
    MHD_add_response_header(response, "Server", "Weather_station_API/1.0");

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

    if(ret == MHD_NO)
        dummy = 0;

    MHD_destroy_response(response);

    return (enum MHD_Result)ret;
}

#endif