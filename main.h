
#ifndef MAIN_HEADER
#define MAIN_HEADER

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable: 4786)//disable warnings about long symbols
#pragma warning(disable: 4244)
#pragma warning(disable: 4018)

//#include <windows.h>
#include <malloc.h>
//#include <mmsystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <cmath>

#include "resource_handler.h"
#include "entities.h"
#include "puzzle.h"
#include "soundmanager.h"
#include "sinecosine.h"
#include "mod_loader.h"
#include "editor.h"
#include "text_output.h"
#include "keys.h"
#include "func.h"

#ifdef _DEBUG

	#include "memleaks.h"
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#define SDL_ASSERT_LEVEL 2

#endif

//#pragma comment(lib,"Strmiids.lib")
//#pragma comment(lib,"dsound.lib")
//#pragma comment(lib,"dxguid")
//#pragma comment(lib,"winmm")
//#pragma comment(lib,"dxerr8.lib")
//#pragma comment(lib,"dinput8.lib")

class game_engine;

class game_engine
{
protected:

	//player's items
	struct item_list_object{
		//bool used;//for unusing the item when dismantled
		int item;//which general item is in this place
		float time_stamp;//when the player got the item
		int amount;//how many of these the player has
		bool wielded;
	};

	//collision_base
	struct collision_base{
		int type;//0=creature, 1=object
		int subtype;
		float x;
		float y;
	};

	//base structure for loading particle info
	struct particle_base{
		bool dead;
		int identifier;

		string name;
		string texture_name;
		int texture;
		float size;
		float r,g,b,alpha;
		float gets_smaller_by_time;
		int can_be_rotated;
		int stop_when_hit_ground;
		int blend_type;

		list <particle> particles_list[3];
	};

	//collision_base
	struct script_info_base{
		float script_calculated_on;
		float timer;
		bool dead;
	};

	//object bases
	vector <script_info_base> script_info;//at which time the scripts were last calculated
	vector <particle_base> particles;

	//name vector
	struct name_vector{
		vector <string> names;
		vector <string> description;
		vector <string> difficulty_level_descriptions;
		vector <bool> visible;
		vector <int> picture;

	};
	vector <name_vector> race_names;

	//terrain timers
	struct timer_base{
		vector <float> subtype;
	};
	vector <timer_base> terrain_timers;
	vector <timer_base> rain_effect_timers;

	float elapsed,elapsed2,elapsed3[31];//timer for fps
	bool perf_flag;        // Timer Selection Flag
	double time_factor;    // Time Scaling Factor
	long last_time;    // Previous timer value
	float game_speed,temp_speed;
	char temprivi[2000];//temporary char table
	string tempstring;
	map *map_main;
	vector <map*> map_storage;
	float real_camera_x,real_camera_y;
	float camera_x,camera_y;
	bool border_visible;
	float x_multiplier,y_multiplier;
	Editor editor;

	bool arrange_item_list_called;
	float light_level;//controls the light level of the gameplay
    float creature_light_value;//for setting the creatures darker
	float time_from_beginning;
	float daylight[3];
	int random_seed;
	int screen_start_x,screen_start_y,screen_end_x,screen_end_y;//which parts of the map are drawn
	//int creature_think_distance;//how far the creatures can be until they don't think
	//float screenshot_timer;//timer for screenshot taken text
	string screenshot_name;//stores name for screenshot
	bool quit_game;//if the game should exit on next frame
	float creatures_checked_on;//at what time the creatures were last checked
	float creature_visibility_checked_on;
	bool ask_quit;//if the player wants to quit
	float darken_timer;//timer for darkening the map traces
	float shortest_distance_to_alien;
	float game_paused_for;//how long the game will still be paused, for pausing after loading a game
	int load_slot;//from which slot to load
	float mouse_speed;
	bool play_sound,play_music;
	bool can_draw_map;
	//int closest_creatures[10];
	vector<int> AI_thinkers;
	float wind_direction_cos,wind_direction_sin;
	int change_map_to;
	bool transfer_enemies_when_changing_map;
	float change_map_player_x,change_map_player_y;
	int debug_level;
	sinecosine sincos;
	float screen_shake_power;
	float screen_shake_time;
	bool show_journals;
	bool mouse_visible;
	int last_saved_game;
	string last_played_mod;
	string save_game_name;
	int proposed_save_game;
	int creature_nearest_to_the_mouse;
	bool mouse_on_item;
	bool can_view_inventory;


