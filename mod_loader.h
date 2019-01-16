#ifndef __MOD_LOADER_H__
#define __MOD_LOADER_H__

#pragma warning(disable: 4786)//disable warnings about long symbols

#include <string>
#include <vector>
#include "resource_handler.h"

class Mod
{
protected:

    char temprivi[5000];//temporary char table

public:

    struct effect{
        int effect_number;
        float parameter1;
        float parameter2;
        float parameter3;
        float parameter4;
    };

    struct condition{
        int condition_number;
        float condition_parameter0;
        float condition_parameter1;
    };

    struct object_animation_frame_base{
        std::string texture_name;
        int texture;
        float time;
    };

    //base structure for loading object info
    struct general_objects_base{
        bool dead;
        std::string name;
        int identifier;
        float vary_size;
        //string texture_name;
        //int texture;
        std::vector <object_animation_frame_base> animation_frames;
        float base_size;
        int transparent;// 0,1 whether it should be transparent or not
        int layer;//0 is below player, 2 is on top of player
        int swing;//trees swing
        int collision_type;
        int get_transparent;
        int provide_shade;
        bool blocks_vision;
        bool stops_bullets;
        float collision_parameter0;
        float collision_parameter1;
        float passable_radius;//how far the passable circle is
    };

    //base structure for loading creature info
    struct general_creatures_base{

        bool dead;

        int identifier;
        std::string name;
        bool close_combat;
        int particle_on_radar;
        int AI_tactics[2];
        int footsteps;
        std::vector <int> footstep_sounds;
        std::vector <std::string> footstep_sound_names;
        int die_after_seconds;
        int spawn_creature;
        int spawn_interval;
        int eat_item;//eat item number from items.dat, -1=nothing
        int eat_event;//eat event -1=nothing, 0=increase health by parameter0, 1=die when eaten amount parameter0, 2=spawn new creature when eaten amount parameter0
        float eat_parameter0;//eat parameter0
        int texture;
        std::string texture_name;
        int corpse_item;
        int corpse_item_amount;
        bool hide_behind_walls;
        float size;
        float weight;
        float movement_speed, minimum_movement_speed, turn_speed;
        float leg_animation_speed;
        //std::vector <int> weapons;
        int weapon;
        //float health;
        float weapon_x,weapon_y;//offset of weapon
        int blood;
        int creature_class;
        float inertia;
        int layer;
        bool bars_visible;
        float death_animation_speed;
        bool can_change_area;
        float AI_hear_range;
        float AI_see_range;
        float AI_see_angle;

        std::vector <int> hit_sound;
        std::vector <std::string> hit_sound_names;
        std::vector <int> die_sound;
        std::vector <std::string> die_sound_names;

        struct effect_block{
            std::vector <effect> effects;
            std::vector <condition> conditions;
        };
        std::vector <effect_block> eat_block;
        std::vector <effect_block> hit_block;

        struct death_effect_block{
            int death_type;
            std::vector <effect> effects;
            std::vector <condition> conditions;
        };
        std::vector <death_effect_block> death_block;

        struct timed_effect_block{
            int interval;
            std::vector <effect> effects;
            std::vector <condition> conditions;
        };
        std::vector <timed_effect_block> timed_block;

        //std::vector <int> immune_weapon_classes;

        //specialties
        struct creature_specialty{
            std::string description;
            int number;
            float parameter0;
            float parameter1;
            float parameter2;
            float parameter3;
        };
        std::vector <creature_specialty> specialties;
    };

