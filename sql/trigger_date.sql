delimiter ;;
create trigger date_time_trigger before insert on station_data for each row
begin
	set new.req_date = current_date();
    set new.req_time = current_time();
end;;

drop trigger date_time_trigger;