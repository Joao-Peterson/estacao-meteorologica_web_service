#ifndef _ROOT_ROUTER_HEADER_
#define _ROOT_ROUTER_HEADER_

#include "router_uri.h"
#include "api/api.router.h"

struct MHD_Response *router_root_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
    struct MHD_Response *response;
    FILE *file;
    char buffer[255] = {0};
    
    switch(method){
        case HTTP_GET:
        
            if(filename_auto == NULL){
                file = fopen("web/index.html", "r+b");                
            }
            else{
                snprintf(buffer, 500, "web/%s", filename_auto);
                file = fopen(buffer, "r+b");                
            }

            if(file == NULL) return NULL;

            size_t file_size = 0;
            char *file_stream = fload_into_mem(file, &file_size);
            
            response = MHD_create_response_from_buffer(file_size, (void *)file_stream, MHD_RESPMEM_MUST_FREE);
            MHD_add_response_header(response, "Content-Type", "text/html");
            MHD_add_response_header(response, "Server", "Weather_station_API/1.0");
            
            fclose(file);

        break;

        default:
            response = NULL;
        break;
    }

    return response;
}

router_uri_t *router_root;

void router_root_init(void){
    router_root = router_uri_new("/", 20, router_root_handler);
    router_uri_use(router_root, &router_api, router_api_init);
}

#endif