	//for collision detection
	struct jump_point{
		bool operator<(const jump_point a) const{ return distance < a.distance;}
		float x,y;
		float distance;
	};

	//mod
	Mod mod;
	string mod_names[20];
	int mods;
	int selected_mod;
	//string mod_name;//which one for this game

	//menu
	//menu item
	struct menu_item{
		string text;
		string help;
		int effect;
		float effect_parameter;
		float height;
		float text_size;
	};
	//menu level
	struct menu_level{
		menu_item item[25];
		int items;
		//float text_size;
		//float item_height;
		float start_x,end_x;
		float start_y;
	};
	int submenu;
	menu_level menu_system[5];
	bool game_running;
	void create_menu_items(void);

	//animation
	float animation_start_timer;
	int animation_frame_current;
	int animation_playing_number;
	int game_state_after_animation;
	void play_animated_sequence(int number, int game_state_after);


	//text viewing
	string log_text;//stores the journal text
	int current_showing_entry;
	void show_text_window(int entry_number);
	//message recorder

	struct journal_record{
		int record_type;//0=journal, 1=item found
		int record_parameter0;//which day, which item
		int record_parameter1;//which race
	};
	vector <journal_record> journal_records;
	bool record_message(int type, int parameter0);


	//player
	bool in_shade;
	//bool near_fire;
	float body_temperature;
	int game_difficulty_level;
	int weapon_selected_from_item_list;
	int item_list_place;
	int combine_item;
	bool combine_item_from_wield;
	float player_middle_x,player_middle_y;
	//int aliens_killed;//works as a difficulty setting
	float day_timer;
	int day_number;
	float scanner_active;
	//int player_resting;
	bool beam_active;
	Mod::effect beam_type;
	float light_addition[3][4];//[r,g,b][map tiles, items, props, creatures]
	float armor;
	float kill_meter_active;
	int player_area;//which area the player is on
	bool alien_attack;//all aliens attack
	int player_race,proposed_player_race;//0=human, 1=alien, 2=android
	float maximum_carry_weight;
	float carry_weight;
	float player_normal_speed;
	bool map_active;
	int old_player_race;
	int attach_camera_type;
	float attach_camera_time, attach_camera_parameter1, attach_camera_parameter2;
	int player_controlled_creature;
	bool can_drop_items;
	bool show_radar;



	//input
	int override_player_controls;
	bool input_override;
	bool override_mouse_left,override_mouse_left2;//override
	float override_mousex,override_mousey;//override
	bool override_key_escape,override_key_escape2;
	bool override_key_enter,override_key_enter2;
	float override_mouse_wheel;
	float stagger_mouse_time,stagger_mouse_parameter1,stagger_mouse_parameter2;
	struct quick_uses{
		char key;
		int item_type;
	};
	vector <quick_uses> quick_keys;
	bool key_down[100];
	bool key_down2[100];
	bool key_clicked[100];

	int player_control_type;
	bool mouse_left,mouse_left2;
	bool mouse_right,mouse_right2;
	bool mouse_middle,mouse_middle2;
	float mouse_wheel;
	float mousex,mousey;//mouse pointer
	bool key_escape,key_escape2;
	bool key_enter,key_enter2;
	bool key_x,key_x2;
	bool key_i,key_i2;
	bool key_f,key_f2;
	bool key_p,key_p2;
	bool key_f5,key_f52;
	bool key_f9,key_f92;
	bool key_f12,key_f122;
	bool key_j,key_j2;
	bool key_b,key_b2;
	bool key_d,key_d2;
	bool key_e,key_e2;
	MouseState		  mousestate;
	MouseState		  mousestate2;