    //base structure for loading weapon info
    struct general_weapons_base{
        bool dead;
        std::string name;
        int identifier;
        int weapon_class;
        int texture;
        std::string texture_name;
        int bullets_at_once;
        int stop_on_hit;
        float size;
        float bullet_speed;
        float time;//how long the bullet goes before disappearing, -1=forever
        int fire_rate;//time between two shots
        //float energy_consumption;
        //int use_ammo_from_inventory;
        //float damage;
        //int trace0;
        //int gunblast_particle;
        int trace1;
        float push;//how much to push target back when hit
        float push_shooter;//how much to push shooter back when hit
        int special_effect_visual_color;
        int special_effect_visual_particle;
        int special_effect_sound;
        std::string special_effect_sound_name;
        float spread;
        int sound_fire;
        std::string sound_fire_name;
        int sound_hit;
        std::string sound_hit_name;
        float AI_hear_volume;

        std::vector<condition> wield_conditions;
        std::vector<effect> fire_effects;
        std::vector<effect> hit_effects;
    };


    //for effects
    struct effect_base{

        std::string event_text;
        std::string event_failure_text;
        int vanish_after_used;
        int sound;
        std::string sound_name;

        std::vector <effect> effects;
        std::vector <condition> conditions;
    };

    //base structure for loading ground info
    struct general_climate_base{

        struct rain_effect_base{
            effect_base effect;
            float interval;
        };

        bool dead;
        int identifier;
        bool can_be_random;
        std::vector<int> terrain_types;
        float temperature[2];//0=night temperature, 1=day temperature
        std::vector<int> prop_object_definition_number;
        std::vector<int> prop_amount;
        std::vector<int> night_sounds;
        std::vector<int> day_sounds;
        float maximum_wind_speed;
        std::vector <rain_effect_base> rain_effects;
        int rain_particle;
        float rain_probability;
        float rain_length_min,rain_length_max;
        float rain_particle_life_min,rain_particle_life_max;

        float light_oscillate_time;
        float light_phase_r,light_phase_g,light_phase_b;
        float light_amplitude_r,light_amplitude_g,light_amplitude_b;
        float light_min_r,light_min_g,light_min_b;
        float light_max_r,light_max_g,light_max_b;

        /*int rain_type;
        float rain_parameter1;
        float rain_parameter2;
        int rain_particle_type;
        string rain_warning;*/
    };

    //base structure for loading area info
    struct general_area_special_base{
        int identifier;
        int area_class;
        std::string name;
        bool dead;
        int climate_override;//-1=random climate
        float random_item_density;
        int wrap_type;
        //int sizex,sizey;
        int terrain_map_number;
        std::vector<int> alien_type;
        std::vector<int> alien_amount;
        std::vector<int> alien_sides;
        std::string on_enter_text;
        std::vector<int> exclude_plot_objects;
    };

    //to load item effects
    struct item_effect_base{
        effect_base effect;
        char use_key;
        char quick_key;
        //int use_time;
    };
    //to load item effects
    struct combines{
        int combines_with;
        bool discard_that;
        bool discard_this;
        int combine_puzzle_difficulty;
        int can_be_broken_up;

        struct combine_results_base{
            int combines_to;
            int combines_amount;
        };
        std::vector <combine_results_base> combine_results;
    };

    //base structure for loading item info
    struct general_item_base{

        bool dead;
        int identifier;
        int item_class;
        float weight;
        std::string texture_name;
        std::string name;
        std::string description;
        std::string event_text;
        float size;
        int show_on_radar;
        int show_on_radar_particle;
        int visible_in_inventory;
        int texture;//texture
        int wielded_script;
        int wielded_disabling_script;
        bool show_condition_help;
        std::vector <int> wield_slots;
        std::vector <item_effect_base> effects;
        std::vector <combines> combinations;
    };

    //base structure for loading light info
    struct general_light_base{
        bool dead;
        int identifier;
        std::string name;
        int texture;
        int type;//0=flashlight,1=omni
        float pulsating;
        float r,g,b,a;
        int particle;
        float particle_flash_speed;
        //float particle_size;
        float particle_time;
        float intensity;
    };

