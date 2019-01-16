#ifndef PUZZLE_HEADER
#define PUZZLE_HEADER


#define maximum_angles 10//how many line points


//puzzle
class puzzle
{
protected:





public:


    float right_angles[maximum_angles];
    float angles[maximum_angles];
    float rotate_self[maximum_angles];
    float rotate_with[maximum_angles][maximum_angles];
    bool correct_angle[maximum_angles];
    int circles;

    int item_slot,item_slot2;
    int item_number;
    int combine_amount;
/*
    int sizex,sizey,lines_amount;
    //int ruudukko[50][50];

    float lines[10][maximum_line_points][2];//[line_number][point_number][x or y]
    int line_points[10];*/

    bool create_new(int circles);

};


#endif
