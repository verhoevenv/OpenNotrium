#ifndef __SINECOSINE_H__
#define __SINECOSINE_H__


class sinecosine;

class sinecosine
{
public:
    static const int DEGREES_IN_LOOKUP_TABLE = 360;
protected:
    float sin_lookup[DEGREES_IN_LOOKUP_TABLE];
    float cos_lookup[DEGREES_IN_LOOKUP_TABLE];

public:
    float table_sin(float value);
    float table_cos(float value);

    sinecosine(); //constructor
    ~sinecosine() = default; //destructor

};


#endif
