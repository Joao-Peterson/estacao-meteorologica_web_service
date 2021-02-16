create table station_data (
	id int primary key auto_increment,
    temp double,
	humidity double,
	incidency_sun double,
	precipitation double 
);

alter table station_data add column req_date date;

alter table station_data add column req_time time;