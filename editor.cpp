#include "editor.h"

bool Editor::render_map(float elapsed, mouse_control_base mouse_controls){
	bool quit_editor=false;
	grim->System_ClearScreen(0,0,0,1);

	grim->Quads_SetRotation(0);
	grim->System_SetState_Blending(true);
	grim->Quads_SetColor(1,1,1,1);
	//grim->System_SetState(grSTATE_FONTSCALE,1.0f);

	key_w2=key_w;
	key_w=grim->Key_Down(KEY_W);
	key_s2=key_s;
	key_s=grim->Key_Down(KEY_S);
	key_a2=key_a;
	key_a=grim->Key_Down(KEY_A);
	key_d2=key_d;
	key_d=grim->Key_Down(KEY_D);
	key_z2=key_z;
	key_z=grim->Key_Down(KEY_Z);
	key_x2=key_x;
	key_x=grim->Key_Down(KEY_X);
	key_f2=key_f;
	key_f=grim->Key_Down(KEY_F);

	if(selector_active>0){
		draw_selector(elapsed,mouse_controls);
	}
	else{
		//change edited map
		if(key_z2&&!key_z){
			edited_area--;
			if(edited_area<0)edited_area=mod_to_edit->terrain_maps.size()-1;
			while(mod_to_edit->terrain_maps[edited_area].dead){
				edited_area--;
				if(edited_area<0)edited_area=mod_to_edit->terrain_maps.size()-1;
			}
			string name=mod_to_edit->terrain_maps[edited_area].name;
			name+=" (";
			//itoa(mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size(),temprivi,10);
			sprintf(temprivi,"%u",mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size());
			name+=temprivi;
			name+="x";
			//itoa(mod_to_edit->terrain_maps[edited_area].terrain_grid.size(),temprivi,10);
			sprintf(temprivi,"%u",mod_to_edit->terrain_maps[edited_area].terrain_grid.size());
			name+=temprivi;
			name+=")";
			text_manager->message(3000,3000,name);
			center_map();
		}
		if(key_x2&&!key_x){
			edited_area++;
			while(mod_to_edit->terrain_maps[edited_area].dead){
				edited_area++;
				if(edited_area<mod_to_edit->terrain_maps.size())
					break;
			}
			if(edited_area<mod_to_edit->terrain_maps.size()){
				string name=mod_to_edit->terrain_maps[edited_area].name;
				name+=" (";
				//itoa(mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size(),temprivi,10);
				sprintf(temprivi,"%u",mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size());
				name+=temprivi;
				name+="x";
				//itoa(mod_to_edit->terrain_maps[edited_area].terrain_grid.size(),temprivi,10);
				sprintf(temprivi,"%u",mod_to_edit->terrain_maps[edited_area].terrain_grid.size());
				name+=temprivi;
				name+=")";
				text_manager->message(3000,3000,name);
				center_map();
			}
		}

		//map not selected
		if(edited_area>=mod_to_edit->terrain_maps.size()){
			edited_area=mod_to_edit->terrain_maps.size();

			text_manager->message(100,100,"Press F11 to create a new map.");
			if(grim->Key_Click(KEY_F11)){
				create_new_terrain_map();
			}
		}
		//a map is selected
		else{

			//which parts to draw
			int draw_size_x=(int)(screen_width/grid_size_zoom)+4;
			int draw_size_y=(int)(screen_height/grid_size_zoom)+4;
			int alku_x=(int)(camera_x/grid_size_zoom)-2;
			int alku_y=(int)(camera_y/grid_size_zoom)-2;
			int loppu_x=alku_x+draw_size_x;
			int loppu_y=alku_y+draw_size_y;

			//odd fix for smaller screen sizes
			//if((screen_height==480)||(screen_height==600)){loppu_x+=1;loppu_y+=2;}
			//if((screen_height==1200)||(screen_height==1600)){loppu_x+=1;loppu_y+=1;}

			//save which parts are drawn
			screen_start_x=alku_x;
			screen_start_y=alku_y;
			screen_end_x=loppu_x;
			screen_end_y=loppu_y;

			if(screen_start_x<0)screen_start_x=0;
			if(screen_start_y<0)screen_start_y=0;
			if(screen_end_x>mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size())screen_end_x=mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size();
			if(screen_end_y>mod_to_edit->terrain_maps[edited_area].terrain_grid.size())screen_end_y=mod_to_edit->terrain_maps[edited_area].terrain_grid.size();

			draw_map_grid(elapsed,mouse_controls);

			if(paint_tool_object.type>0)
				draw_brush(mouse_controls,elapsed);

			if(grim->Key_Down(KEY_UP)||(mouse_controls.mousey<1))
				camera_y-=elapsed;
			if(grim->Key_Down(KEY_DOWN)||(mouse_controls.mousey>screen_height-1))
				camera_y+=elapsed;
			if(grim->Key_Down(KEY_LEFT)||(mouse_controls.mousex<1))
				camera_x-=elapsed;
			if(grim->Key_Down(KEY_RIGHT)||(mouse_controls.mousex>screen_width-1))
				camera_x+=elapsed;

			grim->Quads_SetRotation(0);
			string coordinates="x=";
			//itoa((int)(mouse_controls.mousex+camera_x)/grid_size_zoom,temprivi,10);
			sprintf(temprivi,"%d",static_cast<int>((mouse_controls.mousex+camera_x)/grid_size_zoom));
			coordinates+=temprivi;
			coordinates+=" y=";
			//itoa((int)(mouse_controls.mousey+camera_y)/grid_size_zoom,temprivi,10);
			sprintf(temprivi,"%d",static_cast<int>((mouse_controls.mousey+camera_y)/grid_size_zoom));
			coordinates+=temprivi;
			text_manager->write(-1,coordinates,1,20,screen_height-100+60,screen_width,screen_height,false,1,1,1,1);

			//clone map
			if(grim->Key_Click(KEY_F8)){

				Mod::terrain_map_base temp_map=mod_to_edit->terrain_maps[edited_area];
				temp_map.dead=false;
				temp_map.identifier=mod_to_edit->terrain_maps.size();
				temp_map.name="map ";
				//itoa(temp_map.identifier,temprivi,10);
				sprintf(temprivi,"%d",temp_map.identifier);
				temp_map.name+=temprivi;
				camera_x=0;
				camera_y=0;

				mod_to_edit->terrain_maps.push_back(temp_map);

				text_manager->message(3000,3000,"Map cloned.");
				edited_area=temp_map.identifier;
			}

			//empty map
			if(grim->Key_Click(KEY_F9)){
				Mod::terrain_map_base *temp_map=&mod_to_edit->terrain_maps[edited_area];

				//clear grid
				temp_map->terrain_grid.clear();

				//clear objects
				temp_map->map_objects.clear();

				//add first grid point,
				Mod::terrain_map_base::terrain_grid_base temp_point;
				Mod::terrain_map_base::terrain_grid_row_base temp_row;
				temp_point.no_random_items=0;
				temp_point.terrain_type=0;
				temp_row.terrain_blocks.push_back(temp_point);
				temp_map->terrain_grid.push_back(temp_row);

				text_manager->message(3000,3000,"Map cleared.");
			}
			//zoom
			if(mouse_controls.mouse_wheel>0){
				//camera_x=mouse_controls.mousex+camera_x;//-screen_width/2*zoom;
				//camera_y=mouse_controls.mousey+camera_y;//-screen_height/2*zoom;

				float middle_x=(screen_width/2+camera_x)/zoom;
				float middle_y=(screen_height/2+camera_y)/zoom;
				zoom+=zoom*elapsed*0.01f;
				camera_x=middle_x*zoom-screen_width/2;
				camera_y=middle_y*zoom-screen_height/2;
			}
			if(mouse_controls.mouse_wheel<0){
				//float middle_x=(mouse_controls.mousex+camera_x)/zoom;
				//float middle_y=(mouse_controls.mousey+camera_y)/zoom;
				float middle_x=(screen_width/2+camera_x)/zoom;
				float middle_y=(screen_height/2+camera_y)/zoom;
				zoom-=zoom*elapsed*0.01f;
				if(zoom<0.05f)zoom=0.05f;
				camera_x=middle_x*zoom-screen_width/2;
				camera_y=middle_y*zoom-screen_height/2;

				//camera_x=(camera_x+(middle_x*zoom-screen_width/2))/2;
				//camera_y=(camera_y+(middle_y*zoom-screen_height/2))/2;
			}
			grid_size_zoom=zoom*128;
		}
	}

	if(grim->Key_Click(KEY_F2)){
		show_selector(1);
	}
	if(grim->Key_Click(KEY_F3)){
		show_selector(2);
	}
	if(grim->Key_Click(KEY_F4)){
		show_selector(3);
	}
	if(grim->Key_Click(KEY_F5)){
		show_selector(4);
	}
	if(grim->Key_Click(KEY_F6)){
		show_selector(5);
	}
	if(grim->Key_Click(KEY_F7)){
		show_selector(6);
	}
	if(grim->Key_Click(KEY_ESCAPE)){

		if(show_help){
			show_help=false;
		}
		else if(selector_active>0){
			selector_active=0;
		}
		else
			show_exit=!show_exit;
	}
	if(grim->Key_Click(KEY_F1)){
		show_help=!show_help;
	}

	//save
	if(grim->Key_Click(KEY_F12)){
		mod_to_edit->save_terrain_maps("");
		text_manager->message(3000,3000,"Saved.");
	}

	//show exit text
	grim->Quads_SetColor(1,1,1,1);
	grim->Quads_SetRotation(0);
	if(show_exit){
		text_manager->write_line(-1,10,30,"Press F10 to exit without saving",1);
		text_manager->write_line(-1,10,50,"Press F12 to save",1);
		/*if(selector_active>0)
			selector_active=0;
		else*/
		{
			if(grim->Key_Click(KEY_F10)){
				quit_editor=true;
				show_exit=false;
			}
		}
	}
	//show help text
	else if(show_help){
		text_manager->write(-1,"F1=help \\ F2=show terrain palette \\ F3=show item palette \\ F4=show plot_object palette \\ F5=show creature palette \\ F6=show light palette \\ F7=show props palette \\ F8=clone map \\ F9=clear map \\ F11=new map \\ F12=save changes \\ Z,X=change edited map \\ A,D=rotate object (slow turn=left shift, fast turn=left control) \\ W,S=change object's parameter \\ HOME/END=change map size \\ DELETE/PAGEDOWN=change map size / delete selected objects \\ Right mouse button=pick up object \\ Left mouse button=drop object / select multiple \\ F=flood fill terrain / bring object to front",1,20,20,screen_width,screen_height,false,1,1,1,1);
	}

	if(quit_editor) return false;
	return true;
}

