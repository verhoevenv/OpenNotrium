#include "sinecosine.h"

#include <cmath>

#include "func.h"

sinecosine::sinecosine()
{
    for (int i=0; i<DEGREES_IN_LOOKUP_TABLE; i++){
        sin_lookup[i] = std::sin(static_cast<float>(i)/DEGREES_IN_LOOKUP_TABLE*2.f*pi);
        cos_lookup[i] = std::cos(static_cast<float>(i)/DEGREES_IN_LOOKUP_TABLE*2.f*pi);
    }
}

float sinecosine::table_sin(float value)
{

//return sin(value);
    int angle=value/pi/2*DEGREES_IN_LOOKUP_TABLE;
    angle = angle % DEGREES_IN_LOOKUP_TABLE;
    if(angle<0)angle += DEGREES_IN_LOOKUP_TABLE;

    return sin_lookup[angle];
}

float sinecosine::table_cos(float value)
{
//return cos(value);
    int angle=value/pi/2*DEGREES_IN_LOOKUP_TABLE;
    angle = angle % DEGREES_IN_LOOKUP_TABLE;
    if(angle<0)angle=DEGREES_IN_LOOKUP_TABLE+angle;

    return cos_lookup[angle];
}
