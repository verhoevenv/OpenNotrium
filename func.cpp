//a few fancy functions

#include "func.h"
#include <cstring>

inline float square(float f) { return sqr(f); };

/*
 * randInt
 *
 * returns a random integer in the specified range
 */
int randInt( int low, int high )
{
    int range = high - low;
	if(range==0)return low;
    int num = rand() % range;
    return( num + low );
}

/*
 * randDouble
 *
 * returns a random double in the specified range
 */
float randDouble( float low, float high )
{
    float range = high - low;
	if(range==0)return low;
    float num = range * (float)rand()/(float)RAND_MAX;
    return( num + low );
}

//bool intersect(RECT eka,RECT toka)
//{
//	int tempx1,tempx2,tempy1,tempy2;
//
//	tempx1=eka.right-toka.left;
//	tempx2=toka.right-eka.left;
//	if (((tempx1>0)&&(tempx2>0)) ||
//	((tempx1<0)&&(tempx2<0)) )
//
//	{
//	tempy1=eka.top-toka.bottom;
//	tempy2=toka.top-eka.bottom;
//	if (((tempy1>0)&&(tempy2>0)) ||
//	((tempy1<0)&&(tempy2<0)))
//	return true;
//	}
//
//
//	return false;
//
//}
// Identical to fgets except the string is trucated at the first ';',
// carriage return or line feed.
char *stripped_fgets(char *s, int n, FILE *f)
{
	using namespace std;

	if (!fgets(s,n,f))
        return NULL;


	//start cut
	int i=0;
	int extra_start_characters=0;
	while((s[i]==' ')||(s[i]==9)){
		extra_start_characters++;
		i++;
	}
	if(extra_start_characters>0){
		int length=strlen(s);
		for(int a=0;a<length-extra_start_characters;a++){
			s[a]=s[a+extra_start_characters];
		}
	}


	//end cut
	i=0;
	while (s[i]!=';' && s[i]!=13 && s[i]!=10 && s[i]!=0)
		i++;
	s[i]=0;


	return(s);
}

bool strtobool(const char *rivi){
	int a=atoi(rivi);
	if (a==0)return false;
	else return true;
}


void random_name(char *creature_name){//give a random name

	char konsonantti[20];
	char vokaali[6];
	char iso_konsonantti[20];
	char iso_vokaali[6];

	//kirjaimet
	vokaali[0]='a';
	vokaali[1]='e';
	vokaali[2]='i';
	vokaali[3]='o';
	vokaali[4]='u';
	vokaali[5]='y';

	iso_vokaali[0]='A';
	iso_vokaali[1]='E';
	iso_vokaali[2]='I';
	iso_vokaali[3]='O';
	iso_vokaali[4]='U';
	iso_vokaali[5]='Y';

	konsonantti[0]='b';
	konsonantti[1]='c';
	konsonantti[2]='d';
	konsonantti[3]='f';
	konsonantti[4]='g';
	konsonantti[5]='h';
	konsonantti[6]='j';
	konsonantti[7]='k';
	konsonantti[8]='l';
	konsonantti[9]='m';
	konsonantti[10]='n';
	konsonantti[11]='p';
	konsonantti[12]='q';
	konsonantti[13]='r';
	konsonantti[14]='s';
	konsonantti[15]='t';
	konsonantti[16]='v';
	konsonantti[17]='w';
	konsonantti[18]='x';
	konsonantti[19]='z';

	iso_konsonantti[0]='B';
	iso_konsonantti[1]='C';
	iso_konsonantti[2]='D';
	iso_konsonantti[3]='F';
	iso_konsonantti[4]='G';
	iso_konsonantti[5]='H';
	iso_konsonantti[6]='J';
	iso_konsonantti[7]='K';
	iso_konsonantti[8]='L';
	iso_konsonantti[9]='M';
	iso_konsonantti[10]='N';
	iso_konsonantti[11]='P';
	iso_konsonantti[12]='Q';
	iso_konsonantti[13]='R';
	iso_konsonantti[14]='S';
	iso_konsonantti[15]='T';
	iso_konsonantti[16]='V';
	iso_konsonantti[17]='W';
	iso_konsonantti[18]='X';
	iso_konsonantti[19]='Z';

			int randomi=randInt(0,6);
			if(randomi==0){
				creature_name[0]=iso_konsonantti[randInt(0,20)];
				creature_name[1]=vokaali[randInt(0,6)];
				creature_name[2]=konsonantti[randInt(0,20)];
				creature_name[3]=creature_name[2];
				creature_name[4]=vokaali[randInt(0,6)];
				creature_name[5]=konsonantti[randInt(0,20)];
				creature_name[6]=0;
			}
			if(randomi==1){
				creature_name[0]=iso_konsonantti[randInt(0,20)];
				creature_name[1]=vokaali[randInt(0,6)];
				creature_name[2]=konsonantti[randInt(0,20)];
				creature_name[3]=vokaali[randInt(0,6)];
				creature_name[4]=konsonantti[randInt(0,20)];
				creature_name[5]=0;
			}
			if(randomi==2){
				creature_name[0]=iso_konsonantti[randInt(0,20)];
				creature_name[1]=vokaali[randInt(0,6)];
				creature_name[2]=konsonantti[randInt(0,20)];
				creature_name[3]=vokaali[randInt(0,6)];
				creature_name[4]=konsonantti[randInt(0,20)];
				creature_name[5]='u';
				creature_name[6]='s';
				creature_name[7]=0;
			}
			if(randomi==3){
				creature_name[0]=iso_konsonantti[randInt(0,20)];
				creature_name[1]=vokaali[randInt(0,6)];
				creature_name[2]=konsonantti[randInt(0,20)];
				creature_name[3]=vokaali[randInt(0,6)];
				creature_name[4]=creature_name[3];
				creature_name[5]=konsonantti[randInt(0,20)];
				creature_name[6]=0;
			}
			if(randomi==4){
				creature_name[0]=iso_konsonantti[randInt(0,20)];
				creature_name[1]=vokaali[randInt(0,6)];
				creature_name[2]=creature_name[1];
				creature_name[3]=konsonantti[randInt(0,20)];
				creature_name[4]=vokaali[randInt(0,6)];
				creature_name[5]=0;
			}
			if(randomi==5){
				creature_name[0]=iso_vokaali[randInt(0,6)];
				creature_name[1]=konsonantti[randInt(0,20)];
				creature_name[2]=creature_name[1];
				creature_name[3]=vokaali[randInt(0,6)];
				creature_name[4]=0;
			}

}

