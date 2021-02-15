select temp, humidity, incidency_sun, precipitation, req_date, req_time
from station_data where
    req_date = current_date()
;