    //base structure for loading plot object info
    struct plot_objects_base{
        bool dead;
        std::string name;
        int identifier;
        int map_type_to_place;//which map type to place to
        int plot_object_class;
        int amount;//how many of these lie around
        int object_definition_number;
        int location_type;//0=random, 1=distance from player start, 2=distance from object
        float location_parameter0;
        float location_parameter1;
        float location_parameter2;
        //int fires;//number of fires around it
        //int clear_area;//area of no trees and rocks around this one
        int show_on_radar;
        int show_on_radar_particle;
        std::string sound_name;
        int sound;
        float live_time;
        bool show_condition_help;

        int trigger_event_by;
        int trigger_event_parameter1;
        std::vector <effect_base> effects;
    };

    //animation frame
    struct animation_frame_base{
        std::string text;
        int start_x;
        int start_y;
        int end_x;
        int end_y;
        int text_y;
    };
    //base structure for loading animation info
    struct animation_base{
        int identifier;
        int texture;//texture
        int frames;

        animation_frame_base frame[20];
    };

    //specialties
    struct specialty{
        int number;
        bool difficulty[3];
        std::string message;
        float parameter0;
        float parameter1;
        float parameter2;
        float parameter3;
    };

    //base structure for loading race info
    struct general_race_base{
        bool dead;
        bool visible_in_start_menu;
        int side;
        int start_area;
        std::string name;
        std::string journal_name;
        int identifier;
        int rag_doll;//texture for paper doll
        float day_speed;
        float temperature_multiplier;

        std::string description;//description in start menu
        int creature_number;//creature number in creatures.dat
        int start_animation;//start animation number in animation.dat
        int interface_texture;
        //int death_animation;
        //int death_by_hunger_animation;
        float maximum_carry_weight;//maximum carry weight
        //float maximum_battery;//maximum battery
        //float weather_damage_multiplier;//weather damage multiplier
        std::vector <specialty> specialties;

        std::vector <int> disabled_endings;

        std::vector <int> disabled_item_classes;
        std::vector <std::string> disabled_item_classes_text;


        struct weapon_class{
            int texture;
            int frame;
            float x, y;
            int can_use;
        };
        std::vector <weapon_class> weapon_classes;

        struct slot_base{
            bool active;
            int texture;
            float x, y;
        };
        std::vector <slot_base> slots;
    };


    //base structure for loading AI info
    struct AI_tactic_base{
        struct anger_level{
            int action;
            float start;
            float size;
            float parameter0;
            bool can_shoot;
            bool can_eat;
        };

        std::string name;
        int identifier;
        std::vector <anger_level> levels;
    };

    //base structure for loading side info
    struct AI_side{
        std::string name;
        std::vector <bool> friend_with_side;
    };

    //base structure for loading bar info
    struct bar_base{
        std::string name;
        bool dead;
        int identifier;
        int bar_type;
        float parameter0;
        int visible;
        int visible_on_enemies;
            bool show_number;
            int anchor_point;
            float location_x;
            float location_y;
            float height;
            float length;
            float color_min_r;
            float color_min_g;
            float color_min_b;
            float color_max_r;
            float color_max_g;
            float color_max_b;

        int bar_picture;
        int background_picture;
        float background_picture_x_offset;
        float background_picture_y_offset;
        float background_picture_width;
        float background_picture_height;
    };

    //base structure for loading script info
    struct script{
        bool dead;
        int run_without_calling;
        int calling_position;
        int calling_creature;
        int identifier;

        int interval;
        std::string name;
        std::string message;
        int message_type;
        std::string sound_name;
        int sound;
        int disable_after_first_use;
        float delay;
        //float timer;

        std::vector <condition> conditions;
        std::vector <effect> effects;
    };

    //collision polygons
    struct polygon_base{
        std::string name;
        int identifier;

        std::vector <point2d> points;
        std::vector <point2d> grown_points;
    };

    //terrain type
    struct terrain_type_base{
        std::string name;
        bool dead;
        int identifier;

