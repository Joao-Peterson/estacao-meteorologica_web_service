#ifndef _HTTP_SERVER_UTILS_
#define _HTTP_SERVER_UTILS_

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>
#include "fload_into_mem.h"
#include "microhttpd.h"
#include "router_uri.h"
#include "../routes/default/default.router.h"
#include "../routes/root.router.h"

/* ----------------------------------------- Definitions -------------------------------------- */

#define HTTP_OPTIONS_LEN_MAX 500

/* ----------------------------------------- Typedef's ---------------------------------------- */

/* ----------------------------------------- Structs ------------------------------------------ */

/* ----------------------------------------- Globals ------------------------------------------ */

char *last_client_ip = NULL;

char *client_url_complete = NULL;

/* ----------------------------------------- Functions ---------------------------------------- */

// calback for before a uri is parsed and handed to the response callback
void *on_uri_parsing(void *cls, const char *uri, struct MHD_Connection *con){
    http_options_t *options = http_parse_uri_options(uri);

    client_url_complete = uri;

    return (void *)options;
}

// callback for when a client connects 
static enum MHD_Result on_client_connect(void *cls, const struct sockaddr *addr, socklen_t addr_len){
    
    struct sockaddr_in *client = (struct sockaddr_in*)addr;

    last_client_ip = inet_ntoa(client->sin_addr);

    return MHD_YES;
}

// iterative function called on key value pairs of information from the client, called by libmicrohttp 
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

    if (dummy == 0){                                                                // The first time only the headers are valid, 
        dummy++;                                                                    // do not respond in the first round...
        return MHD_YES;
    }

    printf("\nServer ----------------------------\n");

    // MHD_get_connection_values(connection, MHD_HEADER_KIND, print_keys, NULL);

    http_options_t *options = *ptr;                                                 // retrieve options from URI
    http_options_t *cursor  = *ptr;
    
    // printf("Options: \n");
    // while(cursor != NULL){
    //     printf("    -%s: %s\n", cursor->name, cursor->value);
    //     cursor = cursor->next;
    // }
    
    time_t req_time_raw = time(NULL);
    struct tm *req_time = localtime(&req_time_raw);
    
    printf("[%s.%i] [Responding request] \n- Time: %s- Client: %s \n- URL: %s \n- Method: %s \n- Version: %s \n- Body: %s\n",
        __FILE__, __LINE__, asctime(req_time), last_client_ip, client_url_complete, method, version, upload_data);

    router_uri_t *router = router_root;                                             // retrieve root router

    if(router == NULL){ printf("[%s.%i] Unintialized root_router\n.", __FILE__, __LINE__); }

    struct MHD_Response *response = router_uri_route_request(router, url, method, options, upload_data, *upload_data_size, cls);

    int ret;

    if(response == NULL){
        response = router_uri_route_request(router_default, "/", method, options, upload_data, *upload_data_size, NULL);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    }
    else{
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    }

    if(ret == MHD_NO) dummy = 0;

    printf("-----------------------------------\n\n");

    MHD_destroy_response(response);
    http_delete_options(options);
    return (enum MHD_Result)ret;
}

#endif