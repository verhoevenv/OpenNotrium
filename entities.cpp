//entities, such as map, creatures, items

#include "entities.h"
#include "func.h"
#include <cstring>

//#include "dxutil.h"



#define sqr(p) ((p)*(p))
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
//#define SAFE_DELETE(p) { if(p) { delete (p);   (p)=NULL; } }

//normal constructor
map::map(int sizex,int sizey, float amount_multiplier, int climate, vector <int> terrain_types, vector <bool> no_random_terrain_types, vector <bool> do_not_place_on_map_edges, vector <bool> terrain_is_hazardous, vector <int> prop_amounts, vector <int> prop_objects, vector <int> alien_types, vector <int> alien_amounts, vector <int> alien_sides, int items_amount)
{

	int i,j;

	//DXUtil_Timer(TIMER_START);

	//store grid size
	map::sizex=sizex;
	map::sizey=sizey;
	map::climate_number=climate;
	area_type=-1;
	been_here_already=false;

	//count needed props
	/*total_objects=0;
	for (i=0; i<prop_amounts.size(); i++){
		total_objects+=(int)(prop_amounts[i]*amount_multiplier);
	}*/

	//initialize grid
	grid=new row[sizex];
	for (i=0; i<sizex; i++){
		grid[i].grid=new grid_point[sizey];
		for (j=0; j<sizey; j++){
			grid[i].grid[j].items.clear();
			grid[i].grid[j].objects.clear();
		}
	}

	//time0=DXUtil_Timer(TIMER_GETELAPSEDTIME);

	//init objects
	//object=new map_object[total_objects];
	object.clear();
	//creature=new creature_base[total_creatures];
	creature.clear();

	//init lights
	//lights=new light[maximum_lights];
	lights.clear();

	//init bullets
	//bullets=new bullet[maximum_bullets];

	//init particles
	//particles=new particle[maximum_particles];

	//init items
	//items=new item[items_amount];
	items.clear();

	//time1=DXUtil_Timer(TIMER_GETELAPSEDTIME);

	generate_map(amount_multiplier,terrain_types,no_random_terrain_types,do_not_place_on_map_edges,terrain_is_hazardous,prop_amounts,prop_objects, alien_types, alien_amounts, alien_sides);

	//time2=DXUtil_Timer(TIMER_GETELAPSEDTIME);

}

//loading constructor
map::map(int sizex,int sizey, int creatures_amount, int objects_amount, int items_amount, int climate)
{

	int i,j;


	//store grid size
	map::sizex=sizex;
	map::sizey=sizey;
	map::climate_number=climate;
	//total_objects=objects_amount;
	area_type=-1;
	been_here_already=false;


	//initialize grid
	grid=new row[sizex];
	for (i=0; i<sizex; i++){
		grid[i].grid=new grid_point[sizey];
		for (j=0; j<sizey; j++){
			grid[i].grid[j].items.clear();
			grid[i].grid[j].objects.clear();
		}
	}

	//init objects
	object.clear();
	//object=new map_object[total_objects];
	//creature=new creature_base[total_creatures];
	creature.clear();

	//init lights
	lights.clear();
	//lights=new light[maximum_lights];

	//init bullets
	//bullets=new bullet[maximum_bullets];

	//init particles
	/*particles=new particle[maximum_particles];
	//zero the particles
	for (i=0; i<maximum_particles; i++){
		particles[i].dead=true;
	}*/

	//init items
	//items=new item[items_amount];
	items.clear();

}

map::~map()
{
	//delete grid
	for (int i=0; i<sizex; i++){
		for (int j=0; j<sizey; j++){
			//SAFE_DELETE_ARRAY(grid[i].grid[j].objects);
			//SAFE_DELETE_ARRAY(grid[i].grid[j].items);
			grid[i].grid[j].items.clear();
			grid[i].grid[j].objects.clear();
			//grid[i].grid[j].creatures.clear();
		}
		SAFE_DELETE_ARRAY(grid[i].grid);
	}
	SAFE_DELETE_ARRAY(grid);

	//delete objects
	//SAFE_DELETE_ARRAY(object);
	object.clear();
	//SAFE_DELETE_ARRAY(creature);
	creature.clear();

	//delete lights
	//SAFE_DELETE_ARRAY(lights);

	//delete bullets
	//SAFE_DELETE_ARRAY(bullets);

	//delete particles
	//SAFE_DELETE_ARRAY(particles);

	//delete items
	items.clear();
	//SAFE_DELETE_ARRAY(items);

}

