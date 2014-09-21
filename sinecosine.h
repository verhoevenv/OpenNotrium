#ifndef __SINECOSINE_H__
#define __SINECOSINE_H__

#define degrees_in_lookup_table 360

class sinecosine;


class sinecosine
{
protected:
	float sin_lookup[degrees_in_lookup_table];
	float cos_lookup[degrees_in_lookup_table];

public:
	float table_sin(float value);
	float table_cos(float value);

    sinecosine();//constructor
    //~light();//destructor
};


#endif
