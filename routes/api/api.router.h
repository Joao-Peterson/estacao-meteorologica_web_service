#ifndef _API_ROUTER_HEADER_
#define _API_ROUTER_HEADER_

#include "router_uri.h"

struct MHD_Response *router_api_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
    struct MHD_Response *response;

    return NULL;
}

router_uri_t *router_api;

void router_api_init(void){
    router_api = router_uri_new("/api", 20, router_api_handler);
}

#endif