//find out on which map square is each creature
void map::check_creatures(void){

	int i,j,k,x,y;

	//squares have no creatures by default
	for (i=0; i<sizex; i++){
		for (j=0; j<sizey; j++){
			grid[i].grid[j].total_creatures=0;
		}
	}
	//find out the squares
	for (k=0; k<creature.size(); k++){
		if(creature[k].dead)continue;
		x=(int)(creature[k].x/grid_size);
		y=(int)(creature[k].y/grid_size);
		if(grid[x].grid[y].total_creatures>=20)continue;
		grid[x].grid[y].creatures[grid[x].grid[y].total_creatures]=k;
		grid[x].grid[y].total_creatures++;

			//break;
	}

}


void map::generate_map(float amount_multiplier, const vector<int>& terrain_types,
                       const vector<bool>& no_random_terrain_types, const vector<bool>& do_not_place_on_map_edges,
                       const vector<bool>& terrain_is_hazardous, const vector<int>& prop_amounts,
                       const vector<int>& prop_objects, const vector<int>& alien_types,
                       const vector<int>& alien_amounts, const vector<int>& alien_sides){
	int i,j,k,a,b;

	//special locations
	float player_start_x=randDouble(sizex*minimum_distance_from_edge*grid_size,sizex*grid_size-sizex*grid_size*minimum_distance_from_edge);
	float player_start_y=randDouble(sizey*minimum_distance_from_edge*grid_size,sizey*grid_size-sizey*grid_size*minimum_distance_from_edge);


	//zero the particles
	/*for (i=0; i<maximum_particles; i++){
		particles[i].dead=true;
	}*/

	//zero the bullets
	bullets.clear();
	/*for (i=0; i<maximum_bullets; i++){
		ZeroMemory(&bullets[i],sizeof(bullets[i]));
		bullets[i].dead=true;
	}*/

	//zero the lights
	lights.clear();
	/*for (i=0; i<maximum_lights; i++){
		ZeroMemory(&lights[i],sizeof(lights[i]));
		lights[i].dead=true;
	}*/

	/*//zero the items
	for (i=0; i<total_items; i++){
		ZeroMemory(&items[i],sizeof(items[i]));
		items[i].dead=true;
	}*/

	//object generation algorithm
	/*for (i=0; i<total_objects; i++){
		object[i].dead=false;
	}*/

	//ground generation algorithm
	//all starts with the first texture
	for (i=0; i<sizex; i++){
		for (int j=0; j<sizey; j++){
			if(terrain_types.size()>0)
				grid[i].grid[j].terrain_type=terrain_types[0];
			else
				grid[i].grid[j].terrain_type=2;
			/*
			grid[i].grid[j].light_rgb[0]=0;
			grid[i].grid[j].light_rgb[0]=1;
			grid[i].grid[j].light_rgb[0]=2;
			*/
		}
	}

	//throw in some other surface
	if(terrain_types.size()>0)
	for (k=0; k<sizex*3; k++){//map size tells how many spots there are
		int ground_type=terrain_types[randInt(0,terrain_types.size())];
		int target_x=randInt(0,sizex);
		int target_y=randInt(0,sizey);
		int size=randInt(3,6);
		int start_x=target_x-size;
		int start_y=target_y-size;
		int end_x=target_x+size;
		int end_y=target_y+size;
		if(start_x<0)start_x=0;
		if(start_y<0)start_y=0;
		if(end_x>sizex-1)end_x=sizex-1;
		if(end_y>sizey-1)end_y=sizey-1;


		for (i=start_x; i<end_x; i++){
			for (j=start_y; j<end_y; j++){
				if(sqrt((float)(sqr(i-target_x)+sqr(j-target_y)))<size){
					//do not place this type on edges
					if(do_not_place_on_map_edges[ground_type]){
						if((i<2)||(j<2)||(i>sizex-4)||(j>sizey-4))
							continue;
					}
					grid[i].grid[j].terrain_type=ground_type;
				}
			}
		}

	}

	//throw in props
	//int prop_counter=0;
	for (j=0; j<prop_amounts.size(); j++){
		for (i=0; i<(int)(prop_amounts[j]*amount_multiplier); i++){
			map_object temp_object;
			int place_bad=100;
			while(place_bad>0){

				temp_object.x=randDouble(0,(sizex-2)*(grid_size));
				temp_object.y=randDouble(0,(sizey-2)*(grid_size));

				//see if it's on some hazardous terrain type
				if(terrain_is_hazardous[(int)(temp_object.x/grid_size)*sizey +(int)(temp_object.y/grid_size)]!=0)
					place_bad--;
				else if(no_random_terrain_types[grid[(int)(temp_object.x/grid_size)].grid[(int)(temp_object.y/grid_size)].terrain_type])
					place_bad--;
				else
					place_bad=0;
			}
			temp_object.dead=false;
			temp_object.rotation=randDouble(0,2*pi);
			temp_object.type=prop_objects[j];
			temp_object.size=1;
			temp_object.sway_phase=0;
			temp_object.sway_power=0;

			//prop_counter++;
			object.push_back(temp_object);
		}
	}

	creature_base temp_creature;
	creature.push_back(temp_creature);

	//throw in the creatures
	//float diameter=sqrtf(sqr(sizex*grid_size)+sqr(sizey*grid_size));
	//for (i=0; i<total_creatures; i++){

	int creature_counter=1;//0=player

	for (a=0; a<alien_amounts.size(); a++){
		for (b=0; b<(int)(alien_amounts[a]); b++){
			bool place_not_ok=true;
			while(place_not_ok){

				//pick a random grid point
				int x=randInt(2,sizex-2);
				int y=randInt(2,sizey-2);

				int aaa=(int)(x)*sizey+(int)(y);

				//bool aa=terrain_is_hazardous[aaa];

				if(terrain_is_hazardous[(int)(x)*sizey+(int)(y)])
					continue;
				if(no_random_terrain_types[grid[x].grid[y].terrain_type])
					continue;

				//randomly the creature can be placed here or not
				/*float distance=sqrtf(sqr(x*grid_size-player_start_x)+sqr(y*grid_size-player_start_y));
				//if(distance<4000)continue;
				if(randInt(0,(int)(diameter/distance))!=0)continue;*/

				i=creature_counter;
				creature_counter++;

				creature_base temp_creature;
				creature.push_back(temp_creature);

				memset(&creature[i], 0, sizeof(creature[i]));
				creature[i].dead=false;
				creature[i].rotation=randDouble(0,2*pi);
				creature[i].rotation_head=creature[i].rotation;
				creature[i].rotation_legs=creature[i].rotation;
				creature[i].size=randDouble(0.9f,1.0f);
				creature[i].type=alien_types[a];
				//creature[i].health=randDouble(0.8f,1.2f);
				creature[i].killed=false;
				creature[i].side=alien_sides[a];
				//creature[i].sneak=false;
				creature[i].animation[0]=0;
				creature[i].animation[1]=2;
				creature[i].animation[2]=5;
				//creature[i].may_change_area=true;
				creature[i].weapon_effects_amount=0;
				creature[i].carry_light=-1;
				creature[i].dialog=-1;
				creature[i].fire_timer=0;
				creature[i].last_bullet_hit_from_creature_number=-1;
				creature[i].carried_creature=-1;
				creature[i].vanish_timer=0;
				creature[i].wall_between_creature_and_player=-1;
				creature[i].force_AI=false;
				creature[i].anger_level=0;
				creature[i].touched_enemy=-1;


				creature[i].x=x*grid_size+randDouble(0,grid_size);
				creature[i].y=y*grid_size+randDouble(0,grid_size);
				creature[i].x2=x*grid_size+randDouble(0,grid_size);
				creature[i].y2=y*grid_size+randDouble(0,grid_size);
				creature[i].move_to_x=creature[i].x;
				creature[i].move_to_y=creature[i].y;
				place_not_ok=false;
			}
		}
	}


	//player
	i=0;
	memset(&creature[i], 0, sizeof(creature[i]));
	creature[i].dead=false;
	creature[i].rotation=randDouble(0,2*pi);
	creature[i].rotation_head=creature[i].rotation;
	creature[i].rotation_legs=creature[i].rotation;
	creature[i].size=1;
	creature[i].type=0;
	//creature[i].health=1;
	creature[i].killed=false;
	creature[i].side=0;
	//creature[i].sneak=false;
	//creature[i].may_change_area=true;
	creature[i].animation[0]=0;
	creature[i].animation[1]=2;
	creature[i].animation[2]=5;
	creature[i].x=player_start_x;
	creature[i].y=player_start_y;
	creature[i].x2=creature[i].x;
	creature[i].y2=creature[i].y;
	creature[i].carry_light=-1;
	creature[i].dialog=-1;
	creature[i].fire_timer=0;
	creature[i].last_bullet_hit_from_creature_number=-1;
	creature[i].carried_creature=-1;
	creature[i].vanish_timer=0;
	creature[i].touched_enemy=-1;


}

