select temp, heat_index, humidity, dew_point, incidency_sun, precipitation, req_date, req_time
from station_data where
    req_date between "%s" and "%s"
;
