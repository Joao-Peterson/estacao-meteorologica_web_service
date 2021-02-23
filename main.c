#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "curl/curl.h"

#include "mysql.h"

#include "doc.h"
#include "doc_json.h"
#include "doc_sql.h"
#include "cmd_friend.h"

#include "my_custom_struct.h"

#include "win_res.h"

#include "microhttpd.h"
#include "http_server_utils.h"
#include "router_uri.h"
#include "routes/root.router.h"

/**
 * JSONTEST:
 * https://jsonplaceholder.typicode.com/todos/2
 * 
 * DOCUMENTATION:
 * https://dev.mysql.com/doc/c-api/8.0/en/
 * https://curl.se/libcurl/c/
 * https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html
 * https://www.gnu.org/software/libmicrohttpd/tutorial.html
 */

/* ----------------------------------------- Definitions -------------------------------------- */

#define DB_KEY_ 1

/* ----------------------------------------- Types -------------------------------------------- */

typedef struct{
    char *stream;
    size_t len;
}json_stream_t;

typedef struct{
    int port;
    char *mysql_credentials;
    char *mysql_schema;
    char *mysql_user;
    char *mysql_password;
    char *mysql_host;
    char *mysql_port_str;
    int mysql_port;
}arg_struct_t;

/* ----------------------------------------- Globals ------------------------------------------ */

cmdf_option options[] = 
{
    {"db",      5,      OPTION_NO_CHAR_KEY, 1, "Data base credentials. Eg: \"User:Password@host:port\""},
    {"mysql",   6,      OPTION_NO_CHAR_KEY | OPTION_ALIAS},
    {"schema",  's',    0, 1, "Mysql database schema"},
    {"port",    'p',    0, 1, "server port. Eg. \"5505\""},
    {0}
};

// cmdf_option options[] = 
// {
//     {"where",   'w', 0,                                             1, "Where to create the project"},
//     {"file",    'f', OPTION_ALIAS },
//     {"tags",    't', OPTION_OPTIONAL,                              -1, "Tags to put in"},
//     {"verbose", 'v', OPTION_OPTIONAL | OPTION_NO_LONG_KEY,          0, "Verbose mode"},
//     {"Wall",    'W', OPTION_OPTIONAL,                               0, "Wall error mode"},
//     {"vscode",   2,  OPTION_OPTIONAL | OPTION_NO_CHAR_KEY,          0, "Visual studio code .vscode folder with .json configuration files"},
//     {0}
// };

arg_struct_t arg_struct = {.port = 0, .mysql_credentials = NULL};

/* ----------------------------------------- Prototypes --------------------------------------- */

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data);

int parse_options(char key, char *arg, int arg_pos, void *extern_user_variables_struct);

/* ----------------------------------------- Main --------------------------------------------- */