	//textures
	int line_texture;//texture handle for line texture
	//int particle_texture;//texture for particle effects
	int mouse_texture[5];//textures for mouse
	//int computer_texture;//texture for portable computer
	int mapdot;//texture for map dots
	//int map_texture;//texture for map
	//int fire_texture;
	//int snowflake_texture;
	//int electric_texture;
	int item_view;
	int black_texture;
	//int combine_screen;
	//int detector_display;
	int menu;
	int presents_texture;
	int bar_texture;
	int slider_texture;
	int credits_texture;
	int carry_icon;

	//pop-up
	float pop_up_x,pop_up_y;//location for the computer
	float pop_up_transparency;
	int pop_up_mode;//0=text, 1=map, 3=items
	float pop_up_picked_x,pop_up_picked_y;
	int previous_pop_up_mode;
	int map_size_x,map_size_y;//minimap size in pixels
	bool item_list_arranged;

	//slider
	bool slider_active;
	int slider_x,slider_y;
	float slider_point;
	int slider_type;//0=give items to player, 1=drop items to ground, 2=countdown to combine items
	int slider_item,slider_item2;
	int slider_minimum,slider_maximum;

	//sound
	SoundManager* g_pSoundManager;
	bool sound_initialized;
	float volume_slider[2];//volumes for music and samples
	void sound_init(void);
	void playsound(int sample_number,float volume,float sound_x,float sound_y,float listener_x,float listener_y);//plays a sound
	//int load_sample(string name, int samples);//loads the sample if it's unique

	vector <int> preloaded_sounds;
	int UI_menu_click[3];
	int UI_game_click[3];
	//int footstep[3];
	//int footstep_a[3];
	int pick_up;
	//int use_sound;
	//int cold_sound;
	//int hot_sound;
	int intro_sound;
	//int geiger;
	//int groan[5],groans;
	//int jungle[5],jungles;
	//int fire_sound;
	//int electric[2];

	//music
	bool music_initialized;
	void play_music_file(int song_number, int *do_not_play);
	int last_played_music;
	bool SwapSourceFilter(const char* file);
	bool set_volume(float volume);
	bool GraphInit(void);
	bool HandleGraphEvent(void);

	// DirectShow Graph, Filter & Pins used
//	IGraphBuilder *g_pGraphBuilder;
//	IMediaControl *g_pMediaControl;
//	IMediaSeeking *g_pMediaSeeking;
//	IBaseFilter   *g_pSourceCurrent;
//	IBaseFilter   *g_pSourceNext;
//	TCHAR          g_szCurrentFile[128];
//	IMediaEventEx  *pEvent;

