#ifndef _HISTORIC_ROUTER_HEADER_
#define _HISTORIC_ROUTER_HEADER_

#include "router_uri.h"
#include "my_custom_struct.h"
#include "doc.h"
#include "doc_json.h"
#include "doc_sql.h"
#include "win_res.h"

struct MHD_Response *router_historic_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
    struct MHD_Response *response;
    my_custom_struct_t *mystruct = (my_custom_struct_t *)data;
    http_options_t *date1;
    http_options_t *date2;
    doc *doc_sql;

    switch(method){
        case HTTP_GET:
            if(mystruct->magic_num != MY_CUSTOM_STRUCT_MAGIC_NUM) return NULL;

            date1 = http_get_option("date1", options); 
            date2 = http_get_option("date2", options); 

            if(date1 == NULL || date2 == NULL) return NULL;

            char *select_query = get_win_resource_binary_data("select_query_interval");
            char query_buffer[500] = {0};
            snprintf(query_buffer, 500, select_query, date1->value, date2->value);

            doc_sql = doc_sql_select_query(mystruct->db, query_buffer, "data");

            if(doc_sql == NULL) return NULL;

            char *select_query_json = doc_stringify_json(doc_sql);

            response = MHD_create_response_from_buffer(strlen(select_query_json), (void *)select_query_json, MHD_RESPMEM_MUST_FREE);
            MHD_add_response_header(response, "Content-Type", "text/json");
            MHD_add_response_header(response, "Server", "Weather_station_API/1.0");

            doc_delete(doc_sql, ".");
            free(select_query);
        break;

        default:
            response = NULL;
        break;
    }

    return response;
}

router_uri_t *router_historic;

void router_historic_init(void){
    router_historic = router_uri_new("/historic", 20, router_historic_handler);
}

#endif