int main(int argc, char **argv){

    // ------------------------- cmd_friend --------------------

    set_cmdf_default_info_usage("Usage: [-s, -p] [SERVER_PORT] [--mysql, --db] [MYSQL_CREDENTIALS (User:password@host:port)]");
    set_cmdf_default_info_version("v1.0 - 22/02/2021");
    set_cmdf_default_info_contact_info("Repo: https://github.com/Joao-Peterson/weather_station_web_service - Email: joco_zx@hotmail.com");

    cdmf_parse_options(options, parse_options, argc, argv, PARSER_FLAG_USE_PREDEFINED_OPTIONS | PARSER_FLAG_PRINT_ERRORS_STDOUT | PARSER_FLAG_DONT_IGNORE_NON_REGISTERED_OPTIONS, (void *)&arg_struct);

    // ------------------------- Mysql -------------------------

    MYSQL *db_weather_station;
    
    db_weather_station = mysql_init(NULL);

    if( mysql_real_connect(db_weather_station, arg_struct.mysql_host, arg_struct.mysql_user, arg_struct.mysql_password, arg_struct.mysql_schema, arg_struct.mysql_port, NULL, 0) ){
        printf("[%s.%u] [MySQL] Connected to DB.\n", __FILE__, __LINE__);
    }
    else{
        printf("[%s.%u] [MySQL] Error: %s.\n", __FILE__, __LINE__, mysql_error(db_weather_station));
        return -1;
    }

    // /* Insert query */
    // doc *insert_doc = doc_new(
    //     "insert_doc", dt_obj,
    //         "temp", dt_double, 30.0,
    //         "humidity", dt_double, 50.0,
    //         "incidency_sun", dt_double, 25.0,
    //         "precipitation", dt_double, 5.0,
    //     ";"
    // );

    // // doc_sql_insert_query(db_weather_station, insert_doc);

    // ------------------------- Micro Http --------------------

    struct MHD_Daemon *server_http;

    router_root_init();
    router_default_init();

    my_custom_struct_t *mystruct = (my_custom_struct_t *)calloc(1, sizeof(*mystruct));
    mystruct->magic_num = MY_CUSTOM_STRUCT_MAGIC_NUM;
    mystruct->db = db_weather_station;
    
    server_http = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        arg_struct.port,
        on_client_connect,
        NULL,
        on_response,
        (void *)mystruct,
        MHD_OPTION_URI_LOG_CALLBACK,
        on_uri_parsing,
        NULL,
        MHD_OPTION_END
    );

    if(server_http == NULL){
        printf("HTTP server daemon initialization failed.\n");
        return -1;
    }
    else{
        printf("HTTP server daemon initialization succeded.\n");
        (void)getc(stdin);
    }


    // ------------------------------ CURL ---------------------

    // CURL *curl;
    // CURLcode curlcode;
    // json_stream_t json_stream = { .len = 1, .stream = calloc(1,1)};

    // doc *mydoc;

    // curl_global_init(CURL_GLOBAL_ALL);

    // curl = curl_easy_init();

    // if(curl){

    //     curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&json_stream);
    //     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback);

    //     curl_easy_setopt(curl, CURLOPT_URL, "https://jsonplaceholder.typicode.com/todos/2");

    //     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    //     curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-bot/1.0");
    //     // curl_easy_setopt(curl, CURLOPT_USERNAME, "XXX");
    //     // curl_easy_setopt(curl, CURLOPT_PASSWORD, "XXX");

    //     // curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, char *error_msg_buffer);


    //     curlcode = curl_easy_perform(curl);

    //     if(curlcode != CURLE_OK)
    //         printf("curk_error: %s", curl_easy_strerror(curlcode));

    //     curl_easy_cleanup(curl);
    // }

    // curl_global_cleanup();

    // printf("[JSON]:\n%s\n", json_stream.stream);

    free(arg_struct.mysql_credentials);
    free(mystruct);
    MHD_stop_daemon(server_http);
    mysql_close(db_weather_station);

    return 0;
}

/* ----------------------------------------- Implementations ---------------------------------- */

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data){

    size_t data_size = element_size * elements;

    json_stream_t *json = (json_stream_t *)user_data;

    json->len += data_size;

    json->stream = realloc(json->stream, json->len);

    memcpy(json->stream + (json->len - data_size - 1), data, data_size);

    json->stream[json->len - 1] = '\0';

    return data_size;
}

int parse_options(char key, char *arg, int arg_pos, void *extern_user_variables_struct)
{
    arg_struct_t *myvariables = (arg_struct_t*)extern_user_variables_struct; // retrieving custom struct by casting
    char *cursor;
    size_t arg_len;

    switch (key)
    {
        case 5: 
        case 6: 
            arg_len = strlen(arg) + 1;
            myvariables->mysql_credentials = (char *)calloc(arg_len, sizeof(*myvariables->mysql_credentials));
            strncpy(myvariables->mysql_credentials, arg, arg_len);

            myvariables->mysql_user = myvariables->mysql_credentials;

            cursor = strpbrk(myvariables->mysql_credentials, ":@");
            *cursor = '\0';
            cursor++;

            myvariables->mysql_password = cursor;

            cursor = strpbrk(cursor, ":@");
            *cursor = '\0';
            cursor++;

            myvariables->mysql_host = cursor;

            cursor = strpbrk(cursor, ":@");
            *cursor = '\0';
            cursor++;

            myvariables->mysql_port_str = cursor;

            myvariables->mysql_port = atoi(myvariables->mysql_port_str);
        break;

        case 'p':
            myvariables->port = atoi(arg);
        break;

        case 's':
            arg_len = strlen(arg) + 1;
            myvariables->mysql_schema = (char *)calloc(arg_len, sizeof(*myvariables->mysql_schema));
            strncpy(myvariables->mysql_schema, arg, arg_len);
        break;

        default: 
        break;
    }

    return 0;
}