	void render_map(void);//renders game map
	//void render_puzzle(void);
	void render_animation(void);
	void render_credits(void);
	void render_menu(void);
	void create_plot_object(map* map_to_edit, int general_item_class, float x, float y, float angle, float size);//throws in the items to the map
	void create_item(map* map_to_edit, int item_type, int amount, float x, float y, float rotation);//throws in the items to the map
	bool draw_object(map_object *object, int layer,float object_x, float object_y);
	bool draw_item(map_object *object, int layer,float object_x, float object_y);
	//void pop_up(char *text);//shows a pop-up for the player
	void calculate_items(void);//checks the items and objects on the map
	void calculate_mouse_on_creatures(void);
	void draw_pop_up(void);//draws the pop-up window
	void vary_object_sizes(map* map_to_edit);//changes the sizes of objects to reflect the object infos
	//int i_c(int i);//map coordinate checker
	//int j_c(int j);//map coordinate checker
	void fix_coordinates(float *c_x, float *c_y);//map coordinate checker
	//void message(float timer, float fade_time, string message);//puts a message to queue
	void draw_item_view(void);//draws the item view
	void draw_text_view(void);//draws the text view
	bool show_journal(int day, int race);
	void draw_targeting_beam(void);
	void arrange_item_list(bool frame_start);
	//void AI_alien(int target, float creature_x, float creature_y, float target_x, float target_y, float distance, int creature);
	vector<int> AI_list_thinkers(void);
	void AI_find_target(int index,int *enemy_index,float *enemy_distance, int *friend_index,float *friend_distance);
	void AI_find_behavior_model(int creature, int *behavior, float *behavior_parameter0, bool *can_shoot, bool *can_eat);
	void AI_act_behavior_model(int creature, int behavior, float parameter0, int enemy_index, float enemy_distance, float enemy_angle, int friend_index, float friend_distance);
	bool AI_eat_behavior(int creature);
	void AI_fire_weapon(int creature,int enemy_index, float enemy_distance, float enemy_angle);
	void AI_move(int creature);
	float AI_pathfinding(int creature);
	void AI_initiate_behavior_parameters(creature_base* creature);
	void save_game(int slot);
	void load_game(int slot);
	void load_mod(const string& mod_name);
	void new_game(void);
	void read_saves(void);
	void calculate_body_temperature(void);
	void draw_loading_screen(void);
	void handle_map_changed(void);
	bool creature_will_collide(map *new_map, creature_base *creature);
	void change_map(int move, float new_x, float new_y, bool move_enemies);
	void combine_items(int a,int combine_item, const vector<Mod::combines::combine_results_base>& combine_results, bool discard_this, bool discard_that);
	void create_maps(void);
	//bool init_mouse(void);
	//void deinit_mouse(void);
	//void calculate_endings(void);
	void calculate_weather(void);
	void load_mod_names(const string& StartingPath);
	void spawn_creature(int side, int tactic, int tactic2, float x, float y, float angle, int type, map *map);
	void set_edges(void);
	static int arrange_item_list_callback(const void *c, const void *d);
	int delete_item(int number);
	void delete_plot_object(map *edit_map, int number, int i, int j, int k);
	void calculate_weight(void);
	void show_slider(int type, int item, int item2, int x, int y, int point);
	void draw_slider(void);
	void disable_input_override(void);
	bool race_specialty(int find_specialty, Mod::specialty *specialty);
	vector <collision_base> list_collisions(float x1,float y1, float x2, float y2, bool only_visible_area);
	void draw_line_map(int x1, int y1, int width, int height, const map* map_draw);
	bool has_terrain_effect(map *map_to_edit, int terrain_type, int search_effect, Mod::effect *effect);
	bool use_item(int general_item_number,int *item_number_in_list, Mod::effect_base effect, bool unuse, bool output, bool just_asking, bool check_slots);
	bool item_has_effect(int item_type, int search_effect, Mod::effect *effect);
	bool plot_object_has_effect(int object_type, int search_effect, Mod::effect *effect);
	bool item_has_combination(int item_type, int search_combination, Mod::combines *combination);
	bool run_effect(Mod::effect effect, creature_base *creature, int creature_number, float x, float y, float angle, bool undo);
	Key translate_key_KEY(char key);
	int translate_key_int(char key);
	bool check_condition(const Mod::condition& condition, const creature_base *creature, int creature_number, float x, float y, bool show_message);
	void draw_bars(void);
	void count_bars(void);
	//void debug_output(string rivi, int level);
	void calculate_scripts(void);
	void run_script(int script_number, bool check_conditions, bool check_time);
	void set_bar(creature_base *creature, unsigned int bar, float value);
	void carry_light(map *edit_map, creature_base *creature, int light);
	void delete_light(map *edit_map, int light);
	bool run_plot_object(int item);
	bool creature_collision_detection(creature_base *creature, const map_object *object, bool correct_place);
	bool point_will_collide(map *new_map, float x, float y, bool only_ones_that_stop_bullets);
	vector <point2d> line_will_collide(float x1, float y1, float x2, float y2, bool return_on_first, bool avoid_terrain, bool only_ones_that_hinder_visibility, bool only_ones_that_stop_bullets, int check_area, bool check_props, bool check_plot_objects);
	vector <point2d> line_hazardous_terrain(float x1, float y1, float x2, float y2);
	vector <point2d> line_collision_detection(float x1, float y1, float x2, float y2,map_object *object,bool return_on_first);
	bool creature_in_object(const creature_base *creature, const map_object *object);
	bool point_in_object(float x, float y, const map_object *object);
	void start_map_editor(void);
	void initialize_creature_specialties(creature_base *creature, map *map_to_edit, bool reset_bars);
	void initialize_animation_frames(map *map_to_edit);
	void delete_creature(map *edit_map, int creature);
	void find_suggested_camera_position(float *suggested_camera_x, float *suggested_camera_y);
	void create_minimap(map *map_to_edit, int d);
	void draw_map_grid_small(map *map_to_edit, int texture, int texture2);//draws the map grid to map texture
	void calculate_quick_keys(bool only_inventory);

public:

