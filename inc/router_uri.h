#ifndef _ROUTER_URI_HEADER_
#define _ROUTER_URI_HEADER_

#include "microhttpd.h"
#include <string.h>

/* ----------------------------------------- Definitions -------------------------------------- */

#define http_method_binder_macro(enum, string) {enum, string}

/* ----------------------------------------- Enum's ------------------------------------------- */

typedef enum{
    HTTP_CONNECT,
    HTTP_DELETE,
    HTTP_GET,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_POST,
    HTTP_PUT,
    HTTP_TRACE,
    HTTP_ACL,
    HTTP_BASELINE_CONTROL,
    HTTP_BIND,
    HTTP_CHECKIN,
    HTTP_CHECKOUT,
    HTTP_COPY,
    HTTP_LABEL,
    HTTP_LINK,
    HTTP_LOCK,
    HTTP_MERGE,
    HTTP_MKACTIVITY,
    HTTP_MKCALENDAR,
    HTTP_MKCOL,
    HTTP_MKREDIRECTREF,
    HTTP_MKWORKSPACE,
    HTTP_MOVE,
    HTTP_ORDERPATCH,
    HTTP_PATCH,
    HTTP_PRI,
    HTTP_PROPFIND,
    HTTP_PROPPATCH,
    HTTP_REBIND,
    HTTP_REPORT,
    HTTP_SEARCH,
    HTTP_UNBIND,
    HTTP_UNCHECKOUT,
    HTTP_UNLINK,
    HTTP_UNLOCK,
    HTTP_UPDATE,
    HTTP_UPDATEREDIRECTREF,
    HTTP_VERSION_CONTROL,
    HTTP_METHOD_QTY
}http_method_t;

typedef enum{
    ROUTER_DEL_SINGLE = 0x00,
    ROUTER_DEL_RECURSIVE = 0x01,
}router_uri_delete_option_t;

/* ----------------------------------------- Typedef's ---------------------------------------- */

typedef struct router_uri_t router_uri_t;

typedef struct http_options_t http_options_t;

typedef struct MHD_Response * (*route_handler_t) (http_method_t method, http_options_t *options, const char *body, size_t body_size, char *filename_auto, void *data);

typedef void (*router_init) (void);

/* ----------------------------------------- Structs ------------------------------------------ */

struct http_options_t{
    char *name;
    char *value;
    http_options_t *next;
};

struct router_uri_t{
    char *route;
    router_uri_t **routers;
    size_t routers_qty;
    route_handler_t handler;
    router_uri_t *next;
};

struct http_method_str_t{
    http_method_t enumerator;
    char *method;
};

/* ----------------------------------------- Globals ------------------------------------------ */

const struct http_method_str_t http_method_str_list[HTTP_METHOD_QTY] = {
    http_method_binder_macro(HTTP_CONNECT, "CONNECT"),
    http_method_binder_macro(HTTP_DELETE, "DELETE"),
    http_method_binder_macro(HTTP_GET, "GET"),
    http_method_binder_macro(HTTP_HEAD, "HEAD"),
    http_method_binder_macro(HTTP_OPTIONS, "OPTIONS"),
    http_method_binder_macro(HTTP_POST, "POST"),
    http_method_binder_macro(HTTP_PUT, "PUT"),
    http_method_binder_macro(HTTP_TRACE, "TRACE"),
    http_method_binder_macro(HTTP_ACL, "ACL"),
    http_method_binder_macro(HTTP_BASELINE_CONTROL, "BASELINE-CONTROL"),
    http_method_binder_macro(HTTP_BIND, "BIND"),
    http_method_binder_macro(HTTP_CHECKIN, "CHECKIN"),
    http_method_binder_macro(HTTP_CHECKOUT, "CHECKOUT"),
    http_method_binder_macro(HTTP_COPY, "COPY"),
    http_method_binder_macro(HTTP_LABEL, "LABEL"),
    http_method_binder_macro(HTTP_LINK, "LINK"),
    http_method_binder_macro(HTTP_LOCK, "LOCK"),
    http_method_binder_macro(HTTP_MERGE, "MERGE"),
    http_method_binder_macro(HTTP_MKACTIVITY, "MKACTIVITY"),
    http_method_binder_macro(HTTP_MKCALENDAR, "MKCALENDAR"),
    http_method_binder_macro(HTTP_MKCOL, "MKCOL"),
    http_method_binder_macro(HTTP_MKREDIRECTREF, "MKREDIRECTREF"),
    http_method_binder_macro(HTTP_MKWORKSPACE, "MKWORKSPACE"),
    http_method_binder_macro(HTTP_MOVE, "MOVE"),
    http_method_binder_macro(HTTP_ORDERPATCH, "ORDERPATCH"),
    http_method_binder_macro(HTTP_PATCH, "PATCH"),
    http_method_binder_macro(HTTP_PRI, "PRI"),
    http_method_binder_macro(HTTP_PROPFIND, "PROPFIND"),
    http_method_binder_macro(HTTP_PROPPATCH, "PROPPATCH"),
    http_method_binder_macro(HTTP_REBIND, "REBIND"),
    http_method_binder_macro(HTTP_REPORT, "REPORT"),
    http_method_binder_macro(HTTP_SEARCH, "SEARCH"),
    http_method_binder_macro(HTTP_UNBIND, "UNBIND"),
    http_method_binder_macro(HTTP_UNCHECKOUT, "UNCHECKOUT"),
    http_method_binder_macro(HTTP_UNLINK, "UNLINK"),
    http_method_binder_macro(HTTP_UNLOCK, "UNLOCK"),
    http_method_binder_macro(HTTP_UPDATE, "UPDATE"),
    http_method_binder_macro(HTTP_UPDATEREDIRECTREF, "UPDATEREDIRECTREF"),
    http_method_binder_macro(HTTP_VERSION_CONTROL, "VERSION-CONTROL")
};

