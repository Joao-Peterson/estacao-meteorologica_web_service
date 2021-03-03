#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
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
#include "log.h"
#include "configuration.h"
#include <windows.h>

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

#define DAY_SEC (24*60*60)

/* ----------------------------------------- Types -------------------------------------------- */

typedef struct{
    char *stream;
    size_t len;
}json_stream_t;

/* ----------------------------------------- Globals ------------------------------------------ */

cmdf_option options[] = 
{
    {"config", 'c', 0, 1, "Configuration file. json."},
    {"log",    'l', OPTION_OPTIONAL, 1, "Sets default log level."},
    {"debug",  'd', OPTION_OPTIONAL, 0, "Enables debug log output."},
    {0}
};

/* ----------------------------------------- Prototypes --------------------------------------- */

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data);

int parse_options(char key, char *arg, int arg_pos, void *extern_user_variables_struct);

size_t tm_to_sec(struct tm time_struct);

/* ----------------------------------------- Main --------------------------------------------- */

int main(int argc, char **argv){

    set_cmd_colors();
    log_out_set(stdout);

    char *sprite = get_win_resource_binary_data("title");
    log_colored(COLOR_RED_LOG, "\n\n\n%s\n\n\n", sprite);
    free(sprite);

    // ------------------------- cmd_friend --------------------

    set_cmdf_default_info_usage("Usage: [-s, -p] [SERVER_PORT] [--mysql, --db] [MYSQL_CREDENTIALS (User:password@host:port)]");
    set_cmdf_default_info_version("v1.0 - 22/02/2021");
    set_cmdf_default_info_contact_info("Repo: https://github.com/Joao-Peterson/weather_station_web_service - Email: joco_zx@hotmail.com");

    cdmf_parse_options(options, parse_options, argc, argv, PARSER_FLAG_USE_PREDEFINED_OPTIONS | PARSER_FLAG_PRINT_ERRORS_STDOUT | PARSER_FLAG_DONT_IGNORE_NON_REGISTERED_OPTIONS, (void *)&configuration);

    // ------------------------- Mysql -------------------------

    MYSQL *db_weather_station;
    
    db_weather_station = mysql_init(NULL);

    doc *user = doc_get(configuration, "mysql.user");
    doc *password = doc_get(configuration, "mysql.password");
    doc *host = doc_get(configuration, "mysql.host");
    doc *port = doc_get(configuration, "mysql.port");
    doc *schema = doc_get(configuration, "mysql.schema");

    if(user->type != dt_string || password->type != dt_string || host->type != dt_string || port->type != dt_int32 || schema->type != dt_string){
        log_error("[MySQL] mysql data is wrong on specified configuration json file.\n");
        exit(-1);
    }
    else if(user == NULL || password == NULL || host == NULL || port == NULL || schema == NULL){
        log_error("[MySQL] mysql data is missing on specified configuration json file.\n");
        exit(-1);
    }

    if( mysql_real_connect(db_weather_station, doc_get_string(host), doc_get_string(user), doc_get_string(password), doc_get_string(schema), (unsigned int)doc_get_value(port, int32_t), NULL, 0) ){
        log_info("[MySQL] Connected to DB.\n");
    }
    else{
        log_error("[MySQL] Error: %s.\n", mysql_error(db_weather_station));
        return -1;
    }

    // ------------------------- Micro Http --------------------

    struct MHD_Daemon *server_http;

    router_root_init();
    router_default_init();

    my_custom_struct_t *mystruct = (my_custom_struct_t *)calloc(1, sizeof(*mystruct));
    mystruct->magic_num = MY_CUSTOM_STRUCT_MAGIC_NUM;
    mystruct->db = db_weather_station;
    
    doc *server_port = doc_get(configuration, "server.port");
    if(server_port == NULL){
        log_error("[Server] server port is missing on the configuration json file.\n");
        exit(-1);
    }
    else if(server_port->type != dt_int32){
        log_error("[Server] server port is wrong on the configuration json file.\n");
        exit(-1);
    }

    server_http = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        (int)doc_get_value(server_port, int32_t),
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
        log_error("HTTP server daemon initialization failed.\n");
        return -1;
    }
    else{
        log_info("HTTP server daemon initialization succeded.\n");
    }

    // ------------------------------ CURL ---------------------

    // times
    time_t raw_time;
    raw_time = time(NULL);
    struct tm *cur_date = localtime(&raw_time);
    
    doc *poll = doc_get(configuration, "station.polling_time");
    if(poll == NULL){
        log_error("[Station] station polling time is missing on the configuration json file.\n");
        exit(-1);
    }
    else if(poll->type != dt_int32){
        log_error("[station] station polling time is wrong on the configuration json file.\n");
        exit(-1);
    }
    doc *url = doc_get(configuration, "station.url");
    if(url == NULL){
        log_error("[Station] station url is missing on the configuration json file.\n");
        exit(-1);
    }
    else if(url->type != dt_string){
        log_error("[station] station url is wrong on the configuration json file.\n");
        exit(-1);
    }

    int time_index = tm_to_sec(*cur_date) / doc_get_value(poll, int32_t);

    log_debug("\n");

    CURL *curl;
    CURLcode curlcode;
    json_stream_t json_stream = { .len = 1, .stream = calloc(1,1)};

    doc *mydoc;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    if(curl){

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&json_stream);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback);

        curl_easy_setopt(curl, CURLOPT_URL, doc_get_string(url));
        log_debug("\n");

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-bot/1.0");
        // curl_easy_setopt(curl, CURLOPT_USERNAME, "XXX");
        // curl_easy_setopt(curl, CURLOPT_PASSWORD, "XXX");
        // curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, char *error_msg_buffer);

        while(1){

            if(tm_to_sec(*cur_date) >= (time_index + 1)*doc_get_value(poll, int32_t)){

                log_debug("\n");
                log_client("\n\n*[Client] --------------------------\n");

                log_client("[Weather station GET]: \n- Time: %s- URL: %s\n", asctime(cur_date), doc_get_string(url));
                log_debug("\n");
                curlcode = curl_easy_perform(curl);

                long http_resp_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_resp_code);

                if(curlcode != CURLE_OK){
                    log_error("[Curl error]: %s\n", curl_easy_strerror(curlcode));
                }
                else if(http_resp_code > 400){
                    log_error("[HTTP error code]: %i\n", http_resp_code);
                }
                else{
                    log_client("[Station Data]:\n%s\n", json_stream.stream);

                    doc *doc_weather_station = doc_parse_json(json_stream.stream);
                    
                    /* Insert query */
                    doc_sql_insert_query(db_weather_station, doc_weather_station);

                    free(json_stream.stream);
                    json_stream.stream = NULL;
                    json_stream.len = 1;
                    doc_delete(doc_weather_station, ".");

                }
                
                log_client("-----------------------------------\n\n");

                if(time_index*doc_get_value(poll, int32_t) >= DAY_SEC){
                    time_index = -1;
                }
                else{
                    time_index++;
                }

                log_debug("\n");
            }

            time(&raw_time);
            cur_date = localtime(&raw_time);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    doc_delete(configuration, ".");
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

int parse_options(char key, char *arg, int arg_pos, void *user_data){
    
    doc **configuration = (doc**)user_data; // retrieving custom struct by casting
    FILE *configuration_file;
    char *configuration_stream;

    switch (key){

        case 'c':
            configuration_file = fopen(arg, "r+b");
            configuration_stream = fload_into_mem(configuration_file, NULL);
            *configuration = doc_parse_json(configuration_stream);

            free(configuration_stream);
            fclose(configuration_file);
        break;

        case 'l':
            set_log_level(atoi(arg));
        break;

        case 'd':
            set_debug_level(1);
        break;

        default: 
        break;
    }

    return 0;
}

size_t tm_to_sec(struct tm time_struct){
    return time_struct.tm_hour*60*60 + time_struct.tm_min*60 + time_struct.tm_sec;
}
