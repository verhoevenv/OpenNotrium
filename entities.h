
#ifndef ENTITIES_HEADER
#define ENTITIES_HEADER

//#include <windows.h>
#include <vector>
#include <list>
using namespace std;

class map;
class map_object;
class creature_base;
class light;
class bullet;
class particle;
class item;
const float grid_size = 128.0f; //how many pixels is each grid square
//#define maximum_lights 100//how many simultaneous lights available
//#define maximum_bullets 100//how many simultaneous bullets available
//#define maximum_particles 100//how many simultaneous particles available
//#define maximum_items 1000//number of items that the map can have
//#define maximum_objects_on_grid 10
#define maximum_bars 100
#define MAXIMUM_WEAPON_EFFECTS 30

const float minimum_distance_from_edge = 0.4f;//how far must items be from map edges


//bullets
class bullet
{
protected:

public:
	bool dead;
	bool dead_on_next;//the bullet will disappear on the next frame
	float x,y;//position now
	float x2,y2;//position was
	float x3,y3;//in between was and now
	float x4,y4;//position in beginning
	float time;//time left until disappears, -1=infinite
	float angle;
	float move_x,move_y;
	float shooter_move_x,shooter_move_y;
	int type;
	float distance;//distance to player
	int side;//0=player, 1=aliens
	int from_creature;//which creature shot this

    bullet();//constructor
    //~light();//destructor
};

//map
class map
{
protected:

	struct grid_point{
		int terrain_type;//terrain type
		//int total_objects;//how many objects are on this grid
		vector <int> objects;//list the objects that are here
		vector <int> items;//list the items that are here
		int total_creatures;//how many objects are on this grid
		float light_rgb[3];//light, r g b
		int creatures[20];//list the creatures that are here
		int current_frame;
		float frame_time;

	};

	struct row{
		grid_point *grid;
	};



public:
	map(int sizex,int sizey, float amount_multiplier, int climate, vector <int> terrain_types, vector <bool> no_random_terrain_types, vector <bool> do_not_place_on_map_edges, vector <bool> terrain_is_hazardous, vector <int> prop_amounts, vector <int> prop_objects, vector <int> alien_types, vector <int> alien_amounts, vector <int> alien_sides,int items_amount);//constructor
    map(int sizex,int sizey, int creatures_amount, int objects_amount, int items_amount, int climate);//constructor for loading
    ~map();//destructor

	int sizex,sizey;//map size
	row *grid;//map tiles
	vector <map_object> object;//map objects
	vector <creature_base> creature;//creatures
	vector <light> lights;//lighting effects
	list <bullet> bullets;//bullets
	vector <item> items;
	int climate_number;//what climate type this map has
	int area_type;
	float wind_speed;
	float wind_direction;
	float wind_direction_target;
	float rain_timer;
	int near_areas[4];//which map is to left,right,up and down
	bool been_here_already;//if the player has been here at least once
	int map_texture,map_texture_2;

	float time0;
	float time1;
	float time2;

	void generate_map(float amount_multiplier, const vector<int>& terrain_types, const vector<bool>& no_random_terrain_types,
                   const vector<bool>& do_not_place_on_map_edges, const vector<bool>& terrain_is_hazardous, const vector<int>& prop_amounts,
                   const vector<int>& prop_objects, const vector<int>& alien_types, const vector<int>& alien_amounts, const vector<int>& alien_sides);//generates the map
	void check_creatures(void);//checks on which squares the creatures are on
	int create_light(float x, float y, int type, float size, float r, float g, float b, float a, float time);//sets in a light
	void initialize_items(void);//initializes the map
	void initialize_objects(void);//initializes the map

};

//objects that are on the map, trees, buildings
class map_object
{
protected:

public:
	float x,y;
	double rotation;
	float size;
	bool dead;//active or not
	bool visible;//visible or not
	int type;//which general object type it is
	float light_level[3];//light r,g and b
	int current_animation_frame;
	float animation_frame_time;

