#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "curl/curl.h"
#include "doc.h"
#include "doc_json.h"
#include "mysql.h"
#include "win_res.h"

// https://jsonplaceholder.typicode.com/todos/2
// https://dev.mysql.com/doc/c-api/8.0/en/
// https://curl.se/libcurl/c/

/*
 * TODO:
 * 
 * make a sprintf that makes a internal buffer with right size
 */

const char *json_array_name = "name";

typedef struct{
    char *name;
    enum_field_types type;
    char *value;
}mysql_cell_data_t;

typedef struct{
    char *stream;
    size_t len;
}json_stream_t;

size_t curl_write_memory_callback(void *data, size_t element_size, size_t elements, void *user_data);

char *sprintf_buff(char *format, ...);

doc_type_t type_mysql_to_doc_type(enum_field_types mysql_type);



int main(int argc, char **argv){

    MYSQL *db_weather_station;
    int mysql_ret_code = 0;
    
    db_weather_station = mysql_init(NULL);

    if( mysql_real_connect(db_weather_station, "localhost", "Peterson", "root", "global", 3306, NULL, 0) ){
        printf("Connect to DB.\n");
    }
    else{
        printf("Error: %s.\n", mysql_error(db_weather_station));
        return -1;
    }


    /* Insert code */
    mysql_ret_code = mysql_query(db_weather_station, 
        "insert into station_data (temp, humidity, incidency_sun, precipitation) values ( 30.0, 70.0, 25.0, 5.0);");

    if(mysql_ret_code)
        printf("Query error: %s.\n", mysql_error(db_weather_station));
    else
        printf("Query complete. Rows altered: [%u].\n", (uint32_t)mysql_affected_rows(db_weather_station));


    /* Select code */
    MYSQL_RES *query_response;

    char *select_query = get_win_resource_binary_data("select_query");

    mysql_ret_code = mysql_query(db_weather_station, select_query);

    free(select_query);

    if(mysql_ret_code)
        printf("Query error: %s.", mysql_error(db_weather_station));
    else
        printf("Select Query complete.\n");

    query_response = mysql_store_result(db_weather_station);

    if(!query_response){
        printf("Query error: %s.\n", mysql_error(db_weather_station));
        return -1;
    }

    doc *json_from_mysql = doc_new(
        "json_from_mysql", dt_obj, 
            json_array_name, dt_array,
            ";",
        ";"
    );

    size_t query_response_fields_len = mysql_num_fields(query_response);

    MYSQL_ROW query_response_row;
    mysql_cell_data_t cell_data; 
    size_t index = 0;

    char *row_index_string = calloc(1, sizeof(*row_index_string)*(UINT64_MAX_DECIMAL_CHARS + 1)); 
    char *json_data_row_obj = calloc(1, sizeof(*row_index_string)*(UINT64_MAX_DECIMAL_CHARS) + strlen(json_array_name) + 1); 

    // puts into doc structure
    while((query_response_row = mysql_fetch_row(query_response))){
        unsigned long *query_response_row_lengths = mysql_fetch_lengths(query_response);

        sprintf(row_index_string, "%u", (uint32_t)index);

        doc_add(json_from_mysql, (char *)json_array_name, 
            row_index_string, dt_obj,
            ";"
        );

        for(size_t i = 0; i < query_response_fields_len; i++){
            
            MYSQL_FIELD *query_response_fields = mysql_fetch_field_direct(query_response, i);

            // field name
            cell_data.name = calloc(query_response_fields->name_length + 1, sizeof(*(cell_data.name)));
            memcpy(cell_data.name, query_response_fields->name, query_response_fields->name_length);
            
            // type
            cell_data.type = query_response_fields->type;

            // value
            if(query_response_row[i]){
                cell_data.value = calloc(query_response_row_lengths[i] + 1, sizeof(*(cell_data.name)));
                snprintf(cell_data.value, query_response_row_lengths[i], "%.*s", (int)query_response_row_lengths[i], query_response_row[i]);
            }
            else{
                cell_data.value = NULL;
            }

            // add to doc
            snprintf(json_data_row_obj, UINT64_MAX_DECIMAL_CHARS + strlen(json_array_name), "%s.%s", json_array_name, row_index_string);
            doc_add(json_from_mysql, json_data_row_obj,
                cell_data.name, type_mysql_to_doc_type(cell_data.type), cell_data.value
            );

            free(cell_data.name);
            free(cell_data.value);
        }

        index++;
    }

    FILE *json_file_out = fopen("./mysql_select.json", "w+");
    char *json_stream = doc_stringify_json(json_from_mysql);
    fprintf(json_file_out, "%s", json_stream);

    fflush(json_file_out);
    fclose(json_file_out);

    free(json_stream);
    free(row_index_string);
    free(json_data_row_obj);
    doc_delete(json_from_mysql, ".");
    mysql_free_result(query_response);
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



doc_type_t type_mysql_to_doc_type(enum_field_types mysql_type){
    switch(mysql_type){
        // rational
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
            return dt_double;
        break;
        
        // integer
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_INT24:
            return dt_int64;
        break;
        
        // bool
        case MYSQL_TYPE_BOOL:
            return dt_bool;
        break;
        
        // null
        case MYSQL_TYPE_NULL:
        case MYSQL_TYPE_INVALID:
            return dt_null;
        break;

        // string
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_TIMESTAMP2:
        case MYSQL_TYPE_DATETIME2:
        case MYSQL_TYPE_TIME2:
        case MYSQL_TYPE_TYPED_ARRAY:
        case MYSQL_TYPE_JSON:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_GEOMETRY:
            return dt_string;
        break;

        // binary data
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
            return dt_bindata;
        break;
    }
}

char *sprintf_buff(char *format, ...){
    va_list list;
    va_start(list, format);

    char *buffer = malloc(sizeof(*buffer)*1048576); // 1 MB

    vsnprintf(buffer, 1048576, format, list);

    char *string = calloc(strlen(buffer) + 1, sizeof(*string));

    memcpy(string, buffer, strlen(buffer) + 1);

    free(buffer);
    va_end(list);

    return string;
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