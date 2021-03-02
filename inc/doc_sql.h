#ifndef _DOC_SQL_HEADER_
#define _DOC_SQL_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include "mysql.h"
#include "fload_into_mem.h"
#include "doc.h"
#include "doc_json.h"
#include "log.h"
#include "dew_point.h"
#include "heat_index.h"

/**
 * @brief makes an type association between the 'mysql' types and 'doc' types
 */
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

/**
 * @brief insert data from 'doc' into 'mysql'
 */
void doc_sql_insert_query(MYSQL *mysql_db, doc *data){
    int mysql_ret_code = 0; 

    FILE *insert_query_file = fopen("./sql/insert_query_c.sql", "r+b");
    char *insert_query = fload_into_mem(insert_query_file, NULL);
    char *query_buffer = calloc(1000, sizeof(*query_buffer));

    doc *temp_ptr           = doc_get(data, "temp");
    doc *humidity_ptr       = doc_get(data, "humidity");
    doc *incidency_sun_ptr  = doc_get(data, "incidency_sun");
    doc *precipitation_ptr  = doc_get(data, "precipitation");

    double temp                 = 0.0;
    double humidity             = 0.0;
    double incidency_sun        = 0.0;
    double precipitation        = 0.0;
    double heat_index_value     = 0.0;
    double dew_point_value      = 0.0;

    if(temp_ptr->type == dt_double)
        temp = doc_get_value(temp_ptr, double);                 
    else if(temp_ptr->type == dt_int32)
        temp = (double)doc_get_value(temp_ptr, int32_t);                 

    if(humidity_ptr->type == dt_double)
        humidity = doc_get_value(humidity_ptr, double);                     
    else if(humidity_ptr->type == dt_int32)
        humidity = (double)doc_get_value(humidity_ptr, int32_t);                     

    if(incidency_sun_ptr->type == dt_double)
        incidency_sun = doc_get_value(incidency_sun_ptr, double);                          
    else if(incidency_sun_ptr->type == dt_int32)
        incidency_sun = (double)doc_get_value(incidency_sun_ptr, int32_t);                          

    if(precipitation_ptr->type == dt_double)
        precipitation = doc_get_value(precipitation_ptr, double);                          
    else if(precipitation_ptr->type == dt_int32)
        precipitation = (double)doc_get_value(precipitation_ptr, int32_t);                          

    if(temp != 0.0 && humidity != 0.0){
        heat_index_value = heat_index(1.8 * temp + 32.0, humidity);     
        dew_point_value  = dew_point (temp, humidity/100.0);     
    }

    snprintf(query_buffer, 1000, insert_query, 
        temp,
        humidity,
        incidency_sun,
        precipitation,
        heat_index_value,
        dew_point_value
    );
    
    mysql_ret_code = mysql_query(mysql_db, query_buffer);

    free(insert_query);
    free(query_buffer);

    if(mysql_ret_code)
        log_error("Query error: %s.\n", mysql_error(mysql_db));
    else
        log_info("Query complete. Rows altered: [%u].\n", (uint32_t)mysql_affected_rows(mysql_db));
}

/**
 * @brief makes a mysql query and returns a doc with a array inside a obj, this array array contains
 * objects representing the rows, each one has members correponding to the fields. 
*/
doc *doc_sql_select_query(MYSQL *mysql_db, char *sql_query, char *name_array_of_rows){

    typedef struct{
        char *name;
        enum_field_types type;
        char *value;
    }mysql_cell_data_t;

    MYSQL_RES *query_response;
    int mysql_ret_code = 0;

    mysql_ret_code = mysql_query(mysql_db, sql_query);

    if(mysql_ret_code){
        printf("[%s.%u] Query error: %s.\n", __FILE__, __LINE__, mysql_error(mysql_db));
        return NULL;
    }
    else
        printf("[%s.%u] Select Query complete.\n", __FILE__, __LINE__);

    query_response = mysql_store_result(mysql_db);

    if(!query_response){
        printf("[%s.%u] Query error: %s.\n", __FILE__, __LINE__, mysql_error(mysql_db));
        return NULL;
    }

    doc *doc_from_mysql = doc_new(
        "doc_from_mysql", dt_obj, 
            name_array_of_rows, dt_array,
            ";",
        ";"
    );

    size_t query_response_fields_len = mysql_num_fields(query_response);

    MYSQL_ROW query_response_row;
    mysql_cell_data_t cell_data; 
    size_t index = 0;

    char *json_data_row_obj = (char *)calloc(1, sizeof(*json_data_row_obj)*(UINT64_MAX_DECIMAL_CHARS) + strlen(name_array_of_rows) + 1); 
    char *buffer_value[FLOAT_MAX_DECIMAL_CHARS] = {0};

    // puts into doc structure
    // https://dev.mysql.com/doc/c-api/8.0/en/mysql-fetch-row.html
    while((query_response_row = mysql_fetch_row(query_response))){
        unsigned long *query_response_row_lengths = mysql_fetch_lengths(query_response);

        doc_add(doc_from_mysql, (char *)name_array_of_rows, 
            "", dt_obj,
            ";"
        );

        snprintf(json_data_row_obj, UINT64_MAX_DECIMAL_CHARS + strlen(name_array_of_rows), "%s[%u]", name_array_of_rows, (uint32_t)index);

        for(size_t i = 0; i < query_response_fields_len; i++){
            
            // https://dev.mysql.com/doc/c-api/8.0/en/mysql-fetch-field-direct.html
            MYSQL_FIELD *query_response_fields = mysql_fetch_field_direct(query_response, i);

            // field name
            cell_data.name = (char *)calloc(query_response_fields->name_length + 1, sizeof(*(cell_data.name)));
            memcpy(cell_data.name, query_response_fields->name, query_response_fields->name_length);
            
            // type
            cell_data.type = query_response_fields->type;

            // value
            if(query_response_row[i]){
                cell_data.value = (char *)calloc(query_response_row_lengths[i] + 1, sizeof(*(cell_data.name)));
                snprintf(cell_data.value, query_response_row_lengths[i] + 1, "%.*s", (int)query_response_row_lengths[i], query_response_row[i]);
            }
            else{
                cell_data.value = NULL;
            }


            // add to doc
            doc_type_t value_type;

            if(cell_data.value == NULL){
                value_type = dt_null;
            }
            else{
                value_type = type_mysql_to_doc_type(cell_data.type);
            }

            switch(value_type){
                case dt_string:
                case dt_bindata:
                    doc_add(doc_from_mysql, json_data_row_obj,
                        cell_data.name, value_type, cell_data.value, (size_t)query_response_row_lengths[i]
                    );
                break;

                case dt_int64:
                    doc_add(doc_from_mysql, json_data_row_obj,
                        cell_data.name, value_type, strtoll(cell_data.value, NULL, 10)
                    );
                break;

                case dt_double:
                    doc_add(doc_from_mysql, json_data_row_obj,
                        cell_data.name, value_type, strtod(cell_data.value, NULL)
                    );
                break;

                case dt_bool:
                    doc_add(doc_from_mysql, json_data_row_obj,
                        cell_data.name, value_type, ((cell_data.value[0] == 't') ? true : false)
                    );
                break;

                case dt_null:
                    doc_add(doc_from_mysql, json_data_row_obj,
                        cell_data.name, value_type
                    );
                break;
            }             

            free(cell_data.name);
            free(cell_data.value);
        }

        index++;
    }

    return doc_from_mysql;

    free(json_data_row_obj);
    mysql_free_result(query_response);
}

#endif