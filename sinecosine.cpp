#include "sinecosine.h"




sinecosine::sinecosine()
{
	for (int i=0; i<degrees_in_lookup_table; i++){
		sin_lookup[i] = sin((float)i/degrees_in_lookup_table*2*pi);
		cos_lookup[i] = cos((float)i/degrees_in_lookup_table*2*pi);
	
	}


}

float sinecosine::table_sin(float value)
{

//return sin(value);
	int angle=value/pi/2*degrees_in_lookup_table;
	angle = angle % degrees_in_lookup_table;
	if(angle<0)angle=degrees_in_lookup_table+angle;

	return sin_lookup[angle];

}

float sinecosine::table_cos(float value)
{
//return cos(value);
	int angle=value/pi/2*degrees_in_lookup_table;
	angle = angle % degrees_in_lookup_table;
	if(angle<0)angle=degrees_in_lookup_table+angle;

	return cos_lookup[angle];

}