#ifndef _TODAY_ROUTER_HEADER_
#define _TODAY_ROUTER_HEADER_

#include "router_uri.h"
#include "my_custom_struct.h"
#include "doc.h"
#include "doc_json.h"
#include "doc_sql.h"
#include "fload_into_mem.h"

struct MHD_Response *router_today_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
    struct MHD_Response *response;
    my_custom_struct_t *mystruct = (my_custom_struct_t *)data;
    doc *doc_sql;
    FILE *select_query_file;
    size_t len;

    switch(method){
        case HTTP_GET:
            if(mystruct->magic_num != MY_CUSTOM_STRUCT_MAGIC_NUM) return NULL;

            select_query_file = fopen("./sql/select_query.sql", "r+b");
            if(select_query_file == NULL)
                printf("[%s.%u]\n", __FILE__, __LINE__);
                
            char *select_query = fload_into_mem(select_query_file, &len);
            doc_sql = doc_sql_select_query(mystruct->db, select_query, "data");
            char *select_query_json = doc_stringify_json(doc_sql);

            response = MHD_create_response_from_buffer(strlen(select_query_json), (void *)select_query_json, MHD_RESPMEM_MUST_FREE);
            MHD_add_response_header(response, "Content-Type", "text/json");
            MHD_add_response_header(response, "Server", "Weather_station_API/1.0");

            doc_delete(doc_sql, ".");
            free(select_query);
            fclose(select_query_file);
        break;

        default:
            response = NULL;
        break;
    }

    return response;
}

router_uri_t *router_today;

void router_today_init(void){
    router_today = router_uri_new("/today", 20, router_today_handler);
}

#endif