        std::vector <object_animation_frame_base> terrain_frames;
        bool AI_avoid;
        bool do_not_place_on_map_edges;
        float base_r,base_g,base_b;
        bool do_not_place_random_objects;
        float footstep_particle_time;
        int override_footstep_particle;
        int override_footstep_sound;
        std::string override_footstep_sound_name;

        struct terrain_effect_base{
            effect_base effect;
            float interval;
        };

        std::vector <terrain_effect_base> effects;
    };

    //terrain map
    struct terrain_map_base{
        std::string name;
        bool dead;
        int identifier;

        //terrain grid
        struct terrain_grid_base{
            int terrain_type;
            //int AI_avoid;
            int no_random_items;

        };
        //terrain grid
        struct terrain_grid_row_base{
            std::vector <terrain_grid_base> terrain_blocks;
        };

        //editor placed objects
        struct editor_object_base{
            int type;
            int number;
            float x,y;
            double rotation;
            int amount;//for items
            int side;//for creatures
            float size;//for lights, plot_objects
        };

        std::vector <terrain_grid_row_base> terrain_grid;
        std::vector <editor_object_base> map_objects;

    };

    //dialogs
    struct dialog_base{
        std::string name;
        bool dead;
        int identifier;
        std::string text;
        float duration;
        int next_line;
        float r,g,b;
    };

    //music
    struct music_base{
        std::string name;
        bool dead;
        int identifier;
        bool can_be_random;
    };

    std::vector <general_race_base> general_races;//race info from file
    std::vector <std::string> difficulty_level_descriptions;
    std::vector <animation_base> animations;
    std::vector <general_light_base> general_lights;//light info from file
    std::vector <std::string> plot_object_classes;
    std::vector <plot_objects_base> general_plot_objects;//plot object info from file
    std::vector <general_area_special_base> general_areas;//stores area info
    std::vector <general_climate_base> general_climates;//area info from file
    std::vector <general_weapons_base> general_weapons;//weapon info from file
    std::vector <general_creatures_base> general_creatures;//creature info from file
    std::vector <general_objects_base> general_objects;//object (trees, houses) info from file
    std::vector <general_item_base> general_items;//item info from file
    std::vector <AI_tactic_base> AI_tactics;//AI info from file
    std::vector <AI_side> AI_sides;
    std::vector <bar_base> general_bars;
    std::vector <script> scripts;
    std::vector <polygon_base> polygons;
    std::vector <terrain_type_base> terrain_types;
    std::vector <terrain_map_base> terrain_maps;
    std::vector <dialog_base> dialogs;
    std::vector <music_base> music;

    std::string mod_name;
    debugger *debug;
    resource_handler *resources;

    void print_effect_numbers(FILE *fil);
    void load_animation_info(const std::string& filename);//loads item info from file
    void load_light_info(const std::string& filename);//loads light info from file
    void load_plot_object_info(const std::string& filename);//loads plot object info from file
    void load_area_info(const std::string& filename);//loads area info from file
    void load_climate_info(const std::string& filename);//loads climate info from file
    void load_creature_info(const std::string& filename);//loads creature info from file
    void load_item_info(const std::string& filename);//loads object info from file
    void load_object_info(const std::string& filename);//loads object info from file
    void load_weapon_info(const std::string& filename);//loads weapon info from file
    void load_race_info(const std::string& filename);//loads race info from file
    void load_polygons(const std::string& filename);
    void grow_polygon(polygon_base *temp_polygon);
    void load_scripts(const std::string& filename);
    void load_AI_info(const std::string& filename);//loads AI info from file
    void load_AI_side(const std::string& filename);
    void load_bars(const std::string& filename);
    void load_terrain_types(const std::string& filename);
    void load_terrain_maps(const std::string& filename);
    void save_terrain_maps(std::string filename);
    void load_dialogs(const std::string& filename);
    void load_music(const std::string& filename);

    void load_mod(const std::string& mod, debugger *debugger, resource_handler *resources);

    //editor();//constructor
    //~light();//destructor

};


#endif
