#ifndef _TODAY_ROUTER_HEADER_
#define _TODAY_ROUTER_HEADER_

#include "router_uri.h"
#include "my_custom_struct.h"
#include "doc.h"
#include "doc_json.h"
#include "doc_sql.h"
#include "win_res.h"

struct MHD_Response *router_today_handler(http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data){
    struct MHD_Response *response;

    my_custom_struct_t *mystruct = (my_custom_struct_t *)data;
    if(mystruct->magic_num != MY_CUSTOM_STRUCT_MAGIC_NUM) return NULL;

    char *select_query = get_win_resource_binary_data("select_query");
    doc *doc_sql = doc_sql_select_query(mystruct->db, select_query, "data");
    char *select_query_json = doc_stringify_json(doc_sql);

    response = MHD_create_response_from_buffer(strlen(select_query_json), (void *)select_query_json, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "text/json");
    MHD_add_response_header(response, "Server", "Weather_station_API/1.0");

    doc_delete(doc_sql, ".");
    free(select_query);

    return response;
}

router_uri_t *router_today;

void router_today_init(void){
    router_today = router_uri_new("/today", 20, router_today_handler);
}

#endif