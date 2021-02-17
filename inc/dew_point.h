#ifndef _DEW_POINT_HEADER_
#define _DEW_POINT_HEADER_

#include <math.h>

const double dew_a = 17.62;
const double dew_b = 243.12;

double dew_point(double temperature_cel, double humidity_decimal){
    double term = (dew_a * temperature_cel)/(dew_b + temperature_cel) + log(humidity_decimal);
    return (dew_b * term)/(dew_a - term);
}

#endif