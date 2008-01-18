#include "puzzle.h"
#include "func.h"


bool puzzle::create_new(int circles){


	this->circles=circles;

	//self rotating circles by random
	bool self_rotating=false;
	if(circles>3)
	if(circles<6)	
	{
		if(randInt(0,2)==0){
			self_rotating=true;
			this->circles--;
		}

	}

	for(int a=0;a<circles;a++){
		right_angles[a]=randDouble(0,2*pi);
		angles[a]=0;

		rotate_self[a]=0;
		if(self_rotating){
			if(randInt(0,2)==0)
				rotate_self[a]=randDouble(-1,1);
		}

		for(int b=0;b<circles;b++){
			rotate_with[a][b]=0;
			if(abs(b-a)==0){
				rotate_with[a][b]=1;
			}
			if(abs(b-a)==1){
				if(randInt(0,1)==0){
					rotate_with[a][b]=randDouble(-1.2f,1.2f);					
				}
			}
			if(abs(b-a)==2){
				if(randInt(0,3)==0){
					rotate_with[a][b]=randDouble(-1,1);					
				}
			}
			if(abs(b-a)==3){
				if(randInt(0,5)==0){
					rotate_with[a][b]=randDouble(-1,1);					
				}
			}

		}
	}

	return true;

}