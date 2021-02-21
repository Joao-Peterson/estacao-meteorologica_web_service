insert into station_data (temp, humidity, incidency_sun, precipitation)
values ( 12.3, 75.9, 23.5, 25.0);

delete from station_data where id = 1;

select * from station_data;

select temp, humidity, incidency_sun, precipitation, req_date, req_time
from station_data where
    req_date = current_date()
;