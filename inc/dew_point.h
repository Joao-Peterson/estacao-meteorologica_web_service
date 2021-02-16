#ifndef _DEW_POINT_HEADER_
#define _DEW_POINT_HEADER_

#include <math.h>

const double dew_a = 17.62;
const double dew_b = 243.12;

double dew_point(double temperature, double humidity_decimal){
    double term = (dew_a * temperature)/(dew_b + temperature) + log(humidity_decimal);
    return (dew_b * term)/(dew_a - term);
}

#endif