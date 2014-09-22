#ifndef __EDITOR_H__
#define __EDITOR_H__

//#include "..\grim_api\grim.h"
#include "mod_loader.h"
#include "text_output.h"
#include "resource_handler.h"

//class editor;

//#define grid_size 128.0f//how many pixels is each grid square

//light effects, flashlight, explosions
class Editor
{
protected:
	float grid_size_zoom;
	float zoom;
	resource_handler *resources;
	Engine *grim;
	bool show_exit, show_help;
	Mod *mod_to_edit;
	text_output *text_manager;
	float screen_width;
	float screen_height;
	float camera_x,camera_y;
	int edited_area;
	int selector_active;
	float selector_spot;
	int screen_start_x,screen_start_y,screen_end_x,screen_end_y;//which parts of the map are drawn
	int spot_texture;
	//int paint_tool_number;
	//int paint_tool_type;

	int select_type;//0=paint_tool, 1=select object box, 2=move selected objects
	float select_start_x,select_start_y,select_end_x,select_end_y;
	vector <int> selected_objects;
	Mod::terrain_map_base::editor_object_base paint_tool_object;
	char temprivi[500];//temporary char table
	bool key_w,key_w2,key_s,key_s2,key_a,key_a2,key_d,key_d2,key_z,key_z2,key_x,key_x2,key_f,key_f2;
	sinecosine sincos;
	int start_norandom;

	void draw_map_grid(float elapsed, mouse_control_base mouse_controls);
	void draw_selector(float elapsed, mouse_control_base mouse_controls);


	void show_selector(int type);
	void create_new_terrain_map(void);
	void map_add_row(Mod::terrain_map_base *temp_map, bool remove);
	void map_add_column(Mod::terrain_map_base *temp_map, bool remove);
	void draw_brush(mouse_control_base mouse_controls, float elapsed);
	void find_object_type(int type, int number, bool *dead, string *name, int *texture, float *size);
	void object_draw(Mod::terrain_map_base::editor_object_base *object, float elapsed, bool text_info);
	void paint(int x_grid, int y_grid, int terrain_type, int flood_fill);
	void center_map(void);

public:

	bool render_map(float elapsed, mouse_control_base mouse_controls);
	void start_editor(Engine *engine, Mod *mod_to_edit, text_output *text_manager, resource_handler *resources, float screen_width, float screen_height);

    Editor();//constructor
    //~light();//destructor

};


#endif
