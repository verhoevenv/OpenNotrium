//functions library
#ifndef FUNCTIONS_LIBRARY
#define FUNCTIONS_LIBRARY

#include <cstdio>
//#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>

#include <cstdlib>
#include <ctime>
#include <cassert>
#include <type_traits>

template <typename T>
inline void SAFE_DELETE(T*& p) {
    if (p) {
        delete p;
        p = NULL;
    }
}

template <typename T>
inline void SAFE_DELETE_ARRAY(T*& p) {
    if (p) {
        delete[] p;
        p = NULL;
    }
}

template <typename T>
inline void RELEASE(T*& p) {
    if (p) {
        p->Release();
        p = NULL;
    }
}

template <typename T>
inline T sqr(const T& p) { return p*p; }

class debugger;

const float pi = 3.1415926535897932384626433832795f;

int lines_intersect(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float *x,float *y,float rounding);

int randInt( int low, int high );
float randDouble( float low, float high );
//bool intersect(RECT eka,RECT toka);
char *stripped_fgets(char *s, int n, FILE *f);
bool strtobool(const char* rivi);
void random_name(const char* creature_name);//give the given monk a random name
void find_texture_coordinates(int slot,float *x0,float *y0,float *x1,float *y1,int slots_per_texture);
std::string FloatToText(float n, int nNumberOfDecimalPlaces);
bool isvowel(char character);
/*float minimum(float a, float b);
float maximum(float a, float b);*/

//string tokenizer
template <typename Container>
void stringtok (Container &container, const std::string& in,
           const char * const delimiters = " \t\n")
{
    const std::string::size_type len = in.length();
          std::string::size_type i = 0;

    while ( i < len )
    {
        // eat leading whitespace
        i = in.find_first_not_of (delimiters, i);
        if (i == std::string::npos)
            return;   // nothing left but white space

        // find the end of the token
        std::string::size_type j = in.find_first_of (delimiters, i);

        // push token
        if (j == std::string::npos) {
            container.push_back (in.substr(i));
            return;
        } else
            container.push_back (in.substr(i, j-i));

        // set up for next loop
        i = j + 1;
    }
}

struct point2d{
    float x;
    float y;
};

std::vector <point2d> sphere_line_intersection (
float x1, float y1 ,
float x2, float y2 ,
float x3, float y3 , float r );

namespace Debugger {
	enum class Action { START, END, FAIL_AND_END, LOG };
	enum class Logfile { STARTUP, FRAME };
}

struct EnumClassHash {
    template <typename T>
    std::size_t operator()(T e) const {
        static_assert(std::is_enum<T>::value, "EnumClassHash only applies to enums");
        return static_cast<std::size_t>(e);
    }
};

class debugger
{
    protected:
		std::unordered_map<Debugger::Logfile, int, EnumClassHash> debug_level;
		std::string type2file(Debugger::Logfile type);
    public:
		std::unordered_map<Debugger::Logfile, bool, EnumClassHash> debug_state;
		void debug_output(const std::string& rivi, Debugger::Action level, Debugger::Logfile type);
		void restart_log(Debugger::Logfile type);
        debugger();
};

struct mouse_control_base{
    float mousex,mousey;
    bool mouse_left,mouse_left2;
    bool mouse_right,mouse_right2;
    bool mouse_middle,mouse_middle2;
    float mouse_wheel;
};

#endif