//initializes the map items
void map::initialize_items(void){
	for (int i=0; i<sizex; i++){
		for (int j=0; j<sizey; j++){
			grid[i].grid[j].items.clear();
			//for (k=0; k<maximum_objects_on_grid; k++){
			//	grid[i].grid[j].objects[k]=
			//}
		}
	}
	int x,y;
	for (unsigned int k=0; k<items.size(); k++){
		if(items[k].dead)continue;
		x=static_cast<int>(items[k].x/grid_size);
		y=static_cast<int>(items[k].y/grid_size);
		//if(grid[x].grid[y].total_objects<maximum_objects_on_grid-1){
			grid[x].grid[y].items.push_back(k);
		//	grid[x].grid[y].total_objects++;
		//}
	}

	/*//find out on which map square is each object
	int items_here;
	int items_list[100];
	for (i=0; i<sizex; i++){
		for (j=0; j<sizey; j++){
			items_here=0;

			for (k=0; k<items.size(); k++){
				if(items[k].dead)continue;
				//if the object is on this grid square
				if((items[k].x>=i*grid_size)
				&&(items[k].x<(i+1)*grid_size)
				&&(items[k].y>=j*grid_size)
				&&(items[k].y<(j+1)*grid_size)){
					items_list[items_here]=k;
					items_here++;
				}
			}
			//found objects
			SAFE_DELETE_ARRAY(grid[i].grid[j].items);
			grid[i].grid[j].items=NULL;
			grid[i].grid[j].total_items=items_here;
			if(items_here>0)
			{
				grid[i].grid[j].items = new int[items_here];
				for (k=0; k<items_here; k++){
					grid[i].grid[j].items[k]=items_list[k];
				}
			}
		}
	}*/
}


