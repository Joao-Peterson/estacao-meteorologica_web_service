#ifndef _HTTP_CLIENT_UTILS_HEADER_
#define _HTTP_CLIENT_UTILS_HEADER_

#include <time.h>
#include "configuration.h"
#include "log.h"
#include "doc.h"
#include "doc_json.h"
#include "curl/curl.h"
#include "mysql.h"
#include "doc_sql.h"

#define DAY_SEC (24*60*60)

typedef struct{
    char *stream;
    size_t len;
}json_stream_t;


size_t tm_to_sec(struct tm time_struct){
    return time_struct.tm_hour*60*60 + time_struct.tm_min*60 + time_struct.tm_sec;
}

int verify_station_data(doc *data){

    if(data == NULL) return 0;

    doc *temp = doc_get(data, "temp");
    doc *humidity = doc_get(data, "humidity");
    doc *incidency_sun = doc_get(data, "incidency_sun");
    doc *precipitation = doc_get(data, "precipitation");

    if( temp->type          == dt_null ||
        humidity->type      == dt_null ||
        incidency_sun->type == dt_null ||
        precipitation->type == dt_null
    )
        return 0;
    else
        return 1;
} 


void curl_perform_schedule(CURL *curl, struct tm *cur_date, int poll, int retries, int *time_index, char *url, json_stream_t *json_stream, MYSQL *db_weather_station){
    CURLcode curlcode;
    
    size_t sec = tm_to_sec(*cur_date);

    static int retry = 0;

    if(*time_index >= (DAY_SEC / poll) && (sec < poll)){
        log_debug("if - sec: %u, index: %i\n", sec, *time_index);
        sec += DAY_SEC;  
    } 

    if(sec >= (*time_index)*poll || retry > 0){

        if(retry == 0){
            retry = retries - 1;
        }
        else{
            log_client("[RETRY] [%i]", retries - retry);
            retry--;
        }

        log_debug("sec: %u, index: %i\n", sec, *time_index);
        (*time_index)++;

        if(*time_index >= (DAY_SEC / poll) + 1){
            log_debug("%i\n", *time_index);
            *time_index = 1;
            log_debug("%i\n", *time_index);
        }

        log_client("\n*[Client] --------------------------\n");

        log_client("[Weather station GET]: \n- Time: %s- URL: %s\n", asctime(cur_date), url);
        curlcode = curl_easy_perform(curl);

        long http_resp_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_resp_code);

        if(curlcode != CURLE_OK){
            log_error("[HTTP error code]: %i\n", http_resp_code);
            log_error("[Curl error]: %s\n", curl_easy_strerror(curlcode));
        }
        else if(http_resp_code != 200){
            log_error("[HTTP error code]: %i\n", http_resp_code);
        }
        else if(*(json_stream->stream) != '{'){
            log_error("[HTTP error code]: %i\n", http_resp_code);
            log_error("[Body Error]: the body received is not a json file: \n%s\n", json_stream->stream);
        }
        else{
            log_client("[HTTP error code]: %i\n", http_resp_code);
            log_client("[Station Data]:\n%s\n", json_stream->stream);

            doc *doc_weather_station = doc_parse_json(json_stream->stream);

            if(verify_station_data(doc_weather_station)){
                /* Insert query */
                doc_sql_insert_query(db_weather_station, doc_weather_station);
                doc_delete(doc_weather_station, ".");
                retry = 0;
            }

            free(json_stream->stream);
            json_stream->stream = NULL;
            json_stream->len = 1;
        }
        
        log_client("-----------------------------------\n\n");
    }
}

#endif