void find_texture_coordinates(int slot,float *x0,float *y0,float *x1,float *y1,int slots_per_texture){
	/**x0=(slot % slots_per_texture)/(float)slots_per_texture;
	*y0=(float)(slot/slots_per_texture);
	*x1=*x0+1.0f/slots_per_texture;
	*y1=*y0+1.0f/slots_per_texture;*/

	*x0=((float)slot/(float)slots_per_texture)-(int)(slot/slots_per_texture);
	*y0=((int)(slot/slots_per_texture))/4.0f;
	*x1=*x0+1.0f/slots_per_texture;
	*y1=*y0+1.0f/slots_per_texture;
}

/*char *copy_substring(char *string, int from, int to){
	char *text;

	int b=0;
	for(int a=from;a<to;a++){
		text[b]=string[a];
		b++;
	}

	return text;
}*/


// Converts a floating point number to string
string FloatToText(float n, int nNumberOfDecimalPlaces)
{

	char s[200];

	sprintf(s, "%f", n);

    string str(s);

	//find decimal
	int start_clear=str.find('.');
	if(nNumberOfDecimalPlaces>0)start_clear+=nNumberOfDecimalPlaces+1;

	str.resize(start_clear, '0');

	return str;
}

/*
float minimum(float a, float b){
	if(a<b)return a;
	return b;
}

float maximum(float a, float b){
	if(a>b)return a;
	return b;
}
*/
std::vector <point2d> sphere_line_intersection (
    float x1, float y1 ,
    float x2, float y2 ,
    float x3, float y3 , float r )
{

	// x1,y1,z1  P1 coordinates (point of line)
	// x2,y2,z2  P2 coordinates (point of line)
	// x3,y3,z3, r  P3 coordinates and radius (sphere)
	// x,y,z   intersection coordinates
	//
	// This function returns a pointer array which first index indicates
	// the number of intersection point, followed by coordinate pairs.





	//float x , y , z;
	float a, b, c, mu, i ;
	vector <point2d> intersections;

	a =  square(x2 - x1) + square(y2 - y1);
	b =  2* ( (x2 - x1)*(x1 - x3)
	  + (y2 - y1)*(y1 - y3) ) ;
	c =  square(x3) + square(y3) +
	  square(x1) + square(y1) -
	  2* ( x3*x1 + y3*y1 ) - square(r) ;
	i =   b * b - 4 * a * c ;

	if ( i < 0.0 )
	{
	// no intersection
	return(intersections);
	}


	if ( i == 0.0 )
	{
	// one intersection
	mu = -b/(2*a) ;
	point2d temp_point;
	temp_point.x = x1 + mu*(x2-x1);
	temp_point.y = y1 + mu*(y2-y1);
	//check if the intersect point is within the line
	if((temp_point.x>=min(x1,x2))&&
		(temp_point.y>=min(y1,y2))&&
		(temp_point.x<=max(x1,x2))&&
		(temp_point.y<=max(y1,y2)))
		intersections.push_back(temp_point);
	return(intersections);
	}
	if ( i > 0.0 )
	{
	// two intersections

	// first intersection
	mu = (-b + sqrt( square(b) - 4*a*c )) / (2*a);
	point2d temp_point;
	temp_point.x = x1 + mu*(x2-x1);
	temp_point.y = y1 + mu*(y2-y1);
	// second intersection
	mu = (-b - sqrt(square(b) - 4*a*c )) / (2*a);
	point2d temp_point2;
	temp_point2.x = x1 + mu*(x2-x1);
	temp_point2.y = y1 + mu*(y2-y1);

	//find the one closer to start point
	/*if(square(temp_point.x-x1)+square(temp_point.y-y1)>square(temp_point2.x-x1)+square(temp_point2.y-y1))
		temp_point=temp_point2;*/

	//check if the intersect point is within the line
	if((temp_point.x>=min(x1,x2))&&
		(temp_point.y>=min(y1,y2))&&
		(temp_point.x<=max(x1,x2))&&
		(temp_point.y<=max(y1,y2)))
		intersections.push_back(temp_point);
	if((temp_point2.x>=min(x1,x2))&&
		(temp_point2.y>=min(y1,y2))&&
		(temp_point2.x<=max(x1,x2))&&
		(temp_point2.y<=max(y1,y2)))
		intersections.push_back(temp_point2);

	return(intersections);
	}

	// no intersection
	return(intersections);
}