	debugger debug;
	resource_handler resources;
	int font;
	text_output text_manager;
	int screen_width,screen_height,bit_depth,windowed;//screen properties
	int game_state;//which screen is displayed
	bool paused;//if the game is paused right now
	bool focus;//for not drawing while the game is not in focus
	bool input_grabbed;
	bool arrange_alphabetically;
	bool ask_continue_game;//pause after loading
	//LPDIRECTINPUTDEVICE8	  g_pMouse;
	//HINSTANCE hInst;

	struct inventory_base{
		vector <item_list_object> player_items;
		vector <int> slot_used_by;
	};
	vector <inventory_base> inventory;
	int active_inventory;
	vector <bool> seen_item_text;//if the player has already seen the text associated with an object
	vector <bool> seen_plot_object_text;//if the player has already seen the text associated with an object

    game_engine();
    ~game_engine();

	//void print_effect_numbers(FILE *fil);
	//void load_item_info(string filename);//loads item info from file

	void load_particles(const string& filename);
	void load_sounds(const string& filename);

	bool cfg_load(void);
	void initialize_game(void);//initialize game variables
	bool Frame(void);
	void uninitialize_game(void);//uninitialize game
	void draw_map_grid(void);//draws the map grid
	void draw_map_objects(int layer);//draws the objects (trees, houses)
	//int load_texture(char *name);//loads the texture if it's unique
	void draw_map_creatures(int layer);//draws the creatures
	void creature_AI(void);//the computer controlled creatures think
	void creature_actions_loop(void);//calculates the creature actions and animation
	bool creature_actions(int creature, bool visible);//calculates the creature actions and animation
	void player_controls(int control_type);//check and control the player input
	void draw_lights(int layer);//draws light effects (flashlight, explosions
	//void flash_light(void);//handles the flashlight
	void load_setup(const char *filename);
	void save_setup(const char *filename);
	float calculate_flashlight(float c_x,float c_y,float rotation, float t_x, float t_y,float *distance,float *angle);//returns the light value using the flashlight
	bullet shoot(int from_creature, int side,int type, float startx,float starty,float angle);//fire a bullet
	void calculate_bullets(void);//calculates bullet flight
	void draw_bullets(void);//draws bullets
	//void draw_line(float x,float y, float x2, float y2, float width, float fade0, float fade1, float r, float g, float b);
	void calculate_lights(void);//calculates all the map lighting
	void make_particle(int type, int layer, float time, float x,float y,float vx,float vy, float angle=-1);//particle animation starter
	void calculate_particles(void);
	void draw_particles(int layer);//draws particles
	void draw_mouse(int cursor, float hot_spot_x, float hot_spot_y, float r, float g, float b);//draws mouse
	void Screenshot(string& screenshot_name);
	void GetScreenshotFileName(string& FileName);
	//void kirjain(char kirjain, int *nume, int *kirjainleveys);
	//int tekstaa(float x, float y, string text,float size);
	bool give_item(int item_number, int amount, float time, bool arrange);

};


#endif
