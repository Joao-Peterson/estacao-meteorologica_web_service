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

typedef struct{
    int port;
    char *mysql_credentials;
    char *mysql_schema;
    char *mysql_user;
    char *mysql_password;
    char *mysql_host;
    char *mysql_port_str;
    int mysql_port;
    char *weather_station_url;
    int weather_station_poll_seconds;
}arg_struct_t;

/* ----------------------------------------- Globals ------------------------------------------ */

cmdf_option options[] = 
{
    {"db",        5,      OPTION_NO_CHAR_KEY, 1, "Data base credentials. Eg: \"User:Password@host:port\""},
    {"mysql",     6,      OPTION_NO_CHAR_KEY | OPTION_ALIAS},
    {"schema",    's',    0, 1, "Mysql database schema"},
    {"port",      'p',    0, 1, "server port. Eg. \"5505\""},
    {"station",   'u',    0, 1, "wheater station URL"},
    {"poll_time", 't',    OPTION_OPTIONAL, 1, "time to poll the weather station, in seconds"},
    {0}
};

arg_struct_t arg_struct;

/* ----------------------------------------- Prototypes --------------------------------------- */

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data);

int parse_options(char key, char *arg, int arg_pos, void *extern_user_variables_struct);

size_t tm_to_sec(struct tm time_struct);

/* ----------------------------------------- Main --------------------------------------------- */

int main(int argc, char **argv){

    arg_struct.weather_station_poll_seconds = (15*60);                              // 15 minutes

    set_cmd_colors();
    log_out_set(stdout);

    char *soy = get_win_resource_binary_data("soy");
    log_colored(COLOR_MAGENTA_LOG, "\n\n\n%s\n\n\n", soy);
    free(soy);

    // ------------------------- cmd_friend --------------------

    set_cmdf_default_info_usage("Usage: [-s, -p] [SERVER_PORT] [--mysql, --db] [MYSQL_CREDENTIALS (User:password@host:port)]");
    set_cmdf_default_info_version("v1.0 - 22/02/2021");
    set_cmdf_default_info_contact_info("Repo: https://github.com/Joao-Peterson/weather_station_web_service - Email: joco_zx@hotmail.com");

    cdmf_parse_options(options, parse_options, argc, argv, PARSER_FLAG_USE_PREDEFINED_OPTIONS | PARSER_FLAG_PRINT_ERRORS_STDOUT | PARSER_FLAG_DONT_IGNORE_NON_REGISTERED_OPTIONS, (void *)&arg_struct);

    // ------------------------- Mysql -------------------------

    MYSQL *db_weather_station;
    
    db_weather_station = mysql_init(NULL);

    if( mysql_real_connect(db_weather_station, arg_struct.mysql_host, arg_struct.mysql_user, arg_struct.mysql_password, arg_struct.mysql_schema, arg_struct.mysql_port, NULL, 0) ){
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
    int time_index = tm_to_sec(*cur_date) / arg_struct.weather_station_poll_seconds;

    CURL *curl;
    CURLcode curlcode;
    json_stream_t json_stream = { .len = 1, .stream = calloc(1,1)};

    doc *mydoc;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    if(curl){

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&json_stream);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_memory_callback);

        curl_easy_setopt(curl, CURLOPT_URL, arg_struct.weather_station_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-bot/1.0");
        // curl_easy_setopt(curl, CURLOPT_USERNAME, "XXX");
        // curl_easy_setopt(curl, CURLOPT_PASSWORD, "XXX");
        // curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, char *error_msg_buffer);

        while(1){

            if(tm_to_sec(*cur_date) >= (time_index + 1)*arg_struct.weather_station_poll_seconds){

                log_client("\n\n*[Client] --------------------------\n");

                log_client("[Weather station GET]: \n- Time: %s- URL: %s\n", asctime(cur_date), arg_struct.weather_station_url);
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

                if(time_index*arg_struct.weather_station_poll_seconds >= DAY_SEC){
                    time_index = -1;
                }
                else{
                    time_index++;
                }

            }

            time(&raw_time);
            cur_date = localtime(&raw_time);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    free(arg_struct.mysql_credentials);
    free(arg_struct.mysql_schema);
    free(arg_struct.weather_station_url);
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

int parse_options(char key, char *arg, int arg_pos, void *extern_user_variables_struct){
    
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

        case 't':
            myvariables->weather_station_poll_seconds = atoi(arg);
        break;

        case 's':
            arg_len = strlen(arg) + 1;
            myvariables->mysql_schema = (char *)calloc(arg_len, sizeof(*myvariables->mysql_schema));
            strncpy(myvariables->mysql_schema, arg, arg_len);
        break;

        case 'u':
            arg_len = strlen(arg) + 1;
            myvariables->weather_station_url = (char *)calloc(arg_len, sizeof(*myvariables->weather_station_url));
            strncpy(myvariables->weather_station_url, arg, arg_len);
        break;

        default: 
        break;
    }

    return 0;
}

size_t tm_to_sec(struct tm time_struct){
    return time_struct.tm_hour*60*60 + time_struct.tm_min*60 + time_struct.tm_sec;
}