/* ----------------------------------------- Functions ---------------------------------------- */

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
    if(name == NULL) { free(uri_copy); return NULL;}

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
        cursor->name = (char *)calloc(name_len, sizeof(*(cursor->name)));
        strncpy(cursor->name, name, name_len);

        name = strpbrk(value, "&");                                                 // find pointer starting string
        if(name != NULL){
            *name = '\0';
            name++;

            value_len = strlen(value) + 1;
            cursor->value = (char *)calloc(value_len, sizeof(*(cursor->value)));
            strncpy(cursor->value, value, value_len);
        }
        else{
            value_len = strlen(value) + 1;
            cursor->value = (char *)calloc(value_len, sizeof(*(cursor->value)));
            strncpy(cursor->value, value, value_len);

            break;
        }

        last_node = cursor;
    }

    free(uri_copy);
    return first_node;
}

void http_delete_options(http_options_t *options){
    http_options_t *next;
    while(options != NULL){
        free(options->name);
        free(options->value);

        next = options->next;

        free(options);
        options = next;
    }
}

// get an option
http_options_t *http_get_option(const char *name, http_options_t *options){
    while(options != NULL){
        if(!strcmp(options->name, name)) return options;
        options = options->next;
    }
    return NULL;
} 

size_t hash_data(char *name, size_t size){
    size_t acc = 0;

    for(size_t i = 0; i < strlen(name); i++)
        acc += name[i] + i;

    acc %= size;

    return acc;
}

http_method_t http_method_str_to_enum(char *method){
    for(int i = 0; i < HTTP_METHOD_QTY; i++){
        if(!strcmp(method, http_method_str_list[i].method))
            return http_method_str_list[i].enumerator;
    }

    return (http_method_t)(-1);
}

// ------------------------- Router functions ---------------

router_uri_t *router_uri_new(const char *route, size_t sub_routes_qty, route_handler_t handler){
    router_uri_t *router = (router_uri_t*)calloc(1, sizeof(*router));

    router->handler = handler;
    router->next = NULL;

    router->routers_qty = sub_routes_qty;
    router->routers = (router_uri_t **)calloc(sub_routes_qty, sizeof(**(router->routers)));
    
    size_t route_len = strlen(route) + 1;
    router->route = (char *)calloc(route_len, sizeof(*router->route));
    strncpy(router->route, route, route_len);

    return router;
}

void router_uri_use(router_uri_t *router, router_uri_t **child_router, router_init init_function){
    if(router == NULL) return;
    
    init_function();

    size_t index = hash_data((*child_router)->route, router->routers_qty);

    router_uri_t *cursor; 

    if(router->routers[index] == NULL){
        router->routers[index] = *child_router;
    }
    else{
        cursor = router->routers[index];

        while(cursor->next != NULL)
            cursor = cursor->next;

        cursor->next = *child_router;
    }
}

router_uri_t *router_uri_get(router_uri_t *router, char *route){
    size_t index = hash_data(route, router->routers_qty);
    router_uri_t *cursor;

    cursor = router->routers[index];
    while(cursor != NULL){
        if(!strcmp(cursor->route, route)) return cursor;
        cursor = cursor->next;
    }

    return NULL;
}

struct MHD_Response *router_uri_route_request_recursive(router_uri_t *router, char *uri, http_method_t method, http_options_t *options, const char *upload_data, size_t upload_data_size, void *data){
    if(router == NULL) return NULL;
    
    char *next_uri = strpbrk(uri + 1, "/");
    char *point;
    int end = 0;

    if(next_uri == NULL || *(next_uri + 1) == '\0'){                                // end of uri
        if(next_uri != NULL) *next_uri = '\0';                                      // erase last '/' if it is there
        point = strpbrk(uri, "."); 
        end = 1;
    }
    else{
        *next_uri = '\0';
        next_uri++;
    }


    if(!strcmp(uri, "/")){
        return router->handler(                                                     // if it the root route
            method, options, upload_data, upload_data_size, NULL, data
        );
    }
    else{
        router_uri_t *next_router = router_uri_get(router, uri);

        if(next_router == NULL && point != NULL){                                   // not this route, but the handle is called against the auto file
            // ++uri to jump over the '/' at the beggining of the file
            return router->handler(method, options, upload_data, upload_data_size, ++uri, data); 
        }
        else if(next_router == NULL){
            return NULL;
        }
        else if(end){
            return router_uri_route_request_recursive(next_router, "/", method, options, upload_data, upload_data_size, data); 
        }
        else{
            next_uri--;
            *next_uri = '/';
            return router_uri_route_request_recursive(next_router, next_uri, method, options, upload_data, upload_data_size, data); 
        }
    }

}

struct MHD_Response *router_uri_route_request(router_uri_t *router, const char *url, const char *method, http_options_t *options, const char *upload_data, size_t upload_data_size, void *data){
    if(router == NULL) return NULL;
    
    size_t url_len = strlen(url) + 1;
    char *url_copy = (char *)calloc(url_len, sizeof(*url_copy));
    strncpy(url_copy, url, url_len);

    http_method_t method_enum = http_method_str_to_enum((char *)method);

    struct MHD_Response *response = router_uri_route_request_recursive(router, url_copy, method_enum, options, upload_data, upload_data_size, data);
    
    free(url_copy);
    
    return response;
}

#endif