#ifndef _API_ROUTER_HEADER_
#define _API_ROUTER_HEADER_

#include "router_uri.h"
#include "today/today.router.h"
#include "historic/historic.router.h"
#include "fload_into_mem.h"

struct MHD_Response *router_api_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
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

router_uri_t *router_api;

void router_api_init(void){
    if(router_api == NULL)
        router_api = router_uri_new("/api", 20, router_api_handler);

    router_uri_use(router_api, &router_today, router_today_init);
    router_uri_use(router_api, &router_historic, router_historic_init);
}

#endif