//initializes the map objects
void map::initialize_objects(void){
	//find out on which map square is each object
	/*int objects_here;
	int objects[100];
	for (i=0; i<sizex; i++){
		for (j=0; j<sizey; j++){
			objects_here=0;
			for (k=0; k<total_objects; k++){
				if(object[k].dead)continue;
				//if the object is on this grid square
				if((object[k].x>=i*grid_size)
				&&(object[k].x<(i+1)*grid_size)
				&&(object[k].y>=j*grid_size)
				&&(object[k].y<(j+1)*grid_size)){
					objects[objects_here]=k;
					objects_here++;
				}
			}
			//found objects
			SAFE_DELETE_ARRAY(grid[i].grid[j].objects);
			grid[i].grid[j].total_objects=objects_here;
			grid[i].grid[j].objects=NULL;
			if(objects_here>0)
			{
				grid[i].grid[j].objects = new int[objects_here];
				for (k=0; k<objects_here; k++){
					grid[i].grid[j].objects[k]=objects[k];
				}
			}
		}
	}*/

	for (int i=0; i<sizex; i++){
		for (int j=0; j<sizey; j++){
			grid[i].grid[j].objects.clear();
			//for (k=0; k<maximum_objects_on_grid; k++){
			//	grid[i].grid[j].objects[k]=
			//}
		}
	}
	int x,y;
	for (unsigned int k=0; k<object.size(); k++){
		if(object[k].dead)continue;
		x=static_cast<int>(object[k].x/grid_size);
		y=static_cast<int>(object[k].y/grid_size);
		//if(grid[x].grid[y].total_objects<maximum_objects_on_grid-1){
			grid[x].grid[y].objects.push_back(k);
			//grid[x].grid[y].total_objects++;
		//}
	}
	/*		//found objects
			SAFE_DELETE_ARRAY(grid[i].grid[j].objects);
			grid[i].grid[j].total_objects=objects_here;
			grid[i].grid[j].objects=NULL;
			if(objects_here>0)
			{
				grid[i].grid[j].objects = new int[objects_here];
				for (k=0; k<objects_here; k++){
					grid[i].grid[j].objects[k]=objects[k];
				}
			}
		}
	}*/
}


int map::create_light(float x, float y, int type, float size, float r, float g, float b, float a, float time){


	light temp_light;
	memset(&temp_light, 0, sizeof(temp_light));

	temp_light.dead=false;
	temp_light.time=time;
	temp_light.r=r;
	temp_light.g=g;
	temp_light.b=b;
	temp_light.rotation=randDouble(0,2*pi);
	temp_light.size=size;
	temp_light.transparency=a;
	temp_light.type=type;
	temp_light.x=x-size*128*0.5f;
	temp_light.y=y-size*128*0.5f;
	temp_light.x2=x;
	temp_light.y2=y;
	temp_light.pulse_phase=randDouble(0,100);
	temp_light.pulse=1;

	lights.push_back(temp_light);

	return lights.size()-1;
}

void creature_base::die(void){
	animation[0]=-1;
	animation[2]=-1;

	animation[1]=10;
	animation_timer[1]=100;

	killed=true;
	//dead=true;
}


map_object::map_object(){
	x=0;
	y=0;
	rotation=0;
	size=1;
	dead=true;
	type=0;
	visible=true;

}


map_object::~map_object(){
}

creature_base::creature_base(void)
: left(0)
, right(0)
, up(0)
, down(0)
, fire(false)
, backward_forward_speed(0)
, turn_speed(0)
{}

creature_base::~creature_base(){
}


light::light(){
	x=0;
	y=0;
	rotation=0;
	size=1;
	dead=true;
	type=0;
	r=1;
	g=1;
	b=1;
	pulse=1;

}

light::~light(){

}



bullet::bullet(){
	dead=true;
}

item::item():map_object(){
	item_type=0;
	event_used=false;
	interval_timer=0;
	sound_timer=0;

}