bool isvowel(char character){
	switch(character){
	case 'a':
		return true;
	case 'e':
		return true;
	case 'i':
		return true;
	case 'o':
		return true;
	case 'u':
		return true;
	case 'y':
		return true;
	case 'ä':
		return true;
	case 'ö':
		return true;
	case 'å':
		return true;
	case 'A':
		return true;
	case 'E':
		return true;
	case 'I':
		return true;
	case 'O':
		return true;
	case 'U':
		return true;
	case 'Y':
		return true;
	case 'Ä':
		return true;
	case 'Ö':
		return true;
	case 'Å':
		return true;
	}
	return false;
}


int lines_intersect(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float *x,float *y,float rounding)
{

	double u,v,delta;
    double t1,t2;


    double xba,yba,xdc,ydc,xca,yca;


    xba=x2-x1;    yba=y2-y1;
    xdc=x4-x3;    ydc=y4-y3;
    xca=x3-x1;    yca=y3-y1;


    delta=xba*ydc-yba*xdc;
    t1=xca*ydc-yca*xdc;
    t2=xca*yba-yca*xba;


    if(delta!=0)
    {
       u=t1/delta;   v=t2/delta;

      /*two segments intersect (including intersect at end points)*/
	   if ( u<=1+rounding && u>=0-rounding && v<=1+rounding && v>=0-rounding ) {
		   //find coordinates
			*x = x1 + u*(x2 - x1);
			*y = y1 + u*(y2 - y1);

		   return 1;
	   }
      else return 0;
    }
	else{
		return 0;
	}

	return 0;

}


void debugger::debug_output(const string& rivi, int level, int type){

	if(debug_state[type]==1){

		if(level==1)debug_level[type]++;

		FILE *fil;
		if(type==0)
			fil = fopen("debug_start.txt","at");
		else if(type==1)
			fil = fopen("debug_frame.txt","at");
		if(fil){
			for(int a=0;a<debug_level[type];a++)fprintf(fil, "  ");
			if(level==1)fprintf(fil, "Starting ");
			if(level==0)fprintf(fil, "Finished ");
			if(level==2)fprintf(fil, "ERROR ");
			fprintf(fil, "%s\n", rivi.c_str());
			fclose(fil);
			if((level==0)||(level==2)) debug_level[type]--;
		}
	}
}


debugger::debugger(){
	debug_state[0]=0;
	debug_state[1]=0;
	debug_level[0]=0;
	debug_level[1]=0;
}
