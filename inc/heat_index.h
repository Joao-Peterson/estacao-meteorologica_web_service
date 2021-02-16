#ifndef _HEAT_INDEX_HEADER_
#define _HEAT_INDEX_HEADER_

const double c1_hi = -42.379;
const double c2_hi = 2.04901523;
const double c3_hi = 10.14333127;
const double c4_hi = -0.22475541;
const double c5_hi = -6.83783E-3;
const double c6_hi = -5.481717E-2;
const double c7_hi = 1.22874E-3;
const double c8_hi = 8.5282E-4;
const double c9_hi = -1.99E-6;

double heat_index(double temperature, double humidity_decimal){
    return (
        (5.0/9.0)*(
            c1_hi + 
            c2_hi * temperature +
            c3_hi * humidity_decimal +
            c4_hi * temperature * humidity_decimal +
            c5_hi * temperature * temperature +
            c6_hi * humidity_decimal * humidity_decimal +
            c7_hi * temperature * temperature * humidity_decimal +
            c8_hi * humidity_decimal * humidity_decimal * temperature +
            c9_hi * humidity_decimal * humidity_decimal * temperature * temperature -
            32
        )
    );
}

#endif