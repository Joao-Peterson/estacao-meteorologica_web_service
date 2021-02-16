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

// https://jsonplaceholder.typicode.com/todos/2
// https://dev.mysql.com/doc/c-api/8.0/en/
// https://curl.se/libcurl/c/

typedef struct{
    char *stream;
    size_t len;
}json_stream_t;

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data);



int main(int argc, char **argv){

    MYSQL *db_weather_station;
    
    db_weather_station = mysql_init(NULL);

    if( mysql_real_connect(db_weather_station, "localhost", "Peterson", "root", "global", 3306, NULL, 0) ){
        printf("Connect to DB.\n");
    }
    else{
        printf("Error: %s.\n", mysql_error(db_weather_station));
        return -1;
    }

    char *select_query = get_win_resource_binary_data("select_query");
    doc *doc_sql = doc_sql_select_query(db_weather_station, select_query, "data");

    FILE *json_file_out = fopen("./mysql_select.json", "w+");
    char *json_stream = doc_stringify_json(doc_sql);

    fprintf(json_file_out, "%s", json_stream);
    fflush(json_file_out);
    fclose(json_file_out);

    free(json_stream);
    free(select_query);
    doc_delete(doc_sql, ".");
    mysql_close(db_weather_station);

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

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data){

    size_t data_size = element_size * elements;

    json_stream_t *json = (json_stream_t *)user_data;

    json->len += data_size;

    json->stream = realloc(json->stream, json->len);

    memcpy(json->stream + (json->len - data_size - 1), data, data_size);

    json->stream[json->len - 1] = '\0';

    return data_size;
}