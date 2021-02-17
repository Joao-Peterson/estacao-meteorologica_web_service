#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "curl/curl.h"
#include "doc.h"
#include "doc_json.h"
#include "doc_sql.h"
#include "mysql.h"
#include "win_res.h"
// #define MHD_PLATFORM_H
// #include <sys/types.h>
// #include <ws2tcpip.h>
#include "microhttpd.h"

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

/* ----------------------------------------- Types -------------------------------------------- */

typedef struct{
    char *stream;
    size_t len;
}json_stream_t;

/* ----------------------------------------- Prototypes --------------------------------------- */

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data);

/* ----------------------------------------- Main --------------------------------------------- */

static enum MHD_Result ahc_echo(void * cls, struct MHD_Connection * connection, const char * url, 
    const char * method, const char * version, const char * upload_data, size_t * upload_data_size, void ** ptr){

    static int dummy;

    const char *index_html = cls;

    if (strcmp(method, "GET"))
        return MHD_NO;                                                              // unexpected method
    
    if (&dummy != *ptr){                                                            // The first time only the headers are valid, 
        *ptr = &dummy;                                                              // do not respond in the first round...
        return MHD_YES;
    }

    if (*upload_data_size != 0)                                                     // upload data in a GET!?
        return MHD_NO; 

    *ptr = NULL;                                                                    // clear context pointer

    printf("Responding HTML.\n");

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(index_html), (void *)index_html, MHD_RESPMEM_MUST_COPY);

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

    MHD_destroy_response(response);

    return ret;
}

int main(int argc, char **argv){

    // ------------------------- Micro Http --------------------

    struct MHD_Daemon *server_http;

    void **data_to_server_callback;

    char *index_html = get_win_resource_binary_data("index_html");

    *data_to_server_callback = index_html;

    server_http = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        5505,
        NULL,
        NULL,
        ahc_echo,
        *data_to_server_callback,
        MHD_OPTION_END
    );

    if(server_http == NULL){
        free(index_html);
        return -1;
    }
    else{
        (void)getc(stdin);
    }

    free(index_html);


    // ------------------------- Mysql -------------------------

    // MYSQL *db_weather_station;
    
    // db_weather_station = mysql_init(NULL);

    // if( mysql_real_connect(db_weather_station, "localhost", "Peterson", "root", "global", 3306, NULL, 0) ){
    //     printf("[%s.%u] Connect to DB.\n", __FILE__, __LINE__);
    // }
    // else{
    //     printf("[%s.%u] Error: %s.\n", __FILE__, __LINE__, mysql_error(db_weather_station));
    //     return -1;
    // }

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

    // /* Select query */
    // char *select_query = get_win_resource_binary_data("select_query");
    // doc *doc_sql = doc_sql_select_query(db_weather_station, select_query, "data");

    // FILE *json_file_out = fopen("./mysql_select.json", "w+");
    // char *json_stream = doc_stringify_json(doc_sql);

    // fprintf(json_file_out, "%s", json_stream);
    // fflush(json_file_out);
    // fclose(json_file_out);

    // free(json_stream);
    // free(select_query);
    // doc_delete(doc_sql, ".");
    // mysql_close(db_weather_station);


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