void Editor::start_editor(Engine *grim, Mod *mod_to_edit, text_output *text_manager, resource_handler *resources, float screen_width, float screen_height){
	this->grim=grim;
	this->mod_to_edit=mod_to_edit;
	this->screen_width=screen_width;
	this->screen_height=screen_height;
	this->text_manager=text_manager;
	this->resources=resources;
	show_exit=false;
	show_help=false;
	edited_area=0;
	paint_tool_object.type=0;
	grid_size_zoom=128.0f;
	zoom=1;
	select_type=0;

	spot_texture=resources->load_texture("bar.png","Default");

	camera_x=0;
	camera_y=0;

	key_w=false;
	key_w2=false;
	key_s=false;
	key_s2=false;
	key_a=false;
	key_a2=false;
	key_d=false;
	key_d2=false;
	key_z=false;
	key_z2=false;
	key_x=false;
	key_x2=false;

	center_map();
}


Editor::Editor(){
}


void Editor::draw_map_grid(float elapsed, mouse_control_base mouse_controls){//renders map grid

	int i,j;
	int texture;

	grim->Quads_SetRotation(0);
	grim->Quads_SetSubset(0,0,1,1);
	grim->Quads_SetColor(1,1,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);

	//grid
	grim->System_SetState_Blending(false);
	for(unsigned int k=0;k<mod_to_edit->terrain_types.size();k++){
		texture=mod_to_edit->terrain_types[k].terrain_frames[0].texture;
		resources->Texture_Set(texture);
		grim->Quads_Begin();
		for(i=screen_start_x;i<screen_end_x;i++){
			for(j=screen_start_y;j<screen_end_y;j++){
				if(mod_to_edit->terrain_maps[edited_area].terrain_grid[j].terrain_blocks[i].terrain_type==k){


					grim->Quads_Draw(-camera_x+i*grid_size_zoom, -camera_y+j*grid_size_zoom, grid_size_zoom, grid_size_zoom);

					if(mod_to_edit->terrain_maps[edited_area].terrain_grid[j].terrain_blocks[i].no_random_items==1){
						grim->Quads_End();
						grim->System_SetState_Blending(true);
						grim->Quads_SetColor(1,1,1,1.0f);
						resources->Texture_Set(mod_to_edit->terrain_types[1].terrain_frames[0].texture);
						grim->Quads_Begin();
						grim->Quads_Draw(-camera_x+i*grid_size_zoom, -camera_y+j*grid_size_zoom, grid_size_zoom, grid_size_zoom);
						grim->Quads_End();
						resources->Texture_Set(texture);
						grim->Quads_SetColor(1,1,1,1);
						grim->System_SetState_Blending(false);
						grim->Quads_Begin();
					}
				}
			}
		}
		grim->Quads_End();
	}

	//objects
	int closest=-1;
	float closest_distance=0;
	grim->System_SetState_Blending(true);
	for(unsigned int k=0;k<mod_to_edit->terrain_maps[edited_area].map_objects.size();k++){
		Mod::terrain_map_base::editor_object_base temp_object=mod_to_edit->terrain_maps[edited_area].map_objects[k];

		float size=grid_size_zoom;
		bool dead;
		string name;
		find_object_type(temp_object.type,temp_object.number,&dead,&name,&texture,&size);
		if((temp_object.type==3)||(temp_object.type==5))size=size*temp_object.size;
		if(size<16)size=16;
		//float x=temp_object.x*zoom-camera_x-0.5f*size;
		//float y=temp_object.y*zoom-camera_y-0.5f*size;
		float distance=sqr((temp_object.x*zoom-camera_x)-mouse_controls.mousex)+sqr((temp_object.y*zoom-camera_y)-mouse_controls.mousey);
		if((distance<sqr(size))&&((distance<closest_distance)||(closest==-1))){
			closest=k;
			closest_distance=distance;
		}
		/*grim->Quads_SetRotation(temp_object.rotation);
		resources->Texture_Set(texture);

		grim->Quads_Begin();
		grim->Quads_Draw(x, y, size, size);
		grim->Quads_End();*/
		grim->Quads_SetColor(0.7f,0.7f,0.7f,1);
		object_draw(&temp_object,elapsed,false);

	}

	//start selection box
	if((select_type==0)&&(paint_tool_object.type==0))
	if(mouse_controls.mouse_left&&!mouse_controls.mouse_left2){
		select_start_x=(mouse_controls.mousex+camera_x)/zoom;
		select_start_y=(mouse_controls.mousey+camera_y)/zoom;
		select_type=1;
	}

	//draw selection box
	if(select_type==1){
		select_end_x=(mouse_controls.mousex+camera_x)/zoom;
		select_end_y=(mouse_controls.mousey+camera_y)/zoom;

		float x1=select_start_x*zoom;
		float y1=select_start_y*zoom;
		float x2=select_end_x*zoom;
		float y2=select_end_y*zoom;
		text_manager->draw_line(x1-camera_x,y1-camera_y,x2-camera_x,y1-camera_y,2,1,1,1,1,1);
		text_manager->draw_line(x2-camera_x,y1-camera_y,x2-camera_x,y2-camera_y,2,1,1,1,1,1);
		text_manager->draw_line(x2-camera_x,y2-camera_y,x1-camera_x,y2-camera_y,2,1,1,1,1,1);
		text_manager->draw_line(x1-camera_x,y2-camera_y,x1-camera_x,y1-camera_y,2,1,1,1,1,1);


		//end selection box
		if(!mouse_controls.mouse_left){
			select_type=2;

			if(select_start_x>select_end_x){
				float temp=select_start_x;
				select_start_x=select_end_x;
				select_end_x=temp;
			}
			if(select_start_y>select_end_y){
				float temp=select_start_y;
				select_start_y=select_end_y;
				select_end_y=temp;
			}

			//list all the selected objects
			selected_objects.clear();
			for(unsigned int k=0;k<mod_to_edit->terrain_maps[edited_area].map_objects.size();k++){
				if((mod_to_edit->terrain_maps[edited_area].map_objects[k].x>select_start_x)
				&&(mod_to_edit->terrain_maps[edited_area].map_objects[k].x<select_end_x)
				&&(mod_to_edit->terrain_maps[edited_area].map_objects[k].y>select_start_y)
				&&(mod_to_edit->terrain_maps[edited_area].map_objects[k].y<select_end_y)){
					selected_objects.push_back(k);
				}

			}

			select_start_x=(mouse_controls.mousex+camera_x)/zoom;
			select_start_y=(mouse_controls.mousey+camera_y)/zoom;
			select_end_x=(mouse_controls.mousex+camera_x)/zoom;
			select_end_y=(mouse_controls.mousey+camera_y)/zoom;

		}
	}

	//move selected items
	if(select_type==2){
		select_end_x=(mouse_controls.mousex+camera_x)/zoom;
		select_end_y=(mouse_controls.mousey+camera_y)/zoom;
		float moved_x=select_end_x-select_start_x;
		float moved_y=select_end_y-select_start_y;
		for(unsigned int k=0;k<selected_objects.size();k++){
			mod_to_edit->terrain_maps[edited_area].map_objects[selected_objects[k]].x+=moved_x;
			mod_to_edit->terrain_maps[edited_area].map_objects[selected_objects[k]].y+=moved_y;
		}
		select_start_x=select_end_x;
		select_start_y=select_end_y;

		//delete selected objects
		if(grim->Key_Click(KEY_DELETE)){
			for(unsigned int k=0;k<selected_objects.size();k++){
				mod_to_edit->terrain_maps[edited_area].map_objects.erase(mod_to_edit->terrain_maps[edited_area].map_objects.begin()+selected_objects[k]);
				for(int a=k+1;a<selected_objects.size();a++){
					if(selected_objects[a]>selected_objects[k])
						selected_objects[a]--;
				}
			}
			select_type=0;
		}

		//unselect
		if(mouse_controls.mouse_left||mouse_controls.mouse_right){
			select_type=0;
		}

	}

	//deselect brush
	if(select_type==0){
		if(paint_tool_object.type!=0){
			if(!mouse_controls.mouse_right&&mouse_controls.mouse_right2){
				paint_tool_object.type=0;
				mouse_controls.mouse_right2=false;
			}
		}

		//highlight selected object
		if((paint_tool_object.type==0)&&(closest>=0)){
			Mod::terrain_map_base::editor_object_base *temp_object=&mod_to_edit->terrain_maps[edited_area].map_objects[closest];

			//info
			grim->Quads_SetColor(1,1,1,1);
			object_draw(temp_object,elapsed,true);

			//select this
			if(!mouse_controls.mouse_right&&mouse_controls.mouse_right2){
				//set paint tool
				select_type=0;
				paint_tool_object=*temp_object;

				//remove from map
				mod_to_edit->terrain_maps[edited_area].map_objects.erase(mod_to_edit->terrain_maps[edited_area].map_objects.begin()+closest);

			}

			//bring to front
			if(!key_f&&key_f2){

				Mod::terrain_map_base::editor_object_base temp_object2=mod_to_edit->terrain_maps[edited_area].map_objects[closest];

				//remove from map
				mod_to_edit->terrain_maps[edited_area].map_objects.erase(mod_to_edit->terrain_maps[edited_area].map_objects.begin()+closest);

				//add to map
				mod_to_edit->terrain_maps[edited_area].map_objects.push_back(temp_object2);

			}

		}
	}

	//change map size
	bool show_new_size=false;
	if(grim->Key_Click(KEY_END)){
		map_add_row(&mod_to_edit->terrain_maps[edited_area],false);
		show_new_size=true;
	}
	if(grim->Key_Click(KEY_HOME)){
		map_add_row(&mod_to_edit->terrain_maps[edited_area],true);
		show_new_size=true;
	}
	if(grim->Key_Click(KEY_PGDN)){
		map_add_column(&mod_to_edit->terrain_maps[edited_area],false);
		show_new_size=true;
	}
	if(grim->Key_Click(KEY_DELETE)){
		map_add_column(&mod_to_edit->terrain_maps[edited_area],true);
		show_new_size=true;
	}

	if(show_new_size){
		string new_size="New size: ";
		//itoa(mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size(),temprivi,10);
		sprintf(temprivi,"%u",mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size());
		new_size+=temprivi;
		new_size+="x";
		//itoa(mod_to_edit->terrain_maps[edited_area].terrain_grid.size(),temprivi,10);
		sprintf(temprivi,"%u",mod_to_edit->terrain_maps[edited_area].terrain_grid.size());
		new_size+=temprivi;
		text_manager->message(500,500,new_size);
	}

	//map limits
	grim->Quads_SetRotation(0);
	grim->Quads_SetSubset(0,0,1,1);
	grim->Quads_SetColor(1,1,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	float x1=grid_size_zoom;
	float y1=grid_size_zoom;
	float x2=mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size()*grid_size_zoom-grid_size_zoom*2;
	float y2=mod_to_edit->terrain_maps[edited_area].terrain_grid.size()*grid_size_zoom-grid_size_zoom*2;
	text_manager->draw_line(x1-camera_x,y1-camera_y,x2-camera_x,y1-camera_y,2,1,1,1,1,1);
	text_manager->draw_line(x2-camera_x,y1-camera_y,x2-camera_x,y2-camera_y,2,1,1,1,1,1);
	text_manager->draw_line(x2-camera_x,y2-camera_y,x1-camera_x,y2-camera_y,2,1,1,1,1,1);
	text_manager->draw_line(x1-camera_x,y2-camera_y,x1-camera_x,y1-camera_y,2,1,1,1,1,1);

}



void Editor::draw_selector(float elapsed, mouse_control_base mouse_controls){//renders map grid

	int texture;
	string name;

	grim->Quads_SetRotation(0);
	grim->Quads_SetSubset(0,0,1,1);
	grim->Quads_SetColor(1,1,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->System_SetState_Blending(true);

	int fit_width=screen_width/128;

	int items_amount=0;
	switch(selector_active){
	//terrain
	case 1:
		items_amount=mod_to_edit->terrain_types.size();
		break;
	//items
	case 2:
		items_amount=mod_to_edit->general_items.size();
		break;
	//plot_objects
	case 3:
		items_amount=mod_to_edit->general_plot_objects.size();
		break;
	//creatures
	case 4:
		items_amount=mod_to_edit->general_creatures.size();
		break;
	//lights
	case 5:
		items_amount=mod_to_edit->general_lights.size();
		break;
	//props
	case 6:
		items_amount=mod_to_edit->general_objects.size();
		break;
	}

	int slot=0;
	string name_info="";
	for(int a=0;a<items_amount;a++){
		float size=0;
		bool dead;
		find_object_type(selector_active,a,&dead,&name,&texture,&size);
		if(dead)continue;

		int x=slot%fit_width;
		int y=slot/fit_width;

		grim->Quads_SetColor(0.8f,0.8f,0.8f,1);
		//mouse on
		if((mouse_controls.mousex>x*128)&&(mouse_controls.mousex<x*128+128)&&(mouse_controls.mousey>y*128-selector_spot)&&(mouse_controls.mousey<y*128-selector_spot+128)){
			name_info=name;
			grim->Quads_SetColor(1,1,1,1);
			if(!mouse_controls.mouse_left&&mouse_controls.mouse_left2){
				select_type=0;
				paint_tool_object.type=selector_active;
				selector_active=0;
				paint_tool_object.number=a;
				paint_tool_object.amount=1;
				paint_tool_object.side=0;
				paint_tool_object.size=1;
				paint_tool_object.rotation=0;
			}
		}

		resources->Texture_Set(texture);
		grim->Quads_Begin();
		grim->Quads_Draw(x*128, y*128-selector_spot, 128, 128);
		grim->Quads_End();

		slot++;
	}

	if(name_info!=""){
		float x=mouse_controls.mousex+20;
		if(x>screen_width-256)x=screen_width-256;
		text_manager->write(-1,name_info,1,x,mouse_controls.mousey,screen_width,screen_height,false,1,1,1,1);
	}

	if(grim->Key_Down(KEY_UP)||(mouse_controls.mousey<=1)||(mouse_controls.mouse_wheel>0)){
		float speed=2;
		if(mouse_controls.mouse_wheel!=0)speed=30;
		selector_spot-=elapsed*speed;
	}
	if(grim->Key_Down(KEY_DOWN)||(mouse_controls.mousey>=screen_height-2)||(mouse_controls.mouse_wheel<0)){
		float speed=2;
		if(mouse_controls.mouse_wheel!=0)speed=30;
		selector_spot+=elapsed*speed;
	}
	if(selector_spot<0)selector_spot=0;

}

void Editor::show_selector(int type){
	selector_spot=0;
	selector_active=type;
}

void Editor::create_new_terrain_map(void){
	Mod::terrain_map_base temp_map;
	temp_map.dead=false;
	temp_map.identifier=mod_to_edit->terrain_maps.size();
	temp_map.name="map ";
	//itoa(temp_map.identifier,temprivi,10);
	sprintf(temprivi,"%d",temp_map.identifier);
	temp_map.name+=temprivi;
	camera_x=0;
	camera_y=0;

	//add first grid point,
	Mod::terrain_map_base::terrain_grid_base temp_point;
	Mod::terrain_map_base::terrain_grid_row_base temp_row;
	temp_point.no_random_items=0;
	temp_point.terrain_type=0;
	temp_row.terrain_blocks.push_back(temp_point);
	temp_map.terrain_grid.push_back(temp_row);

	mod_to_edit->terrain_maps.push_back(temp_map);


}

void Editor::map_add_row(Mod::terrain_map_base *temp_map, bool remove){
	if(!remove){
		temp_map->terrain_grid.push_back(temp_map->terrain_grid[temp_map->terrain_grid.size()-1]);
	}
	else{
		if(temp_map->terrain_grid.size()>1)
			temp_map->terrain_grid.pop_back();
	}
}

void Editor::map_add_column(Mod::terrain_map_base *temp_map, bool remove){

	for(int a=0;a<temp_map->terrain_grid.size();a++){
		if(!remove){
			temp_map->terrain_grid[a].terrain_blocks.push_back(temp_map->terrain_grid[a].terrain_blocks[temp_map->terrain_grid[a].terrain_blocks.size()-1]);
		}
		else{
			if(temp_map->terrain_grid[a].terrain_blocks.size()>1)
				temp_map->terrain_grid[a].terrain_blocks.pop_back();
		}
	}

}

void Editor::draw_brush(mouse_control_base mouse_controls, float elapsed){
	int texture=-1;
	float size=0;
	bool dead;
	string name;

	find_object_type(paint_tool_object.type,paint_tool_object.number,&dead,&name,&texture,&size);
	switch(paint_tool_object.type){
		//terrain
		case 1:
			if(paint_tool_object.number<mod_to_edit->terrain_types.size()){
				int x_grid=(int)((camera_x+mouse_controls.mousex)/grid_size_zoom);
				int y_grid=(int)((camera_y+mouse_controls.mousey)/grid_size_zoom);
				paint_tool_object.x=x_grid*grid_size_zoom;
				paint_tool_object.y=y_grid*grid_size_zoom;

				//painting
				//any terrain
				if(paint_tool_object.number!=1){
					if(mouse_controls.mouse_left){
						paint(x_grid,y_grid,paint_tool_object.number,-1);
					}
				}
				//norandom must have one click only
				else{
					if(!mouse_controls.mouse_left){
						start_norandom=-1;
					}
					//if(!mouse_controls.mouse_left&&mouse_controls.mouse_left2){
					if(mouse_controls.mouse_left){
						if(start_norandom==-1)start_norandom=mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items;
						paint(x_grid,y_grid,paint_tool_object.number,-1);
					}
				}

				if(!key_f&&key_f2){
					paint(x_grid,y_grid,paint_tool_object.number,-2);
				}

				grim->Quads_SetRotation(0);
				grim->Quads_SetSubset(0,0,1,1);
				grim->Quads_SetColor(1,1,1,1);
				grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
				grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
				grim->System_SetState_Blending(true);
				resources->Texture_Set(texture);
				grim->Quads_Begin();
				grim->Quads_Draw(paint_tool_object.x-camera_x, paint_tool_object.y-camera_y, grid_size_zoom, grid_size_zoom);
				grim->Quads_End();
			}
			break;
		//items
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			//if(paint_tool_object.number<mod_to_edit->general_items.size())
			{
				paint_tool_object.x=(mouse_controls.mousex+camera_x)/zoom;
				paint_tool_object.y=(mouse_controls.mousey+camera_y)/zoom;
				//painting
				if(!mouse_controls.mouse_left&&mouse_controls.mouse_left2){
					if((paint_tool_object.y*zoom<mod_to_edit->terrain_maps[edited_area].terrain_grid.size()*grid_size_zoom)&&(paint_tool_object.y>=0)&&(paint_tool_object.x*zoom<mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size()*grid_size_zoom)&&(paint_tool_object.x>=0)){
						mod_to_edit->terrain_maps[edited_area].map_objects.push_back(paint_tool_object);
					}
				}
				grim->Quads_SetColor(1,1,1,1);
				object_draw(&paint_tool_object,elapsed,true);
			}
			break;
	}

	/*if(texture>=0){
		grim->Quads_SetRotation(0);
		grim->Quads_SetSubset(0,0,1,1);
		grim->Quads_SetColor(1,1,1,1);
		grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
		grim->System_SetState_Blending(true);
		resources->Texture_Set(texture);
		grim->Quads_Begin();
		grim->Quads_Draw(paint_tool_object.x-camera_x, paint_tool_object.y-camera_y, paint_tool_object.size, paint_tool_object.size);
		grim->Quads_End();
	}*/
}


void Editor::find_object_type(int type, int number, bool *dead, string *name, int *texture, float *size){

	*dead=false;
	switch(type){
		//terrain
		case 1:
			if(mod_to_edit->terrain_types[number].dead){
				*dead=true;
				return;
			}
			*texture=mod_to_edit->terrain_types[number].terrain_frames[0].texture;
			*name=mod_to_edit->terrain_types[number].name;
			*size=grid_size_zoom;
			break;
		//items
		case 2:
			if(mod_to_edit->general_items[number].dead){
				*dead=true;
				return;
			}
			*texture=mod_to_edit->general_items[number].texture;
			*name=mod_to_edit->general_items[number].name;
			*size=mod_to_edit->general_items[number].size*grid_size_zoom;
			break;
		//plot_objects
		case 3:
			if(mod_to_edit->general_plot_objects[number].dead){
				*dead=true;
				return;
			}
			if(mod_to_edit->general_plot_objects[number].object_definition_number>=0){
				*texture=mod_to_edit->general_objects[mod_to_edit->general_plot_objects[number].object_definition_number].animation_frames[0].texture;
				*name=mod_to_edit->general_plot_objects[number].name;
				*size=mod_to_edit->general_objects[mod_to_edit->general_plot_objects[number].object_definition_number].base_size*grid_size_zoom;
			}
			break;

		//creatures
		case 4:
			if(mod_to_edit->general_creatures[number].dead){
				*dead=true;
				return;
			}
			*texture=mod_to_edit->general_creatures[number].texture;
			*name=mod_to_edit->general_creatures[number].name;
			*size=mod_to_edit->general_creatures[number].size*grid_size_zoom/2;
			break;

		//lights
		case 5:
			if(mod_to_edit->general_lights[number].dead){
				*dead=true;
				return;
			}
			*texture=mod_to_edit->general_lights[number].texture;
			*name=mod_to_edit->general_lights[number].name;
			*size=1*grid_size_zoom;
			break;
		//props
		case 6:
			if(mod_to_edit->general_objects[number].dead){
				*dead=true;
				return;
			}
			*texture=mod_to_edit->general_objects[number].animation_frames[0].texture;
			*name=mod_to_edit->general_objects[number].name;
			*size=mod_to_edit->general_objects[number].base_size*grid_size_zoom;

			break;
	}

}

void Editor::object_draw(Mod::terrain_map_base::editor_object_base *object, float elapsed, bool text_info){

	int polygon=-1;
	float collision_size=0;
	int texture;
	bool dead;
	float size;
	string name;
	find_object_type(object->type,object->number,&dead,&name,&texture,&size);
	grim->Quads_SetRotation(object->rotation);
	//grim->Quads_SetColor(1,1,1,1);

	//spot picture for invisible objects
	resources->Texture_Set(spot_texture);
	grim->Quads_SetSubset(0,0,1,1);
	grim->Quads_Begin();
	grim->Quads_Draw((object->x-16)*zoom-camera_x, (object->y-16)*zoom-camera_y, 32*zoom, 32*zoom);
	grim->Quads_End();

	//info
	resources->Texture_Set(texture);
	float text_x=20;
	float text_y=screen_height-100;
	string info;
	bool draw=true;
	switch(object->type){
		//item
		case 2:
			if(text_info){
				info="Amount: ";
				//itoa(object->amount,temprivi,10);
				sprintf(temprivi,"%d",object->amount);
				info+=temprivi;
				if(!key_w&&key_w2)
					object->amount+=1;
				if(object->amount>0)
				if(!key_s&&key_s2)
					object->amount-=1;
			}
			break;
		//plot_objects
		case 3:
			size=size*object->size;
			if(mod_to_edit->general_objects[mod_to_edit->general_plot_objects[object->number].object_definition_number].collision_type==1){
				polygon=mod_to_edit->general_objects[mod_to_edit->general_plot_objects[object->number].object_definition_number].collision_parameter0;
				collision_size=mod_to_edit->general_objects[mod_to_edit->general_plot_objects[object->number].object_definition_number].collision_parameter1;
			}
			if(text_info){
				info="Size: ";
				info+=FloatToText(object->size,3);
				//itoa(object->size,temprivi,10);
				//info+=temprivi;
				if(key_w)
					object->size+=elapsed*0.001f;
				if(key_s)
					object->size-=elapsed*0.001f;
				if(object->size<0.1f)object->size=0.1f;
			}
			break;
		//creatures
		case 4:
			draw=false;
			grim->Quads_Begin();
			float x,y;
			x=object->x*zoom-camera_x-0.5f*size;
			y=object->y*zoom-camera_y-0.5f*size;
			float x0,x1,y0,y1;
			//draw legs
				find_texture_coordinates(7,x0,y0,x1,y1,4);
				grim->Quads_SetSubset(x0,y0,x1,y1);
				grim->Quads_Draw(x, y, size, size);
			//draw torso
				find_texture_coordinates(1,x0,y0,x1,y1,4);
				grim->Quads_SetSubset(x0,y0,x1,y1);
				grim->Quads_Draw(x, y, size, size);
			//draw head
				find_texture_coordinates(0,x0,y0,x1,y1,4);
				grim->Quads_SetSubset(x0,y0,x1,y1);
				grim->Quads_Draw(x, y, size, size);
			grim->Quads_End();

			if(text_info){
				if(!key_w&&key_w2)
					object->side+=1;
				if(!key_s&&key_s2)
					object->side-=1;
				if(object->side<0)object->side=0;
				if(object->side>=mod_to_edit->AI_sides.size())object->side=mod_to_edit->AI_sides.size()-1;

				info="Side: ";
				info+=mod_to_edit->AI_sides[object->side].name;
				//itoa(object->side,temprivi,10);
				//info+=temprivi;

			}
			break;
		//lights
		case 5:
			size=size*object->size;
			if(text_info){
				info="Size: ";
				info+=FloatToText(object->size,3);
				//itoa(object->size,temprivi,10);
				//info+=temprivi;
				if(key_w)
					object->size+=elapsed*0.001f;
				if(key_s)
					object->size-=elapsed*0.001f;
				if(object->size<0.1f)object->size=0.1f;
			}
			break;
		//props
		case 6:
			size=size*object->size;
			if(mod_to_edit->general_objects[object->number].collision_type==1){
				polygon=mod_to_edit->general_objects[object->number].collision_parameter0;
				collision_size=mod_to_edit->general_objects[object->number].collision_parameter1;
			}
			if(text_info){
				info="Size: ";
				info+=FloatToText(object->size,3);
				//itoa(object->size,temprivi,10);
				//info+=temprivi;
				if(key_w)
					object->size+=elapsed*0.001f;
				if(key_s)
					object->size-=elapsed*0.001f;
				if(object->size<0.1f)object->size=0.1f;
			}
			break;
	}

	//picture
	if(draw){
		float x=object->x*zoom-camera_x-0.5f*size;
		float y=object->y*zoom-camera_y-0.5f*size;
		grim->Quads_SetSubset(0,0,1,1);
		grim->Quads_Begin();
		grim->Quads_Draw(x, y, size, size);
		grim->Quads_End();
	}

	//collision lines
	if(polygon>=0){

		float object_size=size;
		float object_x=object->x*zoom;
		float object_y=object->y*zoom;

		//all the lines
		if(mod_to_edit->polygons[polygon].points.size()>1){
			float sin=sincos.table_sin(object->rotation);
			float cos=sincos.table_cos(object->rotation);
			for(unsigned int a=0;a<mod_to_edit->polygons[polygon].points.size()-1;a++){

				float xx1=mod_to_edit->polygons[polygon].points[a].x*object_size*collision_size;
				float yy1=mod_to_edit->polygons[polygon].points[a].y*object_size*collision_size;
				float xx2=mod_to_edit->polygons[polygon].points[a+1].x*object_size*collision_size;
				float yy2=mod_to_edit->polygons[polygon].points[a+1].y*object_size*collision_size;

				float x1=cos*xx1+sin*yy1+object_x;
				float y1=sin*xx1-cos*yy1+object_y;
				float x2=cos*xx2+sin*yy2+object_x;
				float y2=sin*xx2-cos*yy2+object_y;

				text_manager->draw_line(x1-camera_x,y1-camera_y,x2-camera_x,y2-camera_y,2,1,1,1,1,1);

				/*xx1=mod_to_edit->polygons[polygon].grown_points[a].x*object_size*collision_size;
				yy1=mod_to_edit->polygons[polygon].grown_points[a].y*object_size*collision_size;
				xx2=mod_to_edit->polygons[polygon].grown_points[a+1].x*object_size*collision_size;
				yy2=mod_to_edit->polygons[polygon].grown_points[a+1].y*object_size*collision_size;
				x1=cos*xx1+sin*yy1+object_x;
				y1=sin*xx1-cos*yy1+object_y;
				x2=cos*xx2+sin*yy2+object_x;
				y2=sin*xx2-cos*yy2+object_y;
				text_manager->draw_line(x1-camera_x,y1-camera_y,x2-camera_x,y2-camera_y,2,1,1,1,0,1);*/
			}
		}
	}

	//info text
	if(text_info){
		grim->Quads_SetRotation(0);
		text_manager->write(-1,name,1,text_x,text_y,screen_width,screen_height,false,1,1,1,1);
		text_manager->write(-1,info,1,text_x,text_y+20,screen_width,screen_height,false,1,1,1,1);

		float turn_speed=0.002f;
		if(grim->Key_Down(KEY_LSHIFT))
			turn_speed=0.0005f;

		//rotate
		if(grim->Key_Down(KEY_LCONTROL)){
			if(grim->Key_Click(KEY_D)){
				object->rotation+=pi/4.0f;
				if(object->rotation>pi*2)
					object->rotation-=pi*2;
				if(object->rotation<0)
					object->rotation+=pi*2;
				object->rotation=(static_cast<int>(object->rotation/(pi*0.25f)+pi/8.0f))*(pi*0.25f);
			}
			if(grim->Key_Click(KEY_A)){
				object->rotation-=pi/4.0f;
				if(object->rotation>pi*2)
					object->rotation-=pi*2;
				if(object->rotation<0)
					object->rotation+=pi*2;
				object->rotation=(static_cast<int>(object->rotation/(pi*0.25f)+pi/8.0f))*(pi*0.25f);
			}
		} else {
			if(grim->Key_Down(KEY_D)){
				object->rotation+=elapsed*turn_speed;
			}
			if(grim->Key_Down(KEY_A)){
				object->rotation-=elapsed*turn_speed;
			}
		}
		if(object->rotation>pi*2)
			object->rotation-=pi*2;
		if(object->rotation<0)
			object->rotation+=pi*2;
	}
}

void Editor::paint(int x_grid, int y_grid, int terrain_type, int flood_fill){

	if(terrain_type!=1)
	if(flood_fill==terrain_type)
		return;

	if((y_grid<mod_to_edit->terrain_maps[edited_area].terrain_grid.size())&&(y_grid>=0)&&(x_grid<mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks.size())&&(x_grid>=0)){

		//asked for flood fill
		if(flood_fill==-2){
			if(terrain_type==1)
				flood_fill=mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items;
			else
				flood_fill=mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].terrain_type;
		}

		//see if this terrain is the asked flood fill terrain
		if(flood_fill>=0){
			if(terrain_type==1){
				if(flood_fill!=mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items)
					return;
			}
			else{
				if(flood_fill!=mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].terrain_type)
					return;
			}

			//paint with terrain
			if(terrain_type!=1)
				mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].terrain_type=terrain_type;
			//paint with no random object
			else{
				if(mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items==1)
					mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items=0;
				else mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items=1;
			}

			//ask flood fill for all nearby terrains
			paint(x_grid-1,y_grid,terrain_type,flood_fill);
			paint(x_grid+1,y_grid,terrain_type,flood_fill);
			paint(x_grid,y_grid-1,terrain_type,flood_fill);
			paint(x_grid,y_grid+1,terrain_type,flood_fill);
		}
		else{

			//paint with terrain
			if(terrain_type!=1)
				mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].terrain_type=terrain_type;
			//paint with no random object
			else{
				/*if(mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items==1)
					mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items=0;
				else mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items=1;*/
				if(start_norandom==1)
					mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items=0;
				else mod_to_edit->terrain_maps[edited_area].terrain_grid[y_grid].terrain_blocks[x_grid].no_random_items=1;

			}
		}
	}
}

void Editor::center_map(void){
	camera_x=(mod_to_edit->terrain_maps[edited_area].terrain_grid[0].terrain_blocks.size()*128*0.5f*zoom-screen_width/2);
	camera_y=(mod_to_edit->terrain_maps[edited_area].terrain_grid.size()*128*0.5f*zoom-screen_height/2);
}
