#ifndef _DEFAULT_ROUTER_HEADER_
#define _DEFAULT_ROUTER_HEADER_

#include "router_uri.h"
#include "fload_into_mem.h"

struct MHD_Response *router_default_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
    struct MHD_Response *response;
    
    FILE *file = fopen("web/error.html", "r+b");

    size_t file_size = 0;
    char *file_stream = fload_into_mem(file, &file_size);
    
    response = MHD_create_response_from_buffer(file_size, (void *)file_stream, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "text/html");
    MHD_add_response_header(response, "Server", "Weather_station_API/1.0");

    return response;
}

router_uri_t *router_default;

void router_default_init(void){
    router_default = router_uri_new("/", 20, router_default_handler);
}

#endif