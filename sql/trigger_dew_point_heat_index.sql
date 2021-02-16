delimiter ;;
create trigger extra_values_trigger before insert on station_data for each row
begin
	set new.dew_point = (243.12*((17.62*new.temp)/(243.12+new.temp)+LN(new.humidity)))/(17.62-((17.62*new.temp)/(243.12+new.temp)+LN(new.humidity)));
    set new.heat_index = ;
end;;

-- drop trigger date_time_trigger;