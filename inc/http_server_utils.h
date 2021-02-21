#ifndef _HTTP_SERVER_UTILS_
#define _HTTP_SERVER_UTILS_

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "fload_into_mem.h"
#include "microhttpd.h"
#include "router_uri.h"
#include "../routes/default/default.router.h"

/* ----------------------------------------- Definitions -------------------------------------- */

#define HTTP_OPTIONS_LEN_MAX 500

/* ----------------------------------------- Typedef's ---------------------------------------- */

/* ----------------------------------------- Structs ------------------------------------------ */

/* ----------------------------------------- Globals ------------------------------------------ */

/* ----------------------------------------- Functions ---------------------------------------- */

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

    if (strcmp(method, "GET")) return MHD_NO;                                       // unexpected method
    

    if (dummy == 0){                                                                // The first time only the headers are valid, 
        dummy++;                                                                    // do not respond in the first round...
        return MHD_YES;
    }

    if (*upload_data_size != 0) return MHD_NO;                                      // upload data in a GET!?

    MHD_get_connection_values(connection, MHD_HEADER_KIND, print_keys, NULL);

    http_options_t *options = *ptr;                                                 // retrieve options from URI
    printf("Options: \n");
    while(options != NULL){
        printf("    -%s: %s\n", options->name, options->value);
        options = options->next;
    }
    printf("Responding HTML. url: %s - method: %s - version: %s - data: %s\n\n", url, method, version, upload_data);

    router_uri_t *router = (router_uri_t *)cls;                                     // retrieve root router

    struct MHD_Response *response = router_uri_route_request(router, url, method, options, upload_data, *upload_data_size, NULL);

    int ret;

    if(response == NULL){
        response = router_uri_route_request(router_default, "/", method, options, upload_data, *upload_data_size, NULL);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    }
    else{
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    }

    if(ret == MHD_NO) dummy = 0;

    MHD_destroy_response(response);

    return (enum MHD_Result)ret;
}

#endif