	//swaying
	float sway_power;
	float sway_phase;


    map_object();//constructor
    ~map_object();//destructor

};

//creatures that walk around, including player
class creature_base
{
protected:

public:


	float x,y,x2,y2;
	float vx,vy;//for pushing
	float rotation;//torso rotation
	float rotation_head;
	float rotation_legs;
	float size;
	//bool may_change_area;
	bool dead;//active or not
	int type;//which general creature type it is
	float fire_timer;//if this is below zero, player can fire
	//float health;
	int side;//which side this is
	float light[3];//light level of this creature
	int animation[3];//animation frame of head, torso and feet
	float animation_timer[3];//animation timer for head, torso and feet
	float distance;//distance to player
	int life_time;//dead when reaches target
	int spawn_time;//spawn a creature when reaches target
	bool show_energy;//if the energy meter is to be draw near the creature
	int eat_counter;
	bool killed;
	int carry_light;
	float carry_light_size;
	int carry_light_type;
	int light_attached_to;
	int last_bullet_hit_from_creature_number;//who shot the last bullet that hit this creature
	//bool fired_weapon_in_last_frame;
	int dialog;
	float dialog_timer;
	int carried_creature;
	float vanish_timer;//time to remain hidden
	float enemy_angle;
	bool force_AI;
	int last_weapon_to_hit;
	float last_weapon_to_hit_time;

	//stealth
	int wall_between_creature_and_player;
	float angle_to_player;
	float alpha;

	//AI
	int AI_order;//0=normal, 1=stay put
	//int status;//0=not seen player, 1=seen player, 2=attacking, 3=moving into attack position, 4=killed, 5=evading player

	//float time_in_flashlight;//how long it's been exposed to the flashligh at one time
	//bool last_frame_in_flashlight;
	int touched_enemy;//if it has touched an enemy, should fire on the next opportunity
	//int target;

	float move_to_x,move_to_y;
	//target_data closest_friend;
	//target_data closest_enemy;
	int tactic[2];
	float behavior_parameter0[10];
	float behavior_parameter1[10];
	float anger_level;//0..1

	struct bar_base{
		bool active;
		//int number;
		float value;
		float minimum,maximum;

	};
	bar_base bars[maximum_bars];

	struct weapon_effect{
		int weapon_type;
		int effect_type;
		float time;
		float parameter0;
		float parameter1;
	};
	weapon_effect weapon_effects[MAXIMUM_WEAPON_EFFECTS];
	int weapon_effects_amount;

	float script_timer[30];

	float left,right,up,down;//movement commands
	bool fire;//shoot command
	float backward_forward_speed,turn_speed;//movement
	//bool sneak;//move slower


	void die(void);

    creature_base();//constructor
    ~creature_base();//destructor

};

//light effects, flashlight, explosions
class light
{
protected:

public:
	float x,y;//precise location to draw picture
	float x2,y2;//precise location to illumination
	float size;
	//float angle;
	float rotation;
	bool dead;//active or not
	int type;//which general light type it is
	float transparency;//how intense the light is
	float r,g,b;//colors
	//bool visible;//if the light picture should be drawn
	float time;//time left, -1=infinite
	float pulse;//pulse
	float pulse_phase;//phase

    light();//constructor
    ~light();//destructor

};



//particle
class particle
{
protected:

public:
	float x,y;//precise location to draw picture
	float move_x,move_y;//where to move
	float time,time_start;//time left until disappears
	//bool dead;//active or not
	//int type;//which type it is
	float rotate;
	//int layer;

};

//item
class item: public map_object
{
protected:

public:
	int base_type;//0=plot item, 1=pickable item
	int item_type;//which type of base_type it is
	bool event_used;//if the event has already been used once
	int amount;
	float interval_timer;
	float create_time;
	//float parameter0;
	//float parameter1;
	//float energy_for_battery;//battery charger energy
	float sound_timer;

	item();
};

#endif
