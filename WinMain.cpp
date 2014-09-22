//1433
//#include "..\grim_api\grim.h"
#include "main.h"

#include <iostream>

//#define RELEASE(x) {if (x) {(x)->Release(); (x)=NULL;}}
template<typename T>
inline void RELEASE(T*& x) {
    if (x) {
        x->Release();
        x = NULL;
    }
}

//#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
template<typename T>
inline void SAFE_DELETE_ARRAY(T*& p) {
    if (p) {
        delete[] p;
        p = NULL;
    }
}

#define WM_GRAPHNOTIFY  WM_APP + 1

using namespace std;

Engine *grim = NULL;
game_engine *engine;
//HHOOK hkb;
int bug1=0,bug2=0,bug3=0;//debugging variables
bool debugging=false;//debugging control

float turn_speed=0.009f*2;
float movement_speed=0.35f;

int general_creature_size=64;
int general_object_size=128;
string game_version="1.3.4.1";
#define AI_ANGER_REDUCE_SPEED 2.0f
#define AI_ANGER_INCREASE_SPEED 1.0f
#define CREATURE_FADE_OUT_SPEED 0.01f
int climate_override=-1;
int active_distance=15;

game_engine::game_engine() {
	sound_initialized=false;
}

game_engine::~game_engine() {
}

void game_engine::Screenshot(string& screenshot_name){
    string FileName;
    GetScreenshotFileName(FileName);
    /*
    strcpy(filename,FileName->data());
    strcpy(screenshot_name,FileName->data());*/
    screenshot_name=FileName;
    grim->System_SaveScreenshot(FileName);
}

void game_engine::GetScreenshotFileName(string& FileName)
{
    // search for first unused filename
    string buffer;
    for (int i = 0; i < 1000; i++)
    {
		ostringstream os;
		os << "shot" << i << ".bmp";
		buffer = os.str();

		if (!grim->File_Exists(buffer))
        {   break; }
    }

    // set FileName to the first unused filename
    FileName = buffer;
}

/*
LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode,WPARAM wParam,
                            LPARAM lParam)
{
    char ch;
    if (((DWORD)lParam & 0x40000000) &&(HC_ACTION==nCode))
    {
        if ((wParam==VK_SPACE)||(wParam==VK_RETURN)||(wParam>=0x2f ) &&(wParam<=0x100))
        {
			int a=1;

        }
    }

    LRESULT RetVal = CallNextHookEx( hkb, nCode, wParam, lParam );
    return  RetVal;
}*/
/*
BOOL __declspec(dllexport)__stdcall installhook()
{
    //f1=fopen("c:\\report.txt","w");
    //fclose(f1);
    hkb=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KeyboardProc,hins,0);

    return TRUE;
}*/

bool game_engine::Frame(void)
{
	if(!focus)return true;

	bool draw_messages=true;

	if(debug.debug_state[1]==1){
		FILE *fil;
		fil = fopen("debug_frame.txt","wt");
		if(fil){
			fclose(fil);
		}
	}

	debug.debug_output("Frame",1,1);

	/*//find frame time average
	float elapsed2=0;
	elapsed3[30]=grim->Timer_GetDelta()*1000.0f;
	for (i=0; i<30; i++){
		elapsed3[i]=elapsed3[i+1];
		elapsed2=elapsed2+elapsed3[i];
	}
	elapsed=elapsed2/30;*/

	long cur_time;        // current timer value
	//double time_span;         // time elapsed since last frame

	// read appropriate counter
	cur_time = grim->Time_GetTicks();

	// scale time value and save
	//elapsed=((cur_time-last_time)*time_factor*1000);


	/*elapsed=((cur_time-last_time)*time_factor*1000+elapsed2)/2.0f;

	if(elapsed>0)
		elapsed2=elapsed;*/

	resources.system_time=(cur_time)*time_factor;

	//find frame time average
	elapsed2=0;
	elapsed3[30]=((cur_time-last_time)*time_factor*1000);
	for (int i=0; i<30; i++){
		elapsed3[i]=elapsed3[i+1];
		elapsed2=elapsed2+elapsed3[i];
	}
	elapsed=(elapsed2/30.0f)*randDouble(0.99f,1.01f);

	//returning from alt-tab
	if(elapsed>250){
		elapsed=250;
	}

	// save new time reading for next pass through the loop
	last_time=cur_time;

	//elapsed=1000/grim->Timer_GetFPS();
	//if(elapsed<3)elapsed=3;
	//if(elapsed>30)elapsed=30;

	//mouse pointer
	//mousex=grim->Mouse_GetPosX();
	//mousey=grim->Mouse_GetPosY();

	//mouse
	mousestate2=mousestate;
	mousestate = grim->getMouseState();

	if(!input_override){
		mousex= mousex + (mousestate.lX+mousestate2.lX)*mouse_speed;
		mousey= mousey + (mousestate.lY+mousestate2.lY)*mouse_speed;
		mouse_wheel=mousestate.lZ;

		mouse_left2=mouse_left;
		mouse_left= mousestate.btn_left;
		mouse_right2=mouse_right;
		mouse_right= mousestate.btn_right;
		mouse_middle2=mouse_middle;
		mouse_middle=mousestate.btn_middle;

		if(mousex<0)mousex=0;
		if(mousey<0)mousey=0;
		if(mousex>screen_width)mousex=screen_width;
		if(mousey>screen_height)mousey=screen_height;
	}

	override_mousex= override_mousex + (mousestate.lX+mousestate2.lX)*mouse_speed;
	override_mousey= override_mousey + (mousestate.lY+mousestate2.lY)*mouse_speed;
	override_mouse_wheel=mousestate.lZ;

	override_mouse_left2=override_mouse_left;
	override_mouse_left=mousestate.btn_left;

	if(override_mousex<0)override_mousex=0;
	if(override_mousey<0)override_mousey=0;
	if(override_mousex>screen_width)override_mousex=screen_width;
	if(override_mousey>screen_height)override_mousey=screen_height;

	//input
	if(!input_override){

		//keys that have been clicked
		key_clicked[0]=false;//0=any key
		char a;
		for(a='a';a<='z';a++){
			key_down2[translate_key_int(a)]=key_down[translate_key_int(a)];
			key_down[translate_key_int(a)]=grim->Key_Down(translate_key_KEY(a));
			key_clicked[translate_key_int(a)]=!key_down2[translate_key_int(a)]&&key_down[translate_key_int(a)];
		}
		for(a='0';a<='9';a++){
			key_down2[translate_key_int(a)]=key_down[translate_key_int(a)];
			key_down[translate_key_int(a)]=grim->Key_Down(translate_key_KEY(a));
			key_clicked[translate_key_int(a)]=!key_down2[translate_key_int(a)]&&key_down[translate_key_int(a)];
		}

		key_escape2=key_escape;
		key_escape=grim->Key_Down(KEY_ESCAPE);
		key_enter2=key_enter;
		key_enter=grim->Key_Down(KEY_RETURN);
		key_x2=key_x;
		key_x=grim->Key_Down(KEY_X);
		key_i2=key_i;
		key_i=grim->Key_Down(KEY_I);
		key_f2=key_f;
		key_f=grim->Key_Down(KEY_F);
		key_p2=key_p;
		key_p=grim->Key_Down(KEY_P);
		key_f52=key_f5;
		key_f5=grim->Key_Down(KEY_F5);
		key_f92=key_f9;
		key_f9=grim->Key_Down(KEY_F9);
		key_f122=key_f12;
		key_f12=grim->Key_Down(KEY_F12);
		key_j2=key_j;
		key_j=grim->Key_Down(KEY_J);
		key_b2=key_b;
		key_b=grim->Key_Down(KEY_B);
		key_d2=key_d;
		key_d=grim->Key_Down(KEY_D);
		key_e2=key_e;
		key_e=grim->Key_Down(KEY_E);
	}

	//override
	override_key_escape2=override_key_escape;
	override_key_escape=grim->Key_Down(KEY_ESCAPE);
	override_key_enter2=override_key_enter;
	override_key_enter=grim->Key_Down(KEY_RETURN);

	//input_override=false;
	//_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//render
	switch (game_state){
		case 0://game
	//_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF |_CRTDBG_CHECK_ALWAYS_DF);
			debug.debug_output("Render Map",1,1);
			render_map();
			//make sure we don't draw messages if we are going to load the game
			if(game_state!=0)
				draw_messages=false;
			debug.debug_output("Render Map",0,1);
			break;
		case 1://start window
			//grim->System_ClearScreen(0,0,0,1);
			draw_messages=false;

			//initialize timer
			{
				// no performance counter, read in using timeGetTime
				last_time=grim->Time_GetTicks();
				// clear timer selection flag
				perf_flag=false;
				// set timer scaling factor
				time_factor=0.001;
			}

			for (int i=0; i<30; i++){
				elapsed3[i]=5;
			}


			quit_game=false;

			//hkb=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KeyboardProc,NULL,0);

			//initialize texture loader
			resources.initialize_resource_handler(grim,&debug,g_pSoundManager,play_sound, sound_initialized);
			resources.system_time=(cur_time)*time_factor;


			//custom font
			font=resources.load_texture("font.png","Default");
			text_manager.default_font=font;

			//line texture
			line_texture=resources.load_texture("viiva.png","Default");
			text_manager.line_texture=line_texture;

			//loading screen
			presents_texture=resources.load_texture("presents.jpg","Default");
			grim->System_SetState_Blending(false);
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
			grim->Quads_SetRotation(0);
			resources.Texture_Set(presents_texture);
			grim->Quads_SetColor(1,1,1,1);
			grim->Quads_SetSubset(0,0,1,1);

			grim->Quads_Begin();
				grim->Quads_Draw(0, 0, screen_width,screen_height);
			grim->Quads_End();



			text_manager.write(font,"Loading...",1.7f,20, 20,0,0,false,1,1,1,1);
			game_state=2;
			return true;
			break;
		case 2://game starts
			draw_messages=false;
			initialize_game();

			create_menu_items();
			game_state=5;
			submenu=0;
			if(play_music)
				SwapSourceFilter("music/menu.mp3");
			break;

		case 3://puzzle
			//render_puzzle();
			break;

		case 4://animation
			draw_messages=false;
			render_animation();
			break;

		case 5://main menu
			draw_messages=false;
			debug.debug_output("Render Menu",1,1);
			render_menu();
			debug.debug_output("Render Menu",0,1);
			break;
		case 6://load game screen
			draw_messages=false;
			load_game(load_slot);
			game_state=0;
			play_music_file(-1,&last_played_music);
			break;
		case 7://new game screen
			draw_messages=false;
			new_game();
			break;
		case 8://credits
			draw_messages=false;
			render_credits();
			break;
		case 9://editor
			mouse_control_base mouse_controls;

			mouse_controls.mousex=mousex;
			mouse_controls.mousey=mousey;
			mouse_controls.mouse_left=mouse_left;
			mouse_controls.mouse_left2=mouse_left2;
			mouse_controls.mouse_right=mouse_right;
			mouse_controls.mouse_right2=mouse_right2;
			mouse_controls.mouse_middle=mouse_middle;
			mouse_controls.mouse_middle2=mouse_middle2;
			mouse_controls.mouse_wheel=mouse_wheel;

			if(!editor.render_map(elapsed,mouse_controls))
				game_state=5;


			draw_mouse(0,0,0,1,1,1);

			break;

	}

	//fps display
	if(debugging){
	    //TODO: make this use text stuff, no?
		grim->System_SetState_Blending(false);
		grim->Quads_SetColor(1,1,1,1);
//		grim->System_SetState(grSTATE_FONTSCALE,1.0f);
//		grim->Quads_RenderText(0.0f, screen_height-120+0.0f, itoa((int)(1000.0f/elapsed),temprivi,10));
    }


	//message system
	if(draw_messages)
	text_manager.draw_messages(elapsed);


	//print screen to file
	if (grim->Key_Click(KEY_PRINT )){
		Screenshot(screenshot_name);
		if(screenshot_name=="none"){
			tempstring="Screenshots not available in windowed mode";
			text_manager.message(5000,1000,tempstring);
		}
		else{
			tempstring="Screenshot saved as '";
			tempstring+=screenshot_name;
			tempstring+="'";
			text_manager.message(500,500,tempstring);
		}

	}

	//debugging
	if (grim->Key_Down(KEY_KP_SUBTRACT))
		debugging=false;
	if (grim->Key_Down(KEY_KP_ADD))
		debugging=true;

	//music event
	if(music_initialized){
		HandleGraphEvent();
	}

	debug.debug_output("Frame",0,1);

	//quit game
	if(quit_game){
		debug.debug_output("Game",0,0);
		debug.debug_output("Release Graphics Engine", 1,0);
		return false;
	}

	return true;
}

bool FrameFunc()
{
	return engine->Frame();
}

bool focuslost()
{

	engine->debug.debug_output("Focus Lost", 3,0);
	engine->focus=false;
	return true;
}

bool focusgained()
{

	engine->debug.debug_output("Focus Gained", 3,0);
//	if (engine->g_pMouse)
//		{
//		engine->g_pMouse->Acquire();
//		}

	engine->ask_continue_game=true;
	engine->focus=true;
	return true;
}

bool game_engine::cfg_load(void){
		FILE *fil;

		char temprivi[200];

		fil = fopen("cfg.cfg","rt");

		if (!fil) {
            return false;
		}

		fgets(temprivi,sizeof(temprivi),fil);screen_width=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);screen_height=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);bit_depth=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);windowed=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);play_sound=strtobool(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);play_music=strtobool(temprivi);

		fclose(fil);
        return true;
}

//bool game_engine::init_mouse(void){
//
//
//
//	HRESULT hr;
//	g_pMouse=NULL;
//	// Create the DirectInput object.
//    hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
//                           IID_IDirectInput8, (void**)&g_lpDI, NULL);
//    if FAILED(hr) return FALSE;
//
//
//			hr = g_lpDI->CreateDevice(GUID_SysMouse,
//					&g_pMouse, NULL);
//				if (FAILED(hr))
//					{
//						deinit_mouse();
//						return FALSE;
//					}
//				hr = g_pMouse->SetDataFormat(&c_dfDIMouse);
//				if (FAILED(hr))
//					{
//						deinit_mouse();
//						return FALSE;
//					}
//
//				HWND hWnd = GetFocus();
//				hr = g_pMouse->SetCooperativeLevel(hWnd,DISCL_EXCLUSIVE | DISCL_FOREGROUND);
//				//hr = g_pMouse->SetCooperativeLevel(hWnd,DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
//				if (FAILED(hr))
//					{
//						deinit_mouse();
//						return FALSE;
//					}
//
//				hr = g_pMouse->Acquire();
//				if (FAILED(hr))
//					{
//						deinit_mouse();
//						return FALSE;
//					}
//
//				return TRUE;
//}
//
//void game_engine::deinit_mouse(void){
//	if (g_pMouse)
//		{
//
//		//Always unacquire device before calling Release().
//
//				 g_pMouse->Unacquire();
//				 g_pMouse->Release();
//				 g_pMouse = NULL;
//		}
//
//}


//INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
int main(int argc, char* argv[])
{

	#ifdef _DEBUG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	//grim = Grim::Interface_Get("grim.dll");
	grim = new Engine();

	engine=new game_engine();
	//engine->hInst=hInst;
	grim->System_SetState_FrameFunc(FrameFunc);
	grim->System_SetState_FocusLostFunc(focuslost);
	grim->System_SetState_FocusGainFunc(focusgained);

	if (!engine->cfg_load()) {
        cerr << "Config file not found." << endl;
        delete engine;
        delete grim;
        return -1;
	}
	if(engine->windowed==0) {
        grim->System_SetState_Windowed(false);
    } else {
        grim->System_SetState_Windowed(true);
    }
	grim->System_SetState_ScreenWidth(engine->screen_width);
	grim->System_SetState_ScreenHeight(engine->screen_height);
	grim->System_SetState_ScreenBPP(engine->bit_depth);
	grim->System_SetState_Title("Notrium");

	//grim->System_SetState(grSTATE_OS_NATIVE_MOUSE,true);
	//grim->System_SetState(grSTATE_MOUSE,false);
//	grim->System_SetState(grSTATE_OS_NATIVE_MOUSE,false);
//	grim->System_SetState(grSTATE_MOUSE,false);
//	grim->System_SetState(grSTATE_DONTDOTHESTUPIDLOGO,true);

	//engine->initialize_game();


//	engine->g_pMouse=NULL;
	engine->game_state=1;//load screen
	engine->focus=true;

	//initiate graphics engine
	grim->System_Initiate(argv[0]);

	//initialize text manager
	engine->text_manager.initialize_text_output(grim,&engine->resources,engine->screen_width,engine->screen_height);


	grim->System_Start();

	//char *temp=grim->System_GetErrorMessage();


	grim->System_Shutdown();
	//grim->Inferface_Release();

	engine->uninitialize_game();

	SAFE_DELETE(engine);
	SAFE_DELETE(grim);


	return 0;
}


void game_engine::initialize_game(void){//initialize game variables


	/*m_pGA=NULL;*/
	map_main=NULL;
	map_storage.clear();
	/*for(int a=0;a<maximum_maps;a++){
		map_storage[a]=NULL;
	}*/


	/*//initialize timer
	{
		// is there a performance counter available?
		LONGLONG perf_cnt;
		if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)) {
			// yes, timer choice flag
			perf_flag=TRUE;
			// set scaling factor
			time_factor=1.0/perf_cnt;
			// read initial time
			QueryPerformanceCounter((LARGE_INTEGER *) &last_time);
		} else {
			// no performance counter, read in using timeGetTime
			last_time=GetTickCount();
			// clear timer selection flag
			perf_flag=FALSE;
			// set timer scaling factor
			time_factor=0.001;
		}
	}

	for (i=0; i<30; i++){
		elapsed3[i]=5;
	}*/

	game_difficulty_level=0;
	game_running=false;
	ask_quit=false;
	ask_continue_game=false;
	input_override=false;
	text_manager.accept_messages=true;
	//creature_think_distance=(sqr(screen_width*0.5f)+sqr(screen_height*0.5f))*1.3f;

	//frame time delta
	/*elapsed=15;
	for (i=0; i<30; i++){
		elapsed3[i]=15;
	}*/

	//load setup
	load_setup("data/setup.dat");

	if(debug.debug_state[0]==1){
		FILE *fil;
		fil = fopen("debug_start.txt","wt");
		if(fil){
			fprintf(fil, "Initializing Game\n");
			fclose(fil);
		}
	}


	//browse all mods
	mod.debug=&debug;
	mod.resources=&resources;
	load_mod_names(string("data"));
	mod.mod_name="Default";


	//initialize sound
	sound_initialized=false;
	if(play_sound){
		g_pSoundManager = new SoundManager();
		sound_init();
		playsound(intro_sound,0.6f,0,0,0,0);
	}

	//initialize music
	last_played_music=-1;
	music_initialized=false;
	if(play_music){
		music_initialized=GraphInit();
	}

    // FIXME this has a fixed aspect ratio
	x_multiplier=screen_width/1024.0f;
	y_multiplier=screen_height/768.0f;

	//new random seed
	random_seed++;
	srand(random_seed);

	//reset keys
	key_escape=false;
	key_escape2=false;
	key_enter=false;
	key_enter2=false;
	mousex=screen_width/2;
	mousey=screen_height/2;
	override_mousex=screen_width/2;
	override_mousey=screen_height/2;
	mouse_left=false;
	mouse_left2=false;
	mouse_right=false;
	mouse_right2=false;

	//engine->g_pMouse=NULL;
	grim->ShowCursor(false);
	//SetCursor(NULL);

	//load object infos + referred textures
/*	load_object_info("data/object_definitions.dat");
	load_climate_info("data/climate_types.dat");
	load_creature_info("data/creatures.dat");
	load_light_info("data/light.dat");
	load_weapon_info("data/weapons.dat");
	load_plot_object_info("data/plot_objects.dat");
	load_item_info("data/items.dat");
	load_animation_info("data/animation.dat");
	load_area_info("data/areas.dat");
	load_endgame_info("data/endings.dat");*/

	//extra textures
	mouse_texture[0]=resources.load_texture("mouse0.png","Default");
	mouse_texture[1]=resources.load_texture("mouse1.png","Default");
	mouse_texture[2]=resources.load_texture("mouse2.png","Default");
	mapdot=resources.load_texture("mapdot.png","Default");
	black_texture=resources.load_texture("black.bmp","Default");
	menu=resources.load_texture("menu.jpg","Default");
	bar_texture=resources.load_texture("bar.png","Default");
	carry_icon=resources.load_texture("carry_icon.png","Default");

}

void game_engine::uninitialize_game(void){//uninitialize game

	debug.debug_output("Release Graphics Engine", 0,0);

	debug.debug_output("Uninitializing game", 1,0);

	debug.debug_output("Saving setup", 1,0);
	save_setup("data/setup.dat");
	debug.debug_output("Saving setup", 0,0);

	//SAFE_DELETE(m_pGA);

	debug.debug_output("Clearing maps", 1,0);
	for(unsigned int a=0;a<map_storage.size();a++){
		SAFE_DELETE(map_storage[a]);
	}

	map_storage.clear();
	debug.debug_output("Clearing maps", 0,0);
	//SAFE_DELETE_ARRAY(*map_storage);
	//SAFE_DELETE(map_main);

//	debug.debug_output("Freeing mouse", 1,0);
//	deinit_mouse();
//	debug.debug_output("Freeing mouse", 0,0);


	if(sound_initialized){
		debug.debug_output("Uninitializing sounds", 1,0);
		resources.uninitialize();

		//SAFE_DELETE_ARRAY(*sample);
		SAFE_DELETE( g_pSoundManager );
		debug.debug_output("Uninitializing sounds", 0,0);

	}
//
//	if(play_music){
//		debug.debug_output("Uninitializing music", 1,0);
//		if (music)
//			music->Stop();
//
//        SAFE_DELETE( music );
//		// Release all remaining pointers
////		RELEASE( g_pSourceNext);
////		RELEASE( g_pSourceCurrent);
////		RELEASE( g_pMediaSeeking);
////		RELEASE( g_pMediaControl);
////		RELEASE( g_pGraphBuilder);
////		RELEASE( pEvent);
//
//		// Clean up COM
//		//CoUninitialize();
//		debug.debug_output("Uninitializing music", 0,0);
//	}

	debug.debug_output("Uninitializing game", 0,0);

}

void game_engine::render_map(void){//renders game map

	//slowdown
	if(debugging){
		//TODO: crossplatformify
/*		LONGLONG cur_time;        // current timer value
		float slow_factor=4*0.001f*20;
		QueryPerformanceCounter((LARGE_INTEGER *) &cur_time);
		if(cur_time-last_time<slow_factor/time_factor){
			//float time_to_do=10*plusmiinus-(time_now-time_at_this_frame);
			while(cur_time-last_time<slow_factor/time_factor){
				QueryPerformanceCounter((LARGE_INTEGER *) &cur_time);
			}
		}*/
	}

//	playsound(footstep[randInt(0,3)],1,player_middle_x+mousex-screen_width/2,player_middle_y+mousey-screen_height/2,player_middle_x,player_middle_y);

	//rearrange the item list
	debug.debug_output("Handle Item List",1,1);
	arrange_item_list(true);
	debug.debug_output("Handle Item List",0,1);

	paused=false;
	if(ask_quit)paused=true;//game paused by p
	if(ask_continue_game)paused=true;//paused after loading
	if(pop_up_mode==0)paused=true;//text showing
	if(pop_up_mode==3)paused=true;//item mode

	temp_speed=game_speed;
	if(paused){
		game_speed=0.000000f;//paused
	}

	//time from beginning
	if(!paused){
		time_from_beginning=time_from_beginning+elapsed*game_speed*0.001f;
	}

	light_level=0.25f;//for all lights
	creature_light_value=0.5f;//creatures in flashlight are darker

	//environmental sounds
	if(!paused){

		//night
		if((sincos.table_sin(day_timer/mod.general_races[player_race].day_speed*pi*2))<0){
			if(mod.general_climates[map_main->climate_number].night_sounds.size()>0)
			if(randInt(0,800/(elapsed*game_speed))==0){
				int sample_number=mod.general_climates[map_main->climate_number].night_sounds[randInt(0,mod.general_climates[map_main->climate_number].night_sounds.size())];
				playsound(sample_number,randDouble(0.3f,0.8f),randDouble(-500,500),randDouble(-500,500),0,0);
			}
		}
		//day
		else{
			if(mod.general_climates[map_main->climate_number].day_sounds.size()>0)
			if(randInt(0,800/(elapsed*game_speed))==0){
				int sample_number=mod.general_climates[map_main->climate_number].day_sounds[randInt(0,mod.general_climates[map_main->climate_number].day_sounds.size())];
				playsound(sample_number,randDouble(0.3f,1),randDouble(-500,500),randDouble(-500,500),0,0);
			}
		}
	}

	//quicksave game
	if(key_f12&&!key_f122){
		if((pop_up_mode==1)&&(!ask_continue_game)&&(!ask_quit)&&(!paused)){
			save_game(0);
		}
	}

	//quickload game
	if(key_f9&&!key_f92)
		if(pop_up_mode==1){
			draw_loading_screen();
			text_manager.write(font,"Loading game...",1.7f,20, 20,0,0,false,1,1,1,1);
			game_state=6;
			load_slot=0;
			return;
			//load_game(0);
		}

	//handle map changes
	if(!paused){
		handle_map_changed();
	}

	//scripts
	if(!paused)
	//if(time_from_beginning-scripts_calculated_on>0.5f)
	{
		debug.debug_output("Calculate Scripts",1,1);
		calculate_scripts();
		debug.debug_output("Calculate Scripts",0,1);
		//scripts_calculated_on=time_from_beginning;
	}

	float size=mod.general_creatures[map_main->creature[player_controlled_creature].type].size*map_main->creature[player_controlled_creature].size*general_creature_size;
	player_middle_x=map_main->creature[player_controlled_creature].x+size*0.5f;
	player_middle_y=map_main->creature[player_controlled_creature].y+size*0.5f;

	//camera
	//time elapses
	if(attach_camera_time>=0){
		attach_camera_time-=elapsed*game_speed;
		if(attach_camera_time<0){
			attach_camera_time=-1;
			attach_camera_type=0;
			attach_camera_parameter1=0;
			attach_camera_parameter2=0;
		}
	}

	//find suggested camera position
	float suggested_camera_x,suggested_camera_y;
	find_suggested_camera_position(&suggested_camera_x,&suggested_camera_y);

	//move camera location towards suggested
	//see if we would go overboard
	if(fabs(suggested_camera_x-real_camera_x)>fabs(suggested_camera_x-real_camera_x)*elapsed*game_speed*0.01f){
		float move=(suggested_camera_x-real_camera_x)*elapsed*game_speed*0.01f;
		real_camera_x+=move;
	}
	else{
		real_camera_x=suggested_camera_x;
	}
	//see if we would go overboard
	if(fabs(suggested_camera_y-real_camera_y)>fabs(suggested_camera_y-real_camera_y)*elapsed*game_speed*0.01f){
		float move=(suggested_camera_y-real_camera_y)*elapsed*game_speed*0.01f;
		real_camera_y+=move;
	}
	else{
		real_camera_y=suggested_camera_y;
	}

	float camera_shake_x=0;
	float camera_shake_y=0;

	if(!paused)
	if(screen_shake_time>0){
		screen_shake_time-=elapsed*game_speed;
		camera_shake_x=randDouble(-screen_shake_power,screen_shake_power);
		camera_shake_y=randDouble(-screen_shake_power,screen_shake_power);
	}

	camera_x=real_camera_x+camera_shake_x;
	camera_y=real_camera_y+camera_shake_y;

	bool x_moved=false;
	bool y_moved=false;
	if(camera_x<grid_size){
		if(!paused)
			mousex+=map_main->creature[player_controlled_creature].x-map_main->creature[player_controlled_creature].x2;
		camera_x=grid_size;
		x_moved=true;
	}
	if(camera_y<grid_size){
		if(!paused)
			mousey+=map_main->creature[player_controlled_creature].y-map_main->creature[player_controlled_creature].y2;
		camera_y=grid_size;
		y_moved=true;
	}
	if(camera_x>(map_main->sizex-2)*grid_size-screen_width){
		if(!paused)
			if(!x_moved)
				mousex+=map_main->creature[player_controlled_creature].x-map_main->creature[player_controlled_creature].x2;
		camera_x=(map_main->sizex-2)*grid_size-screen_width;
		x_moved=true;
	}
	if(camera_y>(map_main->sizey-2)*grid_size-screen_height){
		if(!paused)
			if(!y_moved)
				mousey+=map_main->creature[player_controlled_creature].y-map_main->creature[player_controlled_creature].y2;
		camera_y=(map_main->sizey-2)*grid_size-screen_height;
		y_moved=true;
	}

	//control input
	if(!paused){
		debug.debug_output("Player Controls",1,1);
		player_controls(player_control_type);
		calculate_quick_keys(false);
		debug.debug_output("Player Controls",0,1);
	}
	//flash_light();

	//which parts to draw
	int draw_size_x=(int)(screen_width/grid_size)+2;
	int draw_size_y=(int)(screen_height/grid_size)+2;
	int alku_x=(int)(camera_x/grid_size)-1;
	int alku_y=(int)(camera_y/grid_size)-1;
	int loppu_x=alku_x+draw_size_x;
	int loppu_y=alku_y+draw_size_y;

	//odd fix for smaller screen sizes
	if((screen_height==480)||(screen_height==600)){loppu_x+=1;loppu_y+=2;}
	if((screen_height==1200)||(screen_height==1600)){loppu_x+=1;loppu_y+=1;}

	//save which parts are drawn
	screen_start_x=alku_x;
	screen_start_y=alku_y;
	screen_end_x=loppu_x;
	screen_end_y=loppu_y;

	if(screen_start_x<0)screen_start_x=0;
	if(screen_start_y<0)screen_start_y=0;
	if(screen_end_x>map_main->sizex-1)screen_end_x=map_main->sizex-1;
	if(screen_end_y>map_main->sizey-1)screen_end_y=map_main->sizey-1;

	//check on which grid blocks the creatures are
	float check_time=0.15f/mod.general_creatures[map_main->creature[player_controlled_creature].type].movement_speed;
	if(check_time>0.25f)
		check_time=0.25f;
	if(!paused)
	if(fabs(time_from_beginning-creatures_checked_on)>check_time)
	{
		map_main->check_creatures();//t�t� voi tehd� v�h�n harvemmin jos haluaa
		creatures_checked_on=time_from_beginning+randDouble(0,check_time*0.25f);

	}

	//check if the creature is visible from player's point of view
	if(!paused)
	if(fabs(time_from_beginning-creature_visibility_checked_on)>0.15f){
		for(unsigned int a=0;a<map_main->creature.size();a++){

			//dead, show the body all the time
			if(map_main->creature[a].killed){
				map_main->creature[a].wall_between_creature_and_player=0;
				continue;
			}


			//if distance is very small, always check
			if(sqr(map_main->creature[a].x-player_middle_x)+sqr(map_main->creature[a].y-player_middle_y)<sqr(200)){
				map_main->creature[a].wall_between_creature_and_player=-1;
			}

			//if there was a wall on the last frame, immediately check if it's gone now
			else if(map_main->creature[a].wall_between_creature_and_player==1)
				map_main->creature[a].wall_between_creature_and_player=-1;

			//no wall on last frame, don't haste to check if there is one
			else if(map_main->creature[a].wall_between_creature_and_player==0){
				if(randInt(0,6)==0)
					map_main->creature[a].wall_between_creature_and_player=-1;
			}
		}
		creature_visibility_checked_on=time_from_beginning+randDouble(0,0.15f);
	}

	//AI
	if(!paused){
		debug.debug_output("AI",1,1);
		creature_AI();
		debug.debug_output("AI",0,1);
	}

	//grim->System_ClearScreen(1,1,1,1);

	//creature actions and animation
	if(!paused){
		debug.debug_output("Calculate Creatures",1,1);
		creature_actions_loop();
		debug.debug_output("Calculate Creatures",0,1);
	}

	//terrain timers
	if(!paused){
		for(unsigned int a=0;a<mod.terrain_types.size();a++){
			for(unsigned int b=0;b<mod.terrain_types[a].effects.size();b++){
				if(time_from_beginning-terrain_timers[a].subtype[b]>mod.terrain_types[a].effects[b].interval*0.001f){
					terrain_timers[a].subtype[b]=time_from_beginning;
				}
			}
		}
	}


	//bullets/attacks
	if(!paused){
		debug.debug_output("Calculate Bullets",1,1);
		calculate_bullets();
		debug.debug_output("Calculate Bullets",0,1);
	}


	/*//calculate map lighting
	debug.debug_output("Calculate Lights",1,1);
	calculate_lights();
	debug.debug_output("Calculate Lights",0,1);*/



	if(!paused){
		calculate_weather();
		//rain timers
		for(unsigned int a=0;a<mod.general_climates.size();a++){
			for(unsigned int b=0;b<mod.general_climates[a].rain_effects.size();b++){
				if(time_from_beginning-rain_effect_timers[a].subtype[b]>mod.general_climates[a].rain_effects[b].interval*0.001f){
					rain_effect_timers[a].subtype[b]=time_from_beginning;
				}
			}
		}
	}

	if(!paused){
		calculate_body_temperature();

		calculate_weight();

		count_bars();
	}

	//calculate particles
	if(!paused){
		debug.debug_output("Calculate Particles",1,1);
		calculate_particles();
		debug.debug_output("Calculate Particles",0,1);
	}

	//check items and objects
	mouse_on_item=false;
	if(!paused){
		debug.debug_output("Calculate Items",1,1);
		calculate_items();
		creature_nearest_to_the_mouse=-1;
		calculate_mouse_on_creatures();
		debug.debug_output("Calculate Items",0,1);
	}


	// clear the screen to black
	//grim->System_ClearScreen(0,0,0,0);

	//camera again
		player_middle_x=map_main->creature[player_controlled_creature].x+size*0.5f;
		player_middle_y=map_main->creature[player_controlled_creature].y+size*0.5f;

		/*camera_x=map_main->creature[0].x+size*0.5f-screen_width/2.0f+camera_shake_x;
		camera_y=map_main->creature[0].y+size*0.5f-screen_height/2.0f+camera_shake_y;

		if(camera_x<grid_size)camera_x=grid_size;
		if(camera_y<grid_size)camera_y=grid_size;
		if(camera_x>(map_main->sizex-1)*grid_size-screen_width-grid_size)camera_x=(map_main->sizex-1)*grid_size-screen_width-grid_size;
		if(camera_y>(map_main->sizey-1)*grid_size-screen_height-grid_size)camera_y=(map_main->sizey-1)*grid_size-screen_height-grid_size;
		*/

	//calculate map lighting
	debug.debug_output("Calculate Lights",1,1);
	calculate_lights();
	debug.debug_output("Calculate Lights",0,1);


	//draw map grid
	debug.debug_output("Draw Map Grid",1,1);
	draw_map_grid();
	debug.debug_output("Draw Map Grid",0,1);

	//particles below creatures - footsteps
	debug.debug_output("Draw Particles level 0",1,1);
	draw_particles(0);
	debug.debug_output("Draw Particles level 0",0,1);

	//draw objects below creatures-rocks
	debug.debug_output("Draw Map Objects level 0",1,1);
	draw_map_objects(0);
	debug.debug_output("Draw Map Objects level 0",0,1);

	//draw objects on top of rocks
	debug.debug_output("Draw Map Objects level 1",1,1);
	draw_map_objects(1);
	debug.debug_output("Draw Map Objects level 1",0,1);

	//lighting effects
	debug.debug_output("Draw Map Lights level 1",1,1);
	draw_lights(0);
	debug.debug_output("Draw Map Lights level 1",0,1);

	//draw creatures
	debug.debug_output("Draw Map Creatures layers 0 and 1",1,1);
	draw_map_creatures(0);//the dead
	draw_map_creatures(1);//living ones
	debug.debug_output("Draw Map Creatures layers 0 and 1",0,1);

	//draw targeting beam
	if(beam_active)//&&(!paused))
		draw_targeting_beam();

	//draw bullets
	debug.debug_output("Draw Bullets",1,1);
	draw_bullets();
	debug.debug_output("Draw Bullets",0,1);

	//particles on top of creatures
	debug.debug_output("Draw Particles level 1",1,1);
	draw_particles(1);
	debug.debug_output("Draw Particles level 1",0,1);

	//draw objects on top of creatures - trees
	debug.debug_output("Draw Map Objects level 2",1,1);
	draw_map_objects(2);
	debug.debug_output("Draw Map Objects level 2",0,1);
	//draw objects on top everything - big stuff
	debug.debug_output("Draw Map Objects level 3",1,1);
	draw_map_objects(3);
	debug.debug_output("Draw Map Objects level 3",0,1);


	debug.debug_output("Draw Map Creatures layer 2",1,1);
	draw_map_creatures(2);//flying ones
	debug.debug_output("Draw Map Creatures layer 2",0,1);

	//rain particles
	debug.debug_output("Draw particles level 2",1,1);
	draw_particles(2);
	debug.debug_output("Draw particles level 2",0,1);


	//draw pop-up text
	debug.debug_output("Draw Interface",1,1);
	if(!ask_quit)
	if(!ask_continue_game)
	draw_pop_up();
	debug.debug_output("Draw Interface",0,1);

	//draw bars
	if(!ask_quit)
	if(!ask_continue_game)
	if(pop_up_mode==1)
		draw_bars();

	/*//draw slider
	if(slider_active){
		mouse_left=mouse_left_temp;
		mouse_left2=mouse_left2_temp;
		mouse_right=mouse_right_temp;
		mouse_right2=mouse_right2_temp;
		draw_slider();
	}*/
	draw_slider();


	//draw_mouse
	if(mouse_visible){
		if(!mouse_on_item)
			draw_mouse(1,0.5f,0.5f,1,1,1);
		else{
			draw_mouse(2,0.5f,0.5f,1,1,1);
		}
	}
	else{
		mousex=screen_height/2;
		mousey=screen_width/2;
	}



	if(ask_quit){
		grim->Quads_SetColor(1,1,1,0.8f);
		text_manager.write_line(font,screen_width/2-171,screen_height/2-33,"Game paused",3);
		text_manager.write_line(font,screen_width/2-134,screen_height/2+18,"Press F10 to quit without saving",1);
		text_manager.write_line(font,screen_width/2-100,screen_height/2+38,"Press P to resume game",1);

		if(grim->Key_Down(KEY_F10))
			quit_game=true;
	}

	if(ask_continue_game){

		//continue by mouse
		if((!mouse_right&&mouse_right2)||(!override_mouse_left&&override_mouse_left2)){
			mouse_right2=false;
			override_mouse_left2=false;
			playsound(UI_game_click[1],1,0,0,0,0);
			ask_continue_game=false;
		}
		//advance by x or i or escape or enter
		if((override_key_enter&&!override_key_enter2)||(!override_key_escape&&override_key_escape2)){
			playsound(UI_game_click[1],1,0,0,0,0);
			ask_continue_game=false;
		}
		grim->Quads_SetColor(1,1,1,0.8f);
		text_manager.write_line(font,screen_width/2-171,screen_height/2-33,"Game Ready",3);
		text_manager.write_line(font,screen_width/2-104,screen_height/2+18,"Press Enter to continue",1);
	}

	//player wants to pause
	if(!key_p&&key_p2){
		playsound(UI_game_click[1],1,0,0,0,0);
		if(!ask_quit){
			ask_quit=true;
		}
		else{
			ask_quit=false;
		}
	}

	/*//save AI
	if(key_f5&&!key_f52){
		save_AI();
	}*/

	//day night cycle
	if(!paused){
		if(!slider_active){
			//if(day_timer==-1)
			//	day_timer=mod.general_races[player_race].day_speed;//start new day
			day_timer+=elapsed*game_speed*0.001f;//(time_from_beginning/day_speed-(int)(time_from_beginning/day_speed));
			if(day_timer>mod.general_races[player_race].day_speed)
			{
				day_timer-=mod.general_races[player_race].day_speed;

				//new day, time to show the journal and save precious AI data
				{
					day_number++;
					if(record_message(0,day_number)){//record the journal showing
						if(show_journals)
							show_text_window(journal_records.size()-1);
					}
				}
			}
		}
	}

	//to main menu
	if(!key_escape&&key_escape2){
		playsound(UI_menu_click[0],1,0,0,0,0);
		create_menu_items();
		game_state=5;
		submenu=0;
		if(play_music)
			SwapSourceFilter("music/menu/menu.mp3");
	}

	//return pause speed
	if(paused){
		game_speed=temp_speed;//paused
	}


}

void game_engine::draw_map_grid(void){//renders map grid

	grim->Quads_SetRotation(0);
	grim->Quads_SetSubset(0,0,1,1);
	grim->Quads_SetColor(1,1,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);

	/*border_visible=false;
	if((screen_start_x<0)||(screen_start_y<0)||(screen_end_x>map_main->sizex-1)||(screen_end_y>map_main->sizey-1))border_visible=true;
*/
	if(screen_start_x<0)screen_start_x=0;
	if(screen_end_x>map_main->sizex-1)screen_end_x=map_main->sizex-1;
	if(screen_start_y<0)screen_start_y=0;
	if(screen_end_y>map_main->sizey-1)screen_end_y=map_main->sizey-1;

	grim->System_SetState_Blending(false);
	/*for(k=0;k<mod.terrain_types.size();k++){
		//texture=mod.general_climates[map_main->climate_number].terrain_types[k].texture;
		texture=mod.terrain_types[k].texture;

		grim->Quads_Begin();*/
	int texture;
		for(int i=screen_start_x;i<screen_end_x;i++){
			for(int j=screen_start_y;j<screen_end_y;j++){
				//advance terrain frames
				if(mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames.size()>1){
					map_main->grid[i].grid[j].frame_time-=elapsed*game_speed;
					if(map_main->grid[i].grid[j].frame_time<0){
						map_main->grid[i].grid[j].frame_time=mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames[map_main->grid[i].grid[j].current_frame].time;
						map_main->grid[i].grid[j].current_frame++;
						if(map_main->grid[i].grid[j].current_frame>=mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames.size())
							map_main->grid[i].grid[j].current_frame=0;
					}
					/*if(time_from_beginning-map_main->grid[i].grid[j].frame_time>mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames[map_main->grid[i].grid[j].current_frame].time*0.001f){
						map_main->grid[i].grid[j].frame_time=time_from_beginning;
						map_main->grid[i].grid[j].current_frame++;
						if(map_main->grid[i].grid[j].current_frame>=mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames.size())
							map_main->grid[i].grid[j].current_frame=0;
					}*/
				}

				texture=mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames[map_main->grid[i].grid[j].current_frame].texture;
				resources.Texture_Set(texture);
				grim->Quads_Begin();

				//if(map_main->grid[i].grid[j].terrain_type==k){

				//draw
				grim->Quads_SetColorVertex(0, map_main->grid[i].grid[j].light_rgb[0], map_main->grid[i].grid[j].light_rgb[1], map_main->grid[i].grid[j].light_rgb[2], 1);
				grim->Quads_SetColorVertex(1, map_main->grid[(i+1)].grid[j].light_rgb[0], map_main->grid[(i+1)].grid[j].light_rgb[1], map_main->grid[(i+1)].grid[j].light_rgb[2], 1);
				grim->Quads_SetColorVertex(2, map_main->grid[(i+1)].grid[(j+1)].light_rgb[0], map_main->grid[(i+1)].grid[(j+1)].light_rgb[1], map_main->grid[(i+1)].grid[(j+1)].light_rgb[2], 1);
				grim->Quads_SetColorVertex(3, map_main->grid[i].grid[(j+1)].light_rgb[0], map_main->grid[i].grid[(j+1)].light_rgb[1], map_main->grid[i].grid[(j+1)].light_rgb[2], 1);

				grim->Quads_Draw(-camera_x+i*128, -camera_y+j*128, 128, 128);
				//}
				grim->Quads_End();
			}
		}
		/*grim->Quads_End();
	}*/

	//smooth slides in map grid
	grim->System_SetState_Blending(true);
	int alku_x=screen_start_x-1;
	int alku_y=screen_start_y-1;
	int loppu_x=screen_end_x+1;
	int loppu_y=screen_end_y+2;

	if(alku_x<1)alku_x=1;
	if(alku_y<1)alku_y=1;
	if(loppu_x>map_main->sizex-2)loppu_x=map_main->sizex-2;
	if(loppu_y>map_main->sizey-2)loppu_y=map_main->sizey-2;

	/*for(k=0;k<mod.terrain_types.size();k++){
		texture=mod.terrain_types[k].texture;
		resources.Texture_Set(texture);
		grim->Quads_Begin();*/
		for(int i=alku_x;i<loppu_x;i++){
			for(int j=alku_y;j<loppu_y;j++){
				int k = map_main->grid[i].grid[j].terrain_type;
				texture=mod.terrain_types[map_main->grid[i].grid[j].terrain_type].terrain_frames[map_main->grid[i].grid[j].current_frame].texture;
				resources.Texture_Set(texture);
				grim->Quads_Begin();
				//if(map_main->grid[i].grid[j].terrain_type==k){

					//lower square is not same
					if(map_main->grid[i].grid[(j+1)].terrain_type!=k){
						if((map_main->grid[i].grid[(j+1)].light_rgb[0]>0)||(map_main->grid[i].grid[(j+1)].light_rgb[1]>0)||(map_main->grid[i].grid[(j+1)].light_rgb[2]>0)
						 ||(map_main->grid[(i+1)].grid[(j+1)].light_rgb[0]>0)||(map_main->grid[(i+1)].grid[(j+1)].light_rgb[1]>0)||(map_main->grid[(i+1)].grid[(j+1)].light_rgb[2]>0)
							){
							grim->Quads_SetColorVertex(0, map_main->grid[i].grid[(j+1)].light_rgb[0], map_main->grid[i].grid[(j+1)].light_rgb[1], map_main->grid[i].grid[(j+1)].light_rgb[2], 0.5f);
							grim->Quads_SetColorVertex(1, map_main->grid[(i+1)].grid[(j+1)].light_rgb[0], map_main->grid[(i+1)].grid[(j+1)].light_rgb[1], map_main->grid[(i+1)].grid[(j+1)].light_rgb[2], 0.5f);
							grim->Quads_SetColorVertex(2, map_main->grid[(i+1)].grid[(j+2)].light_rgb[0], map_main->grid[(i+1)].grid[(j+2)].light_rgb[1], map_main->grid[(i+1)].grid[(j+2)].light_rgb[2], 0);
							grim->Quads_SetColorVertex(3, map_main->grid[i].grid[(j+2)].light_rgb[0], map_main->grid[i].grid[(j+2)].light_rgb[1], map_main->grid[i].grid[(j+2)].light_rgb[2], 0);
							grim->Quads_SetSubset(0,0,1,0.5f);
							grim->Quads_Draw(-camera_x+i*128, -camera_y+(j+1)*128, 128, 64);
						}
					}
					//upper square is not same
					if(map_main->grid[i].grid[(j-1)].terrain_type!=k){
						if((map_main->grid[i].grid[j].light_rgb[0]>0)||(map_main->grid[i].grid[j].light_rgb[1]>0)||(map_main->grid[i].grid[j].light_rgb[2]>0)
						 ||(map_main->grid[(i+1)].grid[j].light_rgb[0]>0)||(map_main->grid[(i+1)].grid[j].light_rgb[1]>0)||(map_main->grid[(i+1)].grid[j].light_rgb[2]>0)
							){
							grim->Quads_SetColorVertex(0, map_main->grid[i].grid[(j-1)].light_rgb[0], map_main->grid[i].grid[(j-1)].light_rgb[1], map_main->grid[i].grid[(j-1)].light_rgb[2], 0);
							grim->Quads_SetColorVertex(1, map_main->grid[(i+1)].grid[(j-1)].light_rgb[0], map_main->grid[(i+1)].grid[(j-1)].light_rgb[1], map_main->grid[(i+1)].grid[(j-1)].light_rgb[2], 0);
							grim->Quads_SetColorVertex(2, map_main->grid[(i+1)].grid[j].light_rgb[0], map_main->grid[(i+1)].grid[j].light_rgb[1], map_main->grid[(i+1)].grid[j].light_rgb[2], 0.5f);
							grim->Quads_SetColorVertex(3, map_main->grid[i].grid[j].light_rgb[0], map_main->grid[i].grid[j].light_rgb[1], map_main->grid[i].grid[j].light_rgb[2], 0.5f);
							grim->Quads_SetSubset(0,0.5f,1,1);
							grim->Quads_Draw(-camera_x+i*128, -camera_y+(j-1)*128+64, 128, 64);
						}
					}
					//left square is not same
					if(map_main->grid[(i-1)].grid[j].terrain_type!=k){
						if((map_main->grid[i].grid[j].light_rgb[0]>0)||(map_main->grid[i].grid[j].light_rgb[1]>0)||(map_main->grid[i].grid[j].light_rgb[2]>0)
						 ||(map_main->grid[i].grid[(j+1)].light_rgb[0]>0)||(map_main->grid[i].grid[(j+1)].light_rgb[1]>0)||(map_main->grid[i].grid[(j+1)].light_rgb[2]>0)
							){
							grim->Quads_SetColorVertex(0, map_main->grid[(i-1)].grid[j].light_rgb[0], map_main->grid[(i-1)].grid[j].light_rgb[1], map_main->grid[(i-1)].grid[j].light_rgb[2], 0);
							grim->Quads_SetColorVertex(1, map_main->grid[i].grid[j].light_rgb[0], map_main->grid[i].grid[j].light_rgb[1], map_main->grid[i].grid[j].light_rgb[2], 0.5f);
							grim->Quads_SetColorVertex(2, map_main->grid[i].grid[(j+1)].light_rgb[0], map_main->grid[i].grid[(j+1)].light_rgb[1], map_main->grid[i].grid[(j+1)].light_rgb[2], 0.5f);
							grim->Quads_SetColorVertex(3, map_main->grid[(i-1)].grid[(j+1)].light_rgb[0], map_main->grid[(i-1)].grid[(j+1)].light_rgb[1], map_main->grid[(i-1)].grid[(j+1)].light_rgb[2], 0);
							grim->Quads_SetSubset(0.5f,0,1,1);
							grim->Quads_Draw(-camera_x+(i-1)*128+64, -camera_y+j*128, 64, 128);
						}
					}
					//right square is not same
					if(map_main->grid[(i+1)].grid[j].terrain_type!=k){
						if((map_main->grid[(i+1)].grid[j].light_rgb[0]>0)||(map_main->grid[(i+1)].grid[j].light_rgb[1]>0)||(map_main->grid[(i+1)].grid[j].light_rgb[2]>0)
						 ||(map_main->grid[(i+1)].grid[(j+1)].light_rgb[0]>0)||(map_main->grid[(i+1)].grid[(j+1)].light_rgb[1]>0)||(map_main->grid[(i+1)].grid[(j+1)].light_rgb[2]>0)
							){
							grim->Quads_SetColorVertex(0, map_main->grid[(i+1)].grid[j].light_rgb[0], map_main->grid[(i+1)].grid[j].light_rgb[1], map_main->grid[(i+1)].grid[j].light_rgb[2], 0.5f);
							grim->Quads_SetColorVertex(1, map_main->grid[(i+2)].grid[j].light_rgb[0], map_main->grid[(i+2)].grid[j].light_rgb[1], map_main->grid[(i+2)].grid[j].light_rgb[2], 0);
							grim->Quads_SetColorVertex(2, map_main->grid[(i+2)].grid[(j+1)].light_rgb[0], map_main->grid[(i+2)].grid[(j+1)].light_rgb[1], map_main->grid[(i+2)].grid[(j+1)].light_rgb[2], 0);
							grim->Quads_SetColorVertex(3, map_main->grid[(i+1)].grid[(j+1)].light_rgb[0], map_main->grid[(i+1)].grid[(j+1)].light_rgb[1], map_main->grid[(i+1)].grid[(j+1)].light_rgb[2], 0.5f);
							grim->Quads_SetSubset(0,0,0.5f,1);
							grim->Quads_Draw(-camera_x+(i+1)*128, -camera_y+j*128, 64, 128);
						}
					}
				//}
                grim->Quads_End();
			}
		}
	/*	grim->Quads_End();
	}*/
}


void game_engine::draw_map_objects(int layer){//renders map grid
//return;
	int i,j,k;
	grim->Quads_SetSubset(0,0,1,1);
	grim->Quads_SetColor(1,1,1,1);
	grim->System_SetState_Blending(true);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	//drawing of the objects
	/*int alku_x=(int)(camera_x/grid_size)-3;
	int alku_y=(int)(camera_y/grid_size)-3;
	int loppu_x=alku_x+(int)(screen_width/grid_size)+6;
	int loppu_y=alku_y+(int)(screen_height/grid_size)+6;*/

	int alku_x=(int)(camera_x/grid_size)-3;
	int alku_y=(int)(camera_y/grid_size)-3;
	int loppu_x=alku_x+(int)(screen_width/grid_size)+6;
	int loppu_y=alku_y+(int)(screen_height/grid_size)+6;

	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;


	wind_direction_cos=sincos.table_cos(map_main->wind_direction);
	wind_direction_sin=sincos.table_sin(map_main->wind_direction);

	for(i=alku_x;i<loppu_x;i++){
		for(j=alku_y;j<loppu_y;j++){

			//props
			for(k=0;k<map_main->grid[i].grid[j].objects.size();k++){
				draw_object(&map_main->object[map_main->grid[i].grid[j].objects[k]],layer,map_main->object[map_main->grid[i].grid[j].objects[k]].x,map_main->object[map_main->grid[i].grid[j].objects[k]].y);
			}
			//items
			for(k=0;k<map_main->grid[i].grid[j].items.size();k++){
				//it's a plot_object
				if(map_main->items[map_main->grid[i].grid[j].items[k]].base_type==0)
					draw_object(&map_main->items[map_main->grid[i].grid[j].items[k]],layer,map_main->items[map_main->grid[i].grid[j].items[k]].x,map_main->items[map_main->grid[i].grid[j].items[k]].y);
				//it's a pickable item
				else
					draw_item(&map_main->items[map_main->grid[i].grid[j].items[k]],layer,map_main->items[map_main->grid[i].grid[j].items[k]].x,map_main->items[map_main->grid[i].grid[j].items[k]].y);
			}

		}
	}

}


bool game_engine::draw_object(map_object *object, int layer,float object_x, float object_y){

				//if on different layer, continue
				if(mod.general_objects[object->type].layer!=layer)return false;
				//dead
				if(object->dead)return false;

				//not visible
				if(!object->visible)return false;

				float sway_x=0;
				float sway_y=0;

				//if it's supposed to swing, swing it
				if(mod.general_objects[object->type].swing==1){
					float phase=sincos.table_sin(time_from_beginning+object->sway_phase);
					sway_x=wind_direction_cos*phase*object->sway_power*map_main->wind_speed;
					sway_y=wind_direction_sin*phase*object->sway_power*map_main->wind_speed;
				}

				float x=object_x+sway_x;
				float y=object_y+sway_y;
				float size=object->size*general_object_size;

				//advance animation frames
				if(mod.general_objects[object->type].animation_frames.size()>1){
					object->animation_frame_time-=elapsed*game_speed;
					if(object->animation_frame_time<0){
						object->animation_frame_time=mod.general_objects[object->type].animation_frames[object->current_animation_frame].time;
						object->current_animation_frame++;
						if(object->current_animation_frame>=mod.general_objects[object->type].animation_frames.size())
							object->current_animation_frame=0;
					}
				}

				//check if it's visible
				if(-camera_x+x<-size*1.415f){return false;}
				if(-camera_x+x>screen_width+size*0.415f){return false;}
				if(-camera_y+y<-size*1.415f){return false;}
				if(-camera_y+y>screen_height+size*0.415f){return false;}

				//trees are transparent near the player
				float transparency=1;
				float transparency2=0.00012f*(sqr(x+size*0.5f-player_middle_x)+sqr(y+size*0.5f-player_middle_y));
				if(mod.general_objects[object->type].get_transparent==1){
					transparency=transparency2;
					if(transparency<0.4f)transparency=0.4f;
					if(transparency>1)transparency=1;
				}
				//player is in shade
				if((mod.general_objects[object->type].provide_shade)&&(transparency2<1)){
					in_shade=true;
				}

				grim->Quads_SetColor(object->light_level[0],object->light_level[1],object->light_level[2],transparency);
				if(mod.general_objects[object->type].transparent==1)grim->System_SetState_Blending(true);
				else grim->System_SetState_Blending(false);
				int frame=object->current_animation_frame;
				resources.Texture_Set(mod.general_objects[object->type].animation_frames[frame].texture);
				grim->Quads_Begin();
				grim->Quads_SetRotation(object->rotation);
				grim->Quads_Draw(-camera_x+x, -camera_y+y, size, size);
				grim->Quads_End();

/*
				//collision lines
				if(mod.general_objects[object->type].collision_type==1){

					float object_size=object->size*general_object_size;
					float object_x=object->x+object_size*0.5f;
					float object_y=object->y+object_size*0.5f;


					//all the lines
					int a;
					if(mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()>1){
						float sin=sincos.table_sin(object->rotation);
						float cos=sincos.table_cos(object->rotation);
						for(a=0;a<mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()-1;a++){

							float xx1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].x*object_size*mod.general_objects[object->type].collision_parameter1;
							float yy1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].y*object_size*mod.general_objects[object->type].collision_parameter1;
							float xx2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].x*object_size*mod.general_objects[object->type].collision_parameter1;
							float yy2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].y*object_size*mod.general_objects[object->type].collision_parameter1;

							float x1=cos*xx1+sin*yy1+object_x;
							float y1=sin*xx1-cos*yy1+object_y;
							float x2=cos*xx2+sin*yy2+object_x;
							float y2=sin*xx2-cos*yy2+object_y;

							text_manager.draw_line(x1-camera_x,y1-camera_y,x2-camera_x,y2-camera_y,2,1,1,1,1,1);
						}
					}
				}*/

	return true;

}

bool game_engine::draw_item(map_object *object, int layer,float object_x, float object_y){

    //if on different layer, continue
    if(0!=layer)return false;
    //dead
    if(object->dead)return false;

    float size=object->size*general_object_size;

    //check if it's visible
    if(-camera_x+object_x<-size*1.415f){return false;}
    if(-camera_x+object_x>screen_width+size*0.415f){return false;}
    if(-camera_y+object_y<-size*1.415f){return false;}
    if(-camera_y+object_y>screen_height+size*0.415f){return false;}

    grim->Quads_SetColor(object->light_level[0],object->light_level[1],object->light_level[2],1);
    grim->System_SetState_Blending(true);
    resources.Texture_Set(mod.general_items[object->type].texture);
    grim->Quads_Begin();
    grim->Quads_SetRotation(object->rotation);
    grim->Quads_Draw(-camera_x+object_x, -camera_y+object_y, size, size);
    grim->Quads_End();

	return true;

}

void game_engine::load_particles(const string& filename){
	debug.debug_output("Load file "+filename,1,0);

	FILE *fil;
	char rivi[800];

	particles.clear();

	fil = fopen(filename.c_str(),"rt");
	if(fil){
		particle_base temp_particle;
		while(strcmp(stripped_fgets(rivi,sizeof(rivi),fil),"end_of_file")!=0){

			for(int b=0;b<3;b++){
				temp_particle.particles_list[b].clear();
			}

			temp_particle.name=rivi;
			debug.debug_output("Load "+temp_particle.name,1,0);
			temp_particle.identifier=atoi(stripped_fgets(rivi,sizeof(rivi),fil));

			//load texture if it's not loaded
			stripped_fgets(rivi,sizeof(rivi),fil);
			temp_particle.texture_name=rivi;
			temp_particle.texture=resources.load_texture(rivi,mod.mod_name);


			temp_particle.size=atof(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.r=atof(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.g=atof(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.b=atof(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.alpha=atof(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.gets_smaller_by_time=atof(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.can_be_rotated=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.stop_when_hit_ground=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
			temp_particle.blend_type=atoi(stripped_fgets(rivi,sizeof(rivi),fil));


			//if the identifier is bigger than current list size, increase list size
			temp_particle.dead=true;
			while(particles.size()<temp_particle.identifier+1){
				//put in dummy elements
				particles.push_back(temp_particle);
			}
			temp_particle.dead=false;
			particles[temp_particle.identifier]=temp_particle;

			debug.debug_output("Load "+temp_particle.name,0,0);
		}
	}

	fclose(fil);

	debug.debug_output("Load file "+filename,0,0);
}

void game_engine::load_sounds(const string& filename){
	debug.debug_output("Load file "+filename,1,0);

	FILE *fil;
	char rivi[2000];

	preloaded_sounds.clear();

	fil = fopen(filename.c_str(),"rt");
	if(fil){
		while(strcmp(stripped_fgets(rivi,sizeof(rivi),fil),"end_of_file")!=0){//name
			tempstring=rivi;
			debug.debug_output("Load "+tempstring,1,0);

			int index=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
			int sound_number=resources.load_sample(stripped_fgets(rivi,sizeof(rivi),fil),3,mod.mod_name);

			//if the identifier is bigger than current list size, increase list size
			while(preloaded_sounds.size()<index+1){
				//put in dummy elements
				preloaded_sounds.push_back(0);
			}
			preloaded_sounds[index]=sound_number;

			debug.debug_output("Load "+tempstring,0,0);
		}
	}

	fclose(fil);

	debug.debug_output("Load file "+filename,0,0);
}


void game_engine::draw_map_creatures(int layer){//draws the creatures

	unsigned int i,j,k;

	int creature;
	float x0,y0,x1,y1;
	float creature_x,creature_y;

	grim->Quads_SetColor(1,1,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);

	//minimum light for player
	float minimum_light=0.5f;
	if(map_main->creature[0].light[0]<minimum_light)map_main->creature[0].light[0]=minimum_light;
	if(map_main->creature[0].light[1]<minimum_light)map_main->creature[0].light[1]=minimum_light;
	if(map_main->creature[0].light[2]<minimum_light)map_main->creature[0].light[2]=minimum_light;

	grim->System_SetState_Blending(true);
	//find correct map squares of the objects
	int alku_x=(int)(camera_x/grid_size)-1;
	int alku_y=(int)(camera_y/grid_size)-1;
	int loppu_x=alku_x+(int)(screen_width/grid_size)+2;
	int loppu_y=alku_y+(int)(screen_height/grid_size)+2;

	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

	//go through all generic creature types
	/*for(a=0;a<total_mod.general_creatures;a++){
		resources.Texture_Set(mod.general_creatures[a].texture);
		grim->Quads_Begin();
*/
		for(i=alku_x;i<loppu_x;i++){
			for(j=alku_y;j<loppu_y;j++){

				//if(map_main->grid[i].grid[j].total_creatures>0)
				for(k=0;k<map_main->grid[i].grid[j].total_creatures;k++){

					creature=map_main->grid[i].grid[j].creatures[k];
					if(map_main->creature[creature].dead)continue;

					//check layer
					int creature_layer=mod.general_creatures[map_main->creature[creature].type].layer;
					if(map_main->creature[creature].killed)creature_layer=0;
					if(layer!=creature_layer)continue;

					//if((layer==0)&&(!map_main->creature[creature].killed))continue;
					//if((layer==1)&&(map_main->creature[creature].killed))continue;
					//if(map_main->creature[creature].type!=a)continue;

					float distance=sqr(map_main->creature[creature].x-map_main->creature[0].x)+sqr(map_main->creature[creature].y-map_main->creature[0].y);
					if(creature!=0)
					if(distance<sqr(300)){
						creature_base temp_creature=map_main->creature[creature];
					}

					//coordinates
					creature_x=map_main->creature[creature].x;
					creature_y=map_main->creature[creature].y;

					float size=mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size;

					//find if the creature is visible from the player's point of view
					if(!mod.general_creatures[map_main->creature[creature].type].hide_behind_walls){
						map_main->creature[creature].alpha=1;
					}
					else{
						if(creature!=0){
							bool visible_to_player=false;
							//obstacles
							//not calculated yet, find it
							if(map_main->creature[creature].wall_between_creature_and_player==-1){
								map_main->creature[creature].wall_between_creature_and_player=0;
								vector <point2d> collisions=line_will_collide(creature_x+size*0.5f,creature_y+size*0.5f,player_middle_x,player_middle_y,false,false,true,false,1,true,true);
								if(collisions.size()>1){
									map_main->creature[creature].wall_between_creature_and_player=1;
								}
							}
							if(map_main->creature[creature].wall_between_creature_and_player==0){
								visible_to_player=true;
							}

							if(visible_to_player){
								map_main->creature[creature].alpha+=elapsed*game_speed*CREATURE_FADE_OUT_SPEED*3;
								if(map_main->creature[creature].alpha>1)
									map_main->creature[creature].alpha=1;
							}
						}
					}

					//if the creature is not visible, we can just continue
					if(map_main->creature[creature].alpha==0)continue;


					//weapon effect visuals
					if(!paused)
					for(int a=0;a<map_main->creature[creature].weapon_effects_amount;a++){

						if(map_main->creature[creature].weapon_effects[a].weapon_type>=0){

							//particles
							if(mod.general_weapons[map_main->creature[creature].weapon_effects[a].weapon_type].special_effect_visual_particle>=0)
							if(randInt(0,100/(elapsed*game_speed))==0){
								make_particle(mod.general_weapons[map_main->creature[creature].weapon_effects[a].weapon_type].special_effect_visual_particle,1,randDouble(0,300),creature_x+randDouble(size*0.2f,size*0.8f),creature_y+randDouble(size*0.2f,size*0.8f),0,0);
								//if(randInt(0,2)==0)
								//playsound(electric[randInt(0,2)],0.7f,creature_x,creature_y,player_middle_x,player_middle_y);
								playsound(mod.general_weapons[map_main->creature[creature].weapon_effects[a].weapon_type].special_effect_sound,0.7f,creature_x,creature_y,player_middle_x,player_middle_y);
							}

							//color
							switch(mod.general_weapons[map_main->creature[creature].weapon_effects[a].weapon_type].special_effect_visual_color){
								//0=?
								case 0:
									/*
									map_main->creature[creature].light[0]=randInt(0,2);
									map_main->creature[creature].light[1]=map_main->creature[creature].light[0];
									map_main->creature[creature].light[2]=1;
									*/
									break;
								//1=red
								case 1:
									map_main->creature[creature].light[0]=randDouble(0.8f,1);
									map_main->creature[creature].light[1]=0.2f;
									map_main->creature[creature].light[2]=0.2f;
									break;
								//2=green
								case 2:
									map_main->creature[creature].light[0]=0.2f;
									map_main->creature[creature].light[1]=randDouble(0.8f,1);
									map_main->creature[creature].light[2]=0.2f;
									break;
								//3=blue
								case 3:
									map_main->creature[creature].light[0]=0.2f;
									map_main->creature[creature].light[1]=0.2f;
									map_main->creature[creature].light[2]=randDouble(0.8f,1);
									break;
							}
						}
					}

					//set light
					if(map_main->creature[creature].light[0]>1){
						float kerroin=(1.0f)/map_main->creature[creature].light[0];
						map_main->creature[creature].light[0]=map_main->creature[creature].light[0]*kerroin;
						map_main->creature[creature].light[1]=map_main->creature[creature].light[1]*kerroin;
						map_main->creature[creature].light[2]=map_main->creature[creature].light[2]*kerroin;
					}
					if(map_main->creature[creature].light[1]>1){
						float kerroin=(1.0f)/map_main->creature[creature].light[1];
						map_main->creature[creature].light[0]=map_main->creature[creature].light[0]*kerroin;
						map_main->creature[creature].light[1]=map_main->creature[creature].light[1]*kerroin;
						map_main->creature[creature].light[2]=map_main->creature[creature].light[2]*kerroin;
					}
					if(map_main->creature[creature].light[2]>1){
						float kerroin=(1.0f)/map_main->creature[creature].light[2];
						map_main->creature[creature].light[0]=map_main->creature[creature].light[0]*kerroin;
						map_main->creature[creature].light[1]=map_main->creature[creature].light[1]*kerroin;
						map_main->creature[creature].light[2]=map_main->creature[creature].light[2]*kerroin;
					}
					grim->Quads_SetColor(map_main->creature[creature].light[0],map_main->creature[creature].light[1],map_main->creature[creature].light[2],map_main->creature[creature].alpha);

					//for computer player's only
					if(creature>0){
						resources.Texture_Set(mod.general_creatures[map_main->creature[creature].type].texture);
						grim->Quads_Begin();

						//draw legs
						if(map_main->creature[creature].animation[2]>=0){
							find_texture_coordinates(map_main->creature[creature].animation[2],&x0,&y0,&x1,&y1,4);
							grim->Quads_SetSubset(x0,y0,x1,y1);
							grim->Quads_SetRotation(map_main->creature[creature].rotation_legs);
							grim->Quads_Draw(-camera_x+creature_x,
								-camera_y+creature_y,
								size,
								size
								);
						}
						//draw torso
						if(map_main->creature[creature].animation[1]>=0){
							int frame=map_main->creature[creature].animation[1];
							find_texture_coordinates(frame,&x0,&y0,&x1,&y1,4);
							grim->Quads_SetSubset(x0,y0,x1,y1);
							grim->Quads_SetRotation(map_main->creature[creature].rotation);
							grim->Quads_Draw(-camera_x+creature_x,
								-camera_y+creature_y,
								size,
								size
								);
						}
						//draw head
						if(map_main->creature[creature].animation[0]>=0){
							find_texture_coordinates(map_main->creature[creature].animation[0],&x0,&y0,&x1,&y1,4);
							grim->Quads_SetSubset(x0,y0,x1,y1);
							grim->Quads_SetRotation(map_main->creature[creature].rotation_head);
							grim->Quads_Draw(-camera_x+creature_x,
								-camera_y+creature_y,
								size,
								size
								);
						}
						grim->Quads_End();
					}

					//draw player
					else{

						//draw legs
						resources.Texture_Set(mod.general_creatures[map_main->creature[creature].type].texture);
						grim->Quads_Begin();
						if(map_main->creature[creature].animation[2]>=0){
							find_texture_coordinates(map_main->creature[creature].animation[2]-4,&x0,&y0,&x1,&y1,4);
							grim->Quads_SetSubset(x0,y0,x1,y1);
							grim->Quads_SetRotation(map_main->creature[creature].rotation_legs);
							grim->Quads_Draw(-camera_x+creature_x,
								-camera_y+creature_y,
								size,
								size
								);
						}
						grim->Quads_End();

						//draw torso
						int texture=mod.general_creatures[map_main->creature[creature].type].texture;
						int frame=6;
						if(mod.general_creatures[map_main->creature[0].type].weapon>=0){
							texture=mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].texture;
							if(map_main->creature[creature].animation[1]>=0){
								frame=map_main->creature[creature].animation[1]-1+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].frame*4;
							}
						}
						resources.Texture_Set(texture);
						grim->Quads_Begin();
							find_texture_coordinates(frame,&x0,&y0,&x1,&y1,4);
							grim->Quads_SetSubset(x0,y0,x1,y1);
							grim->Quads_SetRotation(map_main->creature[creature].rotation+(map_main->creature[creature].animation[2]-7)*0.04f);
							grim->Quads_Draw(-camera_x+creature_x,
								-camera_y+creature_y,
								size,
								size
								);
						grim->Quads_End();

						//draw head
						resources.Texture_Set(mod.general_creatures[map_main->creature[creature].type].texture);
						grim->Quads_Begin();
						if(map_main->creature[creature].animation[0]>=0){
							find_texture_coordinates(map_main->creature[creature].animation[0],&x0,&y0,&x1,&y1,4);
							grim->Quads_SetSubset(x0,y0,x1,y1);
							grim->Quads_SetRotation(map_main->creature[creature].rotation_head);
							grim->Quads_Draw(-camera_x+creature_x,
								-camera_y+creature_y,
								size,
								size
								);
						}
						grim->Quads_End();

					}

					//draw health meter
					int meters_drawn=0;
					float meter_x=-camera_x+creature_x;
					float meter_y=-camera_y+creature_y+meters_drawn*7;

					if(map_main->creature[creature].alpha>0.5f)
					if(mod.general_creatures[map_main->creature[creature].type].bars_visible)
					if(map_main->creature[creature].show_energy){

						for(int meter=0;meter<maximum_bars;meter++){
							if(!map_main->creature[creature].bars[meter].active)continue;
							if(mod.general_bars[meter].visible_on_enemies==0)continue;
							float value_range=map_main->creature[creature].bars[meter].maximum-map_main->creature[creature].bars[meter].minimum;
							float log_value=log10(map_main->creature[creature].bars[meter].maximum-map_main->creature[creature].bars[meter].minimum);
							float x2=(meter_x+(map_main->creature[creature].bars[meter].value/value_range-map_main->creature[creature].bars[meter].minimum/value_range)*log_value*15);
							float x3=(meter_x+log_value*15);
							meter_y=-camera_y+creature_y+meters_drawn*7;
							float y2=meter_y;
							text_manager.draw_line(meter_x,meter_y,x2,y2,5,0.9f,0.8f,0,1,0);
							if(x3>x2)
							text_manager.draw_line(x2,meter_y,x3,y2,5,0.8f,0.7f,1,0,0);

							meters_drawn++;
						}

					}

					//creature is carrying someone
					if(map_main->creature[creature].carried_creature>=0){
						grim->Quads_SetColor(1,1,1,0.5f);
						meter_y=-camera_y+creature_y+meters_drawn*7;
						grim->Quads_SetRotation(0);
						grim->Quads_SetSubset(0,0,1,1);
						float icon_x=meter_x;
						float icon_y=meter_y;
						resources.Texture_Set(carry_icon);
						grim->Quads_Begin();
							grim->Quads_Draw(icon_x,icon_y,16,16);
						grim->Quads_End();
						//text
						if((mousex>icon_x)&&(mousey>icon_y)&&(mousex<icon_x+16)&&(mousey<icon_y+16)){
							tempstring="Carrying: ";
							tempstring+=mod.general_creatures[map_main->creature[map_main->creature[creature].carried_creature].type].name;
							text_manager.write_line(font, mousex+28-16, mousey+10-16, tempstring, 1);
						}
					}

					//speech bubble
					if(!map_main->creature[creature].killed)
					if(map_main->creature[creature].dialog>=0){
						grim->Quads_SetRotation(0);
						float text_x=-camera_x+creature_x+size*0.8f+5;
						float text_y=-camera_y+creature_y+size*0.5f-6;
						text_manager.write(-1,mod.dialogs[map_main->creature[creature].dialog].text,1,text_x,text_y,text_x+300,screen_height-10,false,mod.dialogs[map_main->creature[creature].dialog].r,mod.dialogs[map_main->creature[creature].dialog].g,mod.dialogs[map_main->creature[creature].dialog].b,1);
					}
				}
			}
		}

}

//find 10 closest enemies, add them to list
//find all own side creatures (except player), add them to list
vector<int> game_engine::AI_list_thinkers(void){

	int i,j;
	int creature;

	vector<int> AI_thinkers;
	AI_thinkers.clear();

	const int ALIEN_THINKERS = 20;
	float shortest_distances[ALIEN_THINKERS];
	int closest_creatures[ALIEN_THINKERS];
	for(i=0;i<ALIEN_THINKERS;i++){
		shortest_distances[i]=1000000000;
		closest_creatures[i]=-1;
	}

	for(i=0;i<map_main->creature.size();i++){

		creature=i;


		if(map_main->creature[creature].dead)continue;

		//is dead
		if(map_main->creature[creature].killed){
			AI_thinkers.push_back(creature);
			continue;
		}

		//on alien attack list every creature
		if(alien_attack){
			AI_thinkers.push_back(creature);
			continue;
		}

		//forced AI
		if(map_main->creature[creature].force_AI){
			AI_thinkers.push_back(creature);
			continue;
		}

		//own side, add to list
		if(map_main->creature[creature].side==map_main->creature[0].side){
			AI_thinkers.push_back(creature);
			continue;
		}

		//visible, add to list
		if((map_main->creature[creature].x>camera_x-grid_size)&&(map_main->creature[creature].x<camera_x+screen_width+grid_size)
			&&(map_main->creature[creature].y>camera_y-grid_size)&&(map_main->creature[creature].y<camera_y+screen_height+grid_size)){

			AI_thinkers.push_back(creature);
			continue;
		}


		//for aliens calculate the distances
		map_main->creature[creature].distance=sqr(map_main->creature[creature].x-player_middle_x)+sqr(map_main->creature[creature].y-player_middle_y);

		//place distance in the shortest distance list
		//find if it can go to the list at all, and place it last
		if(map_main->creature[creature].distance<shortest_distances[ALIEN_THINKERS-1]){
			shortest_distances[ALIEN_THINKERS-1]=map_main->creature[creature].distance;
			closest_creatures[ALIEN_THINKERS-1]=creature;

			//now bubble that last item upwards
			for(j=ALIEN_THINKERS-2;j>=0;j--){
				if(shortest_distances[j+1]<shortest_distances[j]){
					//swap
					float temp_dist=shortest_distances[j];
					int temp_creat=closest_creatures[j];

					shortest_distances[j]=shortest_distances[j+1];
					closest_creatures[j]=closest_creatures[j+1];

					shortest_distances[j+1]=temp_dist;
					closest_creatures[j+1]=temp_creat;
				}
				else{
					break;
				}
			}
		}
	}

	//add to list
	for(i=0;i<ALIEN_THINKERS;i++){
		//do not add if does not exist
		if(closest_creatures[i]>=0)
			AI_thinkers.push_back(closest_creatures[i]);
	}

	return AI_thinkers;
}

//find nearest target
void game_engine::AI_find_target(int index,int *enemy_index,float *enemy_distance, int *friend_index,float *friend_distance){

	int i;



	*enemy_index=-1;
	*friend_index=-1;
	*enemy_distance=1000000000;
	*friend_distance=1000000000;
	float temp_dist;

	float creature_x=map_main->creature[index].x+mod.general_creatures[map_main->creature[index].type].size*map_main->creature[index].size*general_creature_size*0.5f;
	float creature_y=map_main->creature[index].y+mod.general_creatures[map_main->creature[index].type].size*map_main->creature[index].size*general_creature_size*0.5f;

	//if it's a player's friend, the friendly creature it follows is the player
	if(map_main->creature[0].side==map_main->creature[index].side){
		float creature_x2=map_main->creature[0].x+mod.general_creatures[map_main->creature[0].type].size*map_main->creature[0].size*general_creature_size*0.5f;
		float creature_y2=map_main->creature[0].y+mod.general_creatures[map_main->creature[0].type].size*map_main->creature[0].size*general_creature_size*0.5f;
		temp_dist=sqr(creature_x2-creature_x)+sqr(creature_y2-creature_y);
		*friend_index=0;
		*friend_distance=temp_dist;
	}

	for(i=AI_thinkers.size()-1;i>=0;i--){
		//int creature_2=map_main->grid[i].grid[j].creatures[k];
		int creature_2=AI_thinkers[i];
		if(creature_2==index)continue;//self

		//if(map_main->creature[creature_2].side==map_main->creature[index].side)continue;
		if(map_main->creature[creature_2].dead)continue;
		if(map_main->creature[creature_2].killed)continue;



		//find distance
		float creature_x2=map_main->creature[creature_2].x+mod.general_creatures[map_main->creature[creature_2].type].size*map_main->creature[creature_2].size*general_creature_size*0.5f;
		float creature_y2=map_main->creature[creature_2].y+mod.general_creatures[map_main->creature[creature_2].type].size*map_main->creature[creature_2].size*general_creature_size*0.5f;
		temp_dist=sqr(creature_x2-creature_x)+sqr(creature_y2-creature_y);

		//friend
		if(map_main->creature[creature_2].side==map_main->creature[index].side){

			//if it's the players friend, don't select a new friend
			if(map_main->creature[0].side!=map_main->creature[index].side){

				//only ones that can move, and are of higher rank
				//if((map_main->creature[friend_index].x!=map_main->creature[friend_index].x2)||(map_main->creature[friend_index].y!=map_main->creature[friend_index].y2))//only if the friend has moved
				if((mod.general_creatures[map_main->creature[creature_2].type].movement_speed!=0)&&(creature_2<index)){
					//no target yet
					if(*friend_index==-1){
						*friend_index=creature_2;
						*friend_distance=temp_dist;
						continue;
					}
					//target exists, find if this is closer	or the player
					else if(((*friend_index!=0)&&(temp_dist<*friend_distance))||(creature_2==0)){
						*friend_index=creature_2;
						*friend_distance=temp_dist;
					}
				}

			}
		}
		//enemy
		else
		if(!mod.AI_sides[map_main->creature[index].side].friend_with_side[map_main->creature[creature_2].side])
		{
			//not a threat, don't attack
			if((mod.general_creatures[map_main->creature[creature_2].type].weapon==0)&&(creature_2!=player_controlled_creature))continue;

			//no target yet
			if(*enemy_index==-1){
				//for player's friends, make sure they don't want to shoot through walls
				if(map_main->creature[index].side==map_main->creature[0].side){
					vector <point2d> collisions=line_will_collide(creature_x,creature_y,creature_x2,creature_y2,true,false,true,false,1,true,true);
					if(collisions.size()>0){
						continue;
					}
				}

				*enemy_index=creature_2;
				*enemy_distance=temp_dist;
				continue;
			}
			//target exists, find if this is closer
			else if(temp_dist<*enemy_distance){

				//for player's friends, make sure they don't want to shoot through walls
				if(map_main->creature[index].side==map_main->creature[0].side){
					vector <point2d> collisions=line_will_collide(creature_x,creature_y,creature_x2,creature_y2,true,false,true,false,1,true,true);
					if(collisions.size()>0){
						continue;
					}
				}



				*enemy_index=creature_2;
				*enemy_distance=temp_dist;
			}
		}
	}

	//*target=map_main->creature[index].target;
	//*target_distance=distance_closest;

}

void game_engine::AI_find_behavior_model(int creature, int *behavior, float *behavior_parameter0, bool *can_shoot, bool *can_eat){


	int a;
	//int tactic=mod.general_creatures[map_main->creature[creature].type].AI_tactic;
	int tactic=map_main->creature[creature].tactic[map_main->creature[creature].AI_order];
	if(tactic<mod.AI_tactics.size())
	for(a=0;a<mod.AI_tactics[tactic].levels.size();a++){

		//find correct level
		if(mod.AI_tactics[tactic].levels[a].start+mod.AI_tactics[tactic].levels[a].size>map_main->creature[creature].anger_level){
			*behavior_parameter0=mod.AI_tactics[tactic].levels[a].parameter0;
			*can_shoot=mod.AI_tactics[tactic].levels[a].can_shoot;
			*can_eat=mod.AI_tactics[tactic].levels[a].can_eat;
			*behavior=mod.AI_tactics[tactic].levels[a].action;
			return;
		}
	}

}


void game_engine::AI_act_behavior_model(int creature,int behavior, float parameter0,int enemy_index, float enemy_distance, float enemy_angle, int friend_index, float friend_distance){

	float creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
	float creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
	float enemy_x,enemy_y,friend_x,friend_y;
	if(enemy_index>=0){
		enemy_x=map_main->creature[enemy_index].x+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;
		enemy_y=map_main->creature[enemy_index].y+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;
	}
	if(friend_index>=0){
		friend_x=map_main->creature[friend_index].x+mod.general_creatures[map_main->creature[friend_index].type].size*map_main->creature[friend_index].size*general_creature_size*0.5f;
		friend_y=map_main->creature[friend_index].y+mod.general_creatures[map_main->creature[friend_index].type].size*map_main->creature[friend_index].size*general_creature_size*0.5f;
	}

	switch(behavior){
		//0=do nothing, good for crates
		case 0:
			map_main->creature[creature].move_to_x=creature_x;
			map_main->creature[creature].move_to_y=creature_y;
			break;
		//1=roam
		case 1:
			if(elapsed*game_speed>1)
				if(randInt(0,(3000)/(int)(elapsed*game_speed))==0){


					//move to random spot
					if(randInt(0,8)!=0){
						map_main->creature[creature].move_to_x=map_main->creature[creature].x+randDouble(-2000,2000);
						map_main->creature[creature].move_to_y=map_main->creature[creature].y+randDouble(-2000,2000);
					}

					//randomly go very near the enemy, only if player has killed a few aliens
					/*if(aliens_killed>5){
						if(randInt(0,8)!=0){
							map_main->creature[creature].move_to_x=enemy_x+randDouble(-1000,1000);
							map_main->creature[creature].move_to_y=enemy_y+randDouble(-1000,1000);
						}
					}*/

					if(map_main->creature[creature].move_to_x<0)map_main->creature[creature].move_to_x=map_main->sizex*general_object_size*0.5f;
					if(map_main->creature[creature].move_to_y<0)map_main->creature[creature].move_to_y=map_main->sizey*general_object_size*0.5f;
					if(map_main->creature[creature].move_to_x>map_main->sizex*general_object_size)map_main->creature[creature].move_to_x=map_main->sizex*general_object_size*0.5f;
					if(map_main->creature[creature].move_to_y>map_main->sizey*general_object_size)map_main->creature[creature].move_to_y=map_main->sizey*general_object_size*0.5f;



				}
			break;
		//2=follow friend, if distance to enemy is smaller than parameter0, attack
		case 2:
			if((enemy_index>=0 && friend_index>=0)){
				float enemy_distance_from_friend=sqr(friend_x-enemy_x)+sqr(friend_y-enemy_y);
				if(enemy_distance_from_friend<sqr(parameter0)){
					goto attack;
				}
			}
			if(friend_index>=0){//can two follow each other?
				if(friend_distance>sqr(2*mod.general_creatures[map_main->creature[friend_index].type].size*map_main->creature[friend_index].size*general_creature_size*0.5f+2*mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f)){
					map_main->creature[creature].move_to_x=friend_x;
					map_main->creature[creature].move_to_y=friend_y;
				}
				else{
					map_main->creature[creature].move_to_x=creature_x;
					map_main->creature[creature].move_to_y=creature_y;
				}
			}
			break;
		//3=attack
		case 3:
		attack:
			if((enemy_index>=0)){
				//melee combatants try to avoid going straight
				if(mod.general_creatures[map_main->creature[creature].type].close_combat){
					//find two points on enemy's sides
					float enemy_sin=sincos.table_sin(map_main->creature[enemy_index].rotation);
					float enemy_cos=sincos.table_cos(map_main->creature[enemy_index].rotation);
					float point_distance=sqrtf(enemy_distance)*0.3f;
					float point_x1=enemy_x+enemy_cos*point_distance;
					float point_y1=enemy_y+enemy_sin*point_distance;
					float point_x2=enemy_x-enemy_cos*point_distance;
					float point_y2=enemy_y-enemy_sin*point_distance;
					//make_particle(1,1,100,point_x1,point_y1,0,0);
					//make_particle(1,1,100,point_x2,point_y2,0,0);
					//pick the one that's closest
					if(sqr(creature_x-point_x1)+sqr(creature_y-point_y1)<sqr(creature_x-point_x2)+sqr(creature_y-point_y2)){
						map_main->creature[creature].move_to_x=point_x1;
						map_main->creature[creature].move_to_y=point_y1;
					}
					else{
						map_main->creature[creature].move_to_x=point_x2;
						map_main->creature[creature].move_to_y=point_y2;
					}

				}
				//gunfighters stop at a distance
				else{
					map_main->creature[creature].move_to_x=enemy_x;
					map_main->creature[creature].move_to_y=enemy_y;
					float bullet_distance=mod.general_weapons[mod.general_creatures[map_main->creature[creature].type].weapon].bullet_speed*mod.general_weapons[mod.general_creatures[map_main->creature[creature].type].weapon].time;
					if((enemy_distance<sqr(bullet_distance*0.6f))&&(enemy_angle<0.1f*randDouble(0,2))){
						map_main->creature[creature].move_to_x=creature_x;
						map_main->creature[creature].move_to_y=creature_y;
					}
				}


			}
			break;
		//4=flee
		case 4:
			if((enemy_index>=0)){
				map_main->creature[creature].move_to_x=map_main->creature[creature].x+(map_main->creature[creature].x-enemy_x);
				map_main->creature[creature].move_to_y=map_main->creature[creature].y+(map_main->creature[creature].y-enemy_y);

				if(map_main->creature[creature].move_to_x<0)map_main->creature[creature].move_to_x=map_main->sizex*general_object_size*0.5f;
				if(map_main->creature[creature].move_to_y<0)map_main->creature[creature].move_to_y=map_main->sizey*general_object_size*0.5f;
				if(map_main->creature[creature].move_to_x>map_main->sizex*general_object_size)map_main->creature[creature].move_to_x=map_main->sizex*general_object_size*0.5f;
				if(map_main->creature[creature].move_to_y>map_main->sizey*general_object_size)map_main->creature[creature].move_to_y=map_main->sizey*general_object_size*0.5f;
			}
			break;
		//5=guard place, parameter0=distance to attack enemies
		case 5:
			{
				//check if the guarded place is within the borders
				if(map_main->creature[creature].behavior_parameter0[behavior]>map_main->sizex*grid_size)
					map_main->creature[creature].behavior_parameter0[behavior]=randDouble(grid_size,(map_main->sizex-2)*grid_size);
				if(map_main->creature[creature].behavior_parameter1[behavior]>map_main->sizey*grid_size)
					map_main->creature[creature].behavior_parameter1[behavior]=randDouble(grid_size,(map_main->sizey-2)*grid_size);

				//by default always guard the place
				map_main->creature[creature].move_to_x=map_main->creature[creature].behavior_parameter0[behavior];
				map_main->creature[creature].move_to_y=map_main->creature[creature].behavior_parameter1[behavior];

				float enemy_distance_from_guard_place=sqr(map_main->creature[creature].behavior_parameter0[behavior]-enemy_x)+sqr(map_main->creature[creature].behavior_parameter1[behavior]-enemy_y);

				//enemy too close, attack
				if(enemy_index>=0){
					if(enemy_distance_from_guard_place<sqr(parameter0)){
						map_main->creature[creature].move_to_x=enemy_x;
						map_main->creature[creature].move_to_y=enemy_y;
					}
				}
			}
			break;
		//6=fire constantly in random directions
		case 6:
			//turn to random direction
			if(randInt(0,100/(elapsed*game_speed))==0){
				map_main->creature[creature].behavior_parameter0[behavior]=randDouble(0,2*pi);
			}
			map_main->creature[creature].move_to_x=creature_x+cosf(map_main->creature[creature].behavior_parameter0[behavior])*100;
			map_main->creature[creature].move_to_y=creature_y+sinf(map_main->creature[creature].behavior_parameter0[behavior])*100;

			map_main->creature[creature].fire=true;
			break;
	}


}

void game_engine::AI_initiate_behavior_parameters(creature_base* creature){


	int tactic=creature->tactic[creature->AI_order];

	int a;



	for(a=0;a<mod.AI_tactics[tactic].levels.size();a++){
		//reset behavior parameters
		switch(mod.AI_tactics[tactic].levels[a].action){
			//0=do nothing, good for crates
			case 0:
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=0;
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=0;
				break;
			//1=roam
			case 1:
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=0;
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=0;
				break;
			//2=follow friend
			case 2:
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=0;
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=0;
				break;
			//3=attack
			case 3:
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=0;
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=0;
				break;
			//4=flee
			case 4:
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=0;
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=0;
				break;
			//5=guard place, parameter0=distance to attack enemies
			case 5:
				//set current place to guard place
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=creature->x+mod.general_creatures[creature->type].size*creature->size*general_creature_size*0.5f;
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=creature->y+mod.general_creatures[creature->type].size*creature->size*general_creature_size*0.5f;
				break;
			//6=fire constantly in random directions
			case 6:
				creature->behavior_parameter0[mod.AI_tactics[tactic].levels[a].action]=randDouble(0,2*pi);
				creature->behavior_parameter1[mod.AI_tactics[tactic].levels[a].action]=0;
				break;
		}
	}

}

bool game_engine::AI_eat_behavior(int creature){

	//find if an eat item is close by, and head there if it is
	if(mod.general_creatures[map_main->creature[creature].type].eat_item>=0){
		int a,b,c;
		float creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
		float creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;

		int start_x=(int)(map_main->creature[creature].x/grid_size)-3;
		int start_y=(int)(map_main->creature[creature].y/grid_size)-3;
		int end_x=(int)(map_main->creature[creature].x/grid_size)+3;
		int end_y=(int)(map_main->creature[creature].y/grid_size)+3;

		if(start_x<0)start_x=0;
		if(end_x>map_main->sizex-1)end_x=map_main->sizex-1;
		if(start_y<0)start_y=0;
		if(end_y>map_main->sizey-1)end_y=map_main->sizey-1;

		int nearest_item=-1;
		float nearest_distance=0;

		for (a=start_x; a<end_x; a++){
			for (b=start_y; b<end_y; b++){
				for (c=0; c<map_main->grid[a].grid[b].items.size(); c++){
					if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
					if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=1)continue;
					if(map_main->items[map_main->grid[a].grid[b].items[c]].type==mod.general_creatures[map_main->creature[creature].type].eat_item){

						float distance=sqr(creature_x-(map_main->items[map_main->grid[a].grid[b].items[c]].x+map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size*0.5f))+sqr(creature_y-(map_main->items[map_main->grid[a].grid[b].items[c]].y+map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size*0.5f));

						if(nearest_item==-1){
							nearest_item=map_main->grid[a].grid[b].items[c];
							nearest_distance=distance;
						}
						else if(distance<nearest_distance){
							nearest_item=map_main->grid[a].grid[b].items[c];
							nearest_distance=distance;
						}
					}
				}
			}
		}

		//some item was found, head there
		if((nearest_item>=0)&&(nearest_distance<sqr(512))){
			map_main->creature[creature].move_to_x=map_main->items[nearest_item].x+map_main->items[nearest_item].size*general_object_size*0.5f;
			map_main->creature[creature].move_to_y=map_main->items[nearest_item].y+map_main->items[nearest_item].size*general_object_size*0.5f;

			//very near, eat it
			if(nearest_distance<100){
				//playsound(footstep[randInt(0,3)],randDouble(0.8f,1.0f),map_main->creature[creature].x,map_main->creature[creature].y,player_middle_x,player_middle_y);

				map_main->items[nearest_item].amount--;
				if(map_main->items[nearest_item].amount==0)map_main->items[nearest_item].dead=true;
				map_main->creature[creature].eat_counter++;

				//run creature eat effects
				for(c=0;c<mod.general_creatures[map_main->creature[creature].type].eat_block.size();c++){
					Mod::general_creatures_base::effect_block effect=mod.general_creatures[map_main->creature[creature].type].eat_block[c];

					//go throught all conditions
					bool OK=true;
					for(b=0;b<effect.conditions.size();b++){
						if(!check_condition(effect.conditions[b],&map_main->creature[creature],creature,creature_x,creature_y,false)){
							OK=false;
							break;
						}
					}
					if(!OK)continue;

					//run effects
					for(b=0;b<effect.effects.size();b++){
						run_effect(effect.effects[b],&map_main->creature[creature],creature,creature_x,creature_y,map_main->creature[creature].rotation,false);
					}
				}

			}
			return true;
		}
	}

	return false;
}

void game_engine::AI_fire_weapon(int creature, int enemy_index, float enemy_distance, float enemy_angle){

	//if target is within firing range and in right angle, fire
	if(mod.general_creatures[map_main->creature[creature].type].weapon>0)
	if(enemy_index!=-1){

		/*
		float creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
		float creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
		float enemy_x=map_main->creature[enemy_index].x+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;
		float enemy_y=map_main->creature[enemy_index].y+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;

		float distance_to_waypoint=sqr(creature_x-enemy_x)+sqr(creature_y-enemy_y);
		*/

		//close combat creature
		if(mod.general_creatures[map_main->creature[creature].type].close_combat){
			if((map_main->creature[creature].touched_enemy>=0)&&(fabs(enemy_angle)<mod.general_weapons[mod.general_creatures[map_main->creature[creature].type].weapon].spread+pi*0.75f)){

				map_main->creature[creature].fire=true;
				map_main->creature[creature].touched_enemy=-1;
			}
		}
		//ranged weapon creature
		else{
			float bullet_distance=mod.general_weapons[mod.general_creatures[map_main->creature[creature].type].weapon].bullet_speed*mod.general_weapons[mod.general_creatures[map_main->creature[creature].type].weapon].time;
			if((enemy_distance<sqr(bullet_distance))&&(fabs(enemy_angle)<mod.general_weapons[mod.general_creatures[map_main->creature[creature].type].weapon].spread+pi*0.05f)){
				bool can_fire=true;

				//if the target is the player, see if there are any walls in between
				if(enemy_index==0){
					//not calculated yet, find it
					if(map_main->creature[creature].wall_between_creature_and_player==-1){
						float creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
						float creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
						float enemy_x=map_main->creature[enemy_index].x+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;
						float enemy_y=map_main->creature[enemy_index].y+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;

						map_main->creature[creature].wall_between_creature_and_player=0;
						vector <point2d> collisions=line_will_collide(creature_x,creature_y,enemy_x,enemy_y,false,false,true,false,1,true,true);
							if(collisions.size()>1){
							map_main->creature[creature].wall_between_creature_and_player=1;
						}
					}
					if(map_main->creature[creature].wall_between_creature_and_player==1)
						can_fire=false;
				}

				if(can_fire)
					map_main->creature[creature].fire=true;
			}
		}
	}

}
void game_engine::AI_move(int creature){

	float creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
	float creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;

	//creatures that can move
	if(mod.general_creatures[map_main->creature[creature].type].movement_speed>0){
		if((creature_x!=map_main->creature[creature].move_to_x)||(creature_y!=map_main->creature[creature].move_to_y)){

			float alien_turn_speed=0.5f;
			map_main->creature[creature].rotation=map_main->creature[creature].rotation_legs;

			//collisions prevented in creature specialties
			bool prop_prevented=false;
			bool plot_object_prevented=false;
			for(int b=0;b<mod.general_creatures[map_main->creature[creature].type].specialties.size();b++){
				if(mod.general_creatures[map_main->creature[creature].type].specialties[b].number==4){
					if(mod.general_creatures[map_main->creature[creature].type].specialties[b].parameter3==1){
						prop_prevented=true;
					}
					if(mod.general_creatures[map_main->creature[creature].type].specialties[b].parameter0==1){
						plot_object_prevented=true;
					}
				}
			}

			//see if we need pathfinding
			float distance_to_waypoint=sqr(creature_x-map_main->creature[creature].move_to_x)+sqr(creature_y-map_main->creature[creature].move_to_y);
			bool need_pathfinding=true;
			if(distance_to_waypoint<sqr(500)){
				//see if the path is clear
				vector <point2d> collisions=line_will_collide(creature_x,creature_y,map_main->creature[creature].move_to_x,map_main->creature[creature].move_to_y,true,true,false,false,1,!prop_prevented,!plot_object_prevented);
				if(collisions.size()==0)
					need_pathfinding=false;

			}

			//far from the player, no pathfinding
			if(sqr(creature_x-player_middle_x)+sqr(creature_y-player_middle_y)>sqr(1500)){
				need_pathfinding=false;
			}

			//pathfinding
			if(need_pathfinding){
				float turn=AI_pathfinding(creature);
				if(turn<0)
					map_main->creature[creature].left=-alien_turn_speed*turn;
				else
					map_main->creature[creature].right=alien_turn_speed*turn;

				map_main->creature[creature].up=1.0f;
			}

			else{

				if(distance_to_waypoint>100){

					//find angle to target compared to creature
					float dq;
					dq=-atan2(creature_x-map_main->creature[creature].move_to_x,creature_y-map_main->creature[creature].move_to_y);

					float temp1=map_main->creature[creature].rotation_legs;
					if(map_main->creature[creature].rotation_legs-dq>pi)temp1=temp1-pi*2;
					if(dq-map_main->creature[creature].rotation_legs>pi)dq=dq-pi*2;
					float angle_to_waypoint=(float)fabs(dq-temp1);

					//if it's not already pointing to the right angle
					float alien_turn_speed=0.5f;
					if(mod.general_creatures[map_main->creature[creature].type].turn_speed!=0){
						if(angle_to_waypoint>turn_speed*elapsed*game_speed*alien_turn_speed*mod.general_creatures[map_main->creature[creature].type].turn_speed){
							//if the waypoint is to the left, turn left
							if(dq-temp1<0)
								map_main->creature[creature].left=alien_turn_speed;
							else
								map_main->creature[creature].right=alien_turn_speed;
						}
						else
							map_main->creature[creature].rotation_legs=dq;
					}

					//if angle is low enough, move forward
					if(angle_to_waypoint<1.4f){
						map_main->creature[creature].up=1.0f;
					}
				}
			}
		}
	}
	//creatures that cannot move
	else{
		if((creature_x!=map_main->creature[creature].move_to_x)||(creature_y!=map_main->creature[creature].move_to_y)){

			//a non-moving non-turning creature cannot do anything
			if(mod.general_creatures[map_main->creature[creature].type].turn_speed==0)return;

			//find distance to target
			float distance_to_waypoint=sqr(creature_x-map_main->creature[creature].move_to_x)+sqr(creature_y-map_main->creature[creature].move_to_y);
			if(distance_to_waypoint>100){

				//find angle to target compared to creature
				float dq;
				dq=-atan2(creature_x-map_main->creature[creature].move_to_x,creature_y-map_main->creature[creature].move_to_y);

				float temp1=map_main->creature[creature].rotation;
				if(map_main->creature[creature].rotation-dq>pi)temp1=temp1-pi*2;
				if(dq-map_main->creature[creature].rotation>pi)dq=dq-pi*2;
				float angle_to_waypoint=(float)fabs(dq-temp1);



				//if it's not already pointing to the right angle
				float alien_turn_speed=0.5f;



				if(angle_to_waypoint>turn_speed*elapsed*game_speed*alien_turn_speed*mod.general_creatures[map_main->creature[creature].type].turn_speed){
					//if the waypoint is to the left, turn left
					if(dq-temp1<0)
						map_main->creature[creature].rotation-=turn_speed*elapsed*game_speed*alien_turn_speed*mod.general_creatures[map_main->creature[creature].type].turn_speed;
					else
						map_main->creature[creature].rotation+=turn_speed*elapsed*game_speed*alien_turn_speed*mod.general_creatures[map_main->creature[creature].type].turn_speed;
				}
				else
					map_main->creature[creature].rotation=dq;
			}
		}
	}

}

//returns if the creature should go left, right, or straight
float game_engine::AI_pathfinding(int creature){
	int a,b;

	float target_x=map_main->creature[creature].move_to_x;
	float target_y=map_main->creature[creature].move_to_y;

	float creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
	float creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;

	int lines=3;
	float points[3]={0,0,0};
	float free_length[3]={0,0,0};
	float target_length[3]={0,0,0};


	float shortest_distance_to_target=0;
	int nearest_to_target=-1;

	//collisions prevented in creature specialties
	bool prop_prevented=false;
	bool plot_object_prevented=false;
	for(b=0;b<mod.general_creatures[map_main->creature[creature].type].specialties.size();b++){
		if(mod.general_creatures[map_main->creature[creature].type].specialties[b].number==4){
			if(mod.general_creatures[map_main->creature[creature].type].specialties[b].parameter3==1){
				prop_prevented=true;
			}
			if(mod.general_creatures[map_main->creature[creature].type].specialties[b].parameter0==1){
				plot_object_prevented=true;
			}
		}
	}

	float angle=pi/2;
	float line_length=100;
	for(a=0;a<lines;a++){
		//float temp=-angle/2+angle/(lines-1)*a;
		float angle_line=map_main->creature[creature].rotation-angle/2+angle/(lines-1)*a-pi/2;
		float end_x=creature_x+sincos.table_cos(angle_line)*line_length;
		float end_y=creature_y+sincos.table_sin(angle_line)*line_length;


		//make_particle(2,1,1000,creature_x,creature_y,0,0);
		//make_particle(1,1,1000,end_x,end_y,0,0);

		//check if this line will collide with obstacles
		vector <point2d> collisions=line_will_collide(creature_x,creature_y,end_x,end_y,false,true,false,false,1,!prop_prevented,!plot_object_prevented);
		//if not already on AI_avoid terrain, avoid such terrain
		if(!mod.terrain_types[map_main->grid[(int)(creature_x/general_object_size)].grid[(int)(creature_y/general_object_size)].terrain_type].AI_avoid){
			vector <point2d> collisions2=line_hazardous_terrain(creature_x,creature_y,end_x,end_y);
			for(b=0;b<collisions2.size();b++)collisions.push_back(collisions2[b]);
		}

		//find free length
		free_length[a]=sqr(line_length);
		for(b=0;b<collisions.size();b++){
			float distance=sqr(creature_x-collisions[b].x)+sqr(creature_y-collisions[b].y);
			if(distance<free_length[a])
				free_length[a]=distance;
		}

		//see distance to target
		float target_distance=sqr(end_x-target_x)+sqr(end_y-target_y);
		target_length[a]=target_distance;
		if((nearest_to_target==-1)||(shortest_distance_to_target>target_distance)){
			shortest_distance_to_target=target_distance;
			nearest_to_target=a;
		}

	}

	//now give points to lines

	//target is towards there
	for(a=0;a<lines;a++){
		points[a]+=-(target_length[a]/sqr(shortest_distance_to_target))*100;
	}


	//points for length
	for(a=0;a<lines;a++){
		points[a]+=(free_length[a]/sqr(line_length))*2-1;
	}




	//choose one with the maximum points
	int best=0;
	for(a=1;a<lines;a++){
		if(points[a]>points[best])
			best=a;
	}


	return ((float)best/lines*3-1);
}


void game_engine::creature_AI(void){

	int i;
	int b;
	int creature,enemy_index,friend_index;
	float enemy_distance,friend_distance;
	float creature_x,creature_y;
	//int creatures_hunting_player=0;

	//find 5 closest enemies, add them to list
	//find all own side creatures (except player), add them to list
	AI_thinkers=AI_list_thinkers();

	//with the creatures in the list do
	for(i=0;i<AI_thinkers.size();i++){

		creature=AI_thinkers[i];

		if(creature==0)continue;


		//is player controlled
		if(creature==player_controlled_creature)continue;

		//is dead
		if(map_main->creature[creature].killed){
			continue;
		}

		//reset controls
		map_main->creature[creature].left=false;
		map_main->creature[creature].right=false;
		map_main->creature[creature].up=false;
		map_main->creature[creature].down=false;
		map_main->creature[creature].fire=false;
		//map_main->creature[creature].sneak=false;

		//AI is prevented by a specialty
		int specialty;
		bool AI_prevented=false;
		for(specialty=0;specialty<mod.general_creatures[map_main->creature[creature].type].specialties.size();specialty++){
			if(mod.general_creatures[map_main->creature[creature].type].specialties[specialty].number==5){
				AI_prevented=true;
				break;
			}
		}
		if(AI_prevented)continue;

		//is stunned
		bool OK=true;
		for(b=0;b<map_main->creature[creature].weapon_effects_amount;b++){
			if(map_main->creature[creature].weapon_effects[b].effect_type==0)
			if(map_main->creature[creature].weapon_effects[b].parameter0==0){
				OK=false;
				break;
			}
		}
		if(!OK)continue;

		creature_x=map_main->creature[creature].x+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;
		creature_y=map_main->creature[creature].y+mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size*0.5f;

		//find target
		AI_find_target(creature,&enemy_index,&enemy_distance,&friend_index,&friend_distance);

		//find angle to enemy compared to creature
		float enemy_angle=10;
		float enemy_x,enemy_y;
		if(enemy_index>=0){
			enemy_x=map_main->creature[enemy_index].x+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;
			enemy_y=map_main->creature[enemy_index].y+mod.general_creatures[map_main->creature[enemy_index].type].size*map_main->creature[enemy_index].size*general_creature_size*0.5f;
			float dq;
			dq=-atan2(creature_x-enemy_x,creature_y-enemy_y);
			float temp1=map_main->creature[creature].rotation;
			if(map_main->creature[creature].rotation-dq>pi)temp1=temp1-pi*2;
			if(dq-map_main->creature[creature].rotation>pi)dq=dq-pi*2;
			enemy_angle=(float)fabs(dq-temp1);
		}
		//no enemy near, set anger to 0
		else{
			map_main->creature[creature].anger_level=0;
		}

		//find behavior model
		int behavior;
		float behavior_parameter0;
		bool can_shoot,can_eat;
		AI_find_behavior_model(creature,&behavior,&behavior_parameter0,&can_shoot,&can_eat);

		//eat
		bool will_eat=false;
		if(can_eat)
			will_eat=AI_eat_behavior(creature);

		//act behavior model, but only if not going to eat soon
		if(!will_eat)
			AI_act_behavior_model(creature,behavior,behavior_parameter0,enemy_index,enemy_distance,enemy_angle,friend_index,friend_distance);

		//fire weapon if can
		if(can_shoot)
			AI_fire_weapon(creature, enemy_index, enemy_distance,enemy_angle);

		//move towards target
		AI_move(creature);

		//anger level
		if(!alien_attack){
			if(enemy_index>=0){
				bool can_see_enemy=true;
				float anger_addition=0;
				float see_distance=mod.general_creatures[map_main->creature[creature].type].AI_see_range;
				float hear_distance=mod.general_creatures[map_main->creature[creature].type].AI_hear_range;
				float see_angle=mod.general_creatures[map_main->creature[creature].type].AI_see_angle;

				//if the target is the player, see if there are any walls in between
				if((see_distance>0)&&(see_angle>0))
				if(enemy_distance<sqr(see_distance))
				if(enemy_angle<see_angle*0.5f)
				if(enemy_index==0)
				{
					//not calculated yet, find it
					if(map_main->creature[creature].wall_between_creature_and_player==-1){
						map_main->creature[creature].wall_between_creature_and_player=0;
						vector <point2d> collisions=line_will_collide(creature_x,creature_y,enemy_x,enemy_y,false,false,true,false,1,true,true);
							if(collisions.size()>1){
							map_main->creature[creature].wall_between_creature_and_player=1;
						}
					}
					if(map_main->creature[creature].wall_between_creature_and_player==1)
						can_see_enemy=false;
				}


				//only if can see enemy (can only be false for player enemy)
				if(can_see_enemy){
					//creature has a flashlight
					if(map_main->creature[enemy_index].carry_light>=0){
						anger_addition+=2;
					}

					//creature just below the nose
					if(enemy_distance<sqr(see_distance*0.15f)){
						//is it in front
						if(enemy_angle<see_angle*0.2f){
							anger_addition+=30;
						}
					}


					//creature is in front and very very near
					if(enemy_distance<sqr(see_distance*0.25f)){
						//is it in front
						if(enemy_angle<see_angle*0.5f){
							anger_addition+=3;
						}
					}

					//creature is in front and very near
					if(enemy_distance<sqr(see_distance*0.5f)){
						//is it in front
						if(enemy_angle<see_angle*0.5f){
							anger_addition+=2;
						}
					}

					//creature is in front and not too far away
					if(enemy_distance<sqr(see_distance)){
						//is it in front
						if(enemy_angle<see_angle*0.25f){
							anger_addition+=1;
						}
					}
				}

				//hearing
				{
					//creature is in same area
					if(enemy_distance<sqr(hear_distance)){
						anger_addition+=1;

						/*//increase anger considerably for hearing a gunshot
						if(map_main->creature[enemy_index].fired_weapon_in_last_frame){
							anger_addition+=3000*mod.general_weapons[map_main->creature[enemy_index].weapon].AI_hear_volume;
						}*/
					}

					//creature is quite near
					if(enemy_distance<sqr(hear_distance*0.5f)){
						anger_addition+=1;
					}

					//creature is very near
					if(enemy_distance<sqr(hear_distance*0.2f)){
						anger_addition+=2;
					}
					//creature is very very near
					if(enemy_distance<sqr(hear_distance*0.05f)){
						anger_addition+=30;
					}
				}

				//reduce anger by random chance if far enough
				if(enemy_distance>sqr(hear_distance*0.2f)){
					if(randInt(0,10000/(elapsed*game_speed))==0){
						anger_addition-=3000;
					}
				}


				map_main->creature[creature].anger_level+=elapsed*game_speed*0.0001f*AI_ANGER_INCREASE_SPEED*anger_addition;
				if(map_main->creature[creature].anger_level>1)
					map_main->creature[creature].anger_level=1;
			}
		}
		else{
			map_main->creature[creature].anger_level=1;
		}


		//head
		map_main->creature[creature].rotation_head=map_main->creature[creature].rotation_legs;

	}

}


void game_engine::creature_actions_loop(void){

	int i;

	//int active_distance=15;

	/*int alku_x=(int)(camera_x/grid_size)-active_distance;
	int alku_y=(int)(camera_y/grid_size)-active_distance;
	int loppu_x=alku_x+(int)(screen_width/grid_size)+active_distance*2;
	int loppu_y=alku_y+(int)(screen_height/grid_size)+active_distance*2;*/
	int creature=1;
	bool visible=true;

/*	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;*/

	//some need to be calculated to all creatures
	for (creature=0; creature<map_main->creature.size(); creature++){

		map_main->creature[creature].touched_enemy=-1;

		//weapon effects
		for(int a=0;a<map_main->creature[creature].weapon_effects_amount;a++){

			//count time
			map_main->creature[creature].weapon_effects[a].time-=elapsed*game_speed;

			//effects on creature
			switch(map_main->creature[creature].weapon_effects[a].effect_type){
				//0=stun for time
				case 0:
					break;
				//1=charm target for time
				case 1:
					//time elapsed, return to normal side
					if(map_main->creature[creature].weapon_effects[a].time<=0){
						map_main->creature[creature].side=map_main->creature[creature].weapon_effects[a].parameter0;
					}
					break;
				//2=continuous damage parameter1 for time
				case 2:
					{
					float damage=map_main->creature[creature].weapon_effects[a].parameter0*elapsed*game_speed*0.001f;
					//map_main->creature[creature].bars[(int)map_main->creature[creature].weapon_effects[a].parameter1].value+=damage;
					Mod::effect temp_effect;
					temp_effect.effect_number=4;
					temp_effect.parameter1=damage;
					temp_effect.parameter2=0;
					temp_effect.parameter3=(int)map_main->creature[creature].weapon_effects[a].parameter1;
					temp_effect.parameter4=0;
					float size=mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size;
					run_effect(temp_effect,&map_main->creature[creature],creature,map_main->creature[creature].x+size*0.5f,map_main->creature[creature].y+size*0.5f,map_main->creature[creature].rotation,false);

					}
					break;
				//3=change type for time
				case 3:
					//time elapsed, return to normal side
					if(map_main->creature[creature].weapon_effects[a].time<=0){
						map_main->creature[creature].type=map_main->creature[creature].weapon_effects[a].parameter0;
					}
					break;
				//4=cannot fire weapon
				case 4:
					break;
				//5=continuous damage parameter1 for time, cap at maximum
				case 5:
					{
					float damage=map_main->creature[creature].weapon_effects[a].parameter0*elapsed*game_speed*0.001f;
					//map_main->creature[creature].bars[(int)map_main->creature[creature].weapon_effects[a].parameter1].value+=damage;
					Mod::effect temp_effect;
					temp_effect.effect_number=4;
					temp_effect.parameter1=damage;
					temp_effect.parameter2=1;
					temp_effect.parameter3=(int)map_main->creature[creature].weapon_effects[a].parameter1;
					temp_effect.parameter4=0;
					float size=mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size;
					run_effect(temp_effect,&map_main->creature[creature],creature,map_main->creature[creature].x+size*0.5f,map_main->creature[creature].y+size*0.5f,map_main->creature[creature].rotation,false);

					}
					break;
				//6=change AI for time
				case 6:
					//time elapsed, return to normal side
					if(map_main->creature[creature].weapon_effects[a].time<=0){
						//map_main->creature[creature].type=map_main->creature[creature].weapon_effects[a].parameter0;
						map_main->creature[creature].tactic[(int)map_main->creature[creature].weapon_effects[a].parameter0]=map_main->creature[creature].weapon_effects[a].parameter1;
						AI_initiate_behavior_parameters(&map_main->creature[creature]);
					}
					break;
			}

			//time elapsed, destroy effect
			if(map_main->creature[creature].weapon_effects[a].time<=0){
				//map_main->creature[creature].weapon_effects.erase(map_main->creature[creature].weapon_effects.begin()+a);
				//a--;
				for(int b=a+1;b<map_main->creature[creature].weapon_effects_amount+1;b++)
					map_main->creature[creature].weapon_effects[b-1]=map_main->creature[creature].weapon_effects[b];

				map_main->creature[creature].weapon_effects_amount--;
				continue;
			}
		}

		//return vanished creatures
		if(map_main->creature[creature].vanish_timer>0){
			map_main->creature[creature].vanish_timer-=elapsed*game_speed;
			if(map_main->creature[creature].vanish_timer<=0){
				map_main->creature[creature].dead=false;
				//set all creatures to be checked
				creatures_checked_on=0;
			}
		}
	}

	//main loop
	/*for(i=alku_x;i<loppu_x;i++){
		for(j=alku_y;j<loppu_y;j++){
			if(map_main->grid[i].grid[j].total_creatures>0)
				for(k=0;k<map_main->grid[i].grid[j].total_creatures;k++){
					creature=map_main->grid[i].grid[j].creatures[k];	*/

	for(i=0;i<AI_thinkers.size();i++){
		creature=AI_thinkers[i];
		if(creature<=-1)continue;

		visible=((map_main->creature[creature].x>camera_x-grid_size)&&(map_main->creature[creature].x<camera_x+screen_width+grid_size)
			&&(map_main->creature[creature].y>camera_y-grid_size)&&(map_main->creature[creature].y<camera_y+screen_height+grid_size));

		if(creature_actions(creature,visible)){
			for(int a=i+1;a<AI_thinkers.size();a++){
				if(AI_thinkers[a]>=creature)
					AI_thinkers[a]--;
			}
		}
	}

	/*//special endgame loop
	if(alien_attack)
		for (i=1; i<map_main->creature.size(); i++){
			creature=i;
			creature_actions(creature,true);
		}*/

}

bool game_engine::creature_actions(const int creature, const bool visible){//calculates the creature actions and animation

    //FIXME: it's strange that a creature right now can be in (NaN,NaN)

	/*
	float alien_speed_difficulty_modifier=0.0005f;
	if(game_difficulty_level==0)
		alien_speed_difficulty_modifier=0.000f;
	if(game_difficulty_level==2)
		alien_speed_difficulty_modifier=0.002f;*/

	//if(creature>0)return;

    creature_base& thiscreature = map_main->creature[creature];

    assert (!isnan(thiscreature.x));
    assert (!isnan(thiscreature.y));

	float bound_circle=14;
	int a,b,c;

		if(thiscreature.dead) return false;

		const float movement_change_speed=0.001f*5*mod.general_creatures[thiscreature.type].inertia;

		const float size=mod.general_creatures[thiscreature.type].size*thiscreature.size*general_creature_size;
        assert (size > 0);
		/*

				map_main->creature[creature].up=true;
				map_main->creature[creature].down=false;
				map_main->creature[creature].left=false;
				map_main->creature[creature].right=false;
				map_main->creature[creature].fire=false;
*/

		//visibility
		if((creature==player_controlled_creature)||(creature==0)){
			thiscreature.alpha=1;
		}
		else{
			thiscreature.alpha-=elapsed*game_speed*CREATURE_FADE_OUT_SPEED;
			if(thiscreature.alpha<0)thiscreature.alpha=0;
		}


		float distance=sqr(thiscreature.x-map_main->creature[0].x)+sqr(thiscreature.y-map_main->creature[0].y);
		if(distance<sqr(300)){
			creature_base temp_creature=thiscreature;
		}


		//death
		if(creature!=0)//only for non-player creatures
		if(!thiscreature.killed){
			bool die_now=false;

			//any bar too low, die
			for(int meter=0;meter<maximum_bars;meter++){
				if(!thiscreature.bars[meter].active)continue;
				if(thiscreature.bars[meter].value<=thiscreature.bars[meter].minimum)
					die_now=true;
			}

			//time is up
			if(mod.general_creatures[thiscreature.type].die_after_seconds>0){
				thiscreature.life_time+=elapsed*game_speed;
				if(thiscreature.life_time>mod.general_creatures[thiscreature.type].die_after_seconds*1000){
					die_now=true;

					//run creature death effects
					for(c=0;c<mod.general_creatures[thiscreature.type].death_block.size();c++){
						Mod::general_creatures_base::death_effect_block effect=mod.general_creatures[thiscreature.type].death_block[c];
						if(effect.death_type!=1)continue;//only on timed death

						//go throught all conditions
						bool OK=true;
						for(b=0;b<effect.conditions.size();b++){
							if(!check_condition(effect.conditions[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,false)){
								OK=false;
								break;
							}
						}
						if(!OK)continue;

						//run effects
						for(b=0;b<effect.effects.size();b++){
							run_effect(effect.effects[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,thiscreature.rotation,false);
						}
					}
				}
			}

			//die now
			if(die_now){
				//map_main->creature[creature].health=0;
				//if(map_main->creature[creature].side!=0)
				//	aliens_killed++;

				//run creature death effects
				for(c=0;c<mod.general_creatures[thiscreature.type].death_block.size();c++){
					Mod::general_creatures_base::death_effect_block effect=mod.general_creatures[thiscreature.type].death_block[c];

					if((effect.death_type==0)||((effect.death_type==2)&&(thiscreature.last_bullet_hit_from_creature_number==0))){//only on any death

						//go throught all conditions
						bool OK=true;
						for(b=0;b<effect.conditions.size();b++){
							if(!check_condition(effect.conditions[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,false)){
								OK=false;
								break;
							}
						}
						if(!OK)continue;

						//run effects
						for(b=0;b<effect.effects.size();b++){
							run_effect(effect.effects[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,thiscreature.rotation,false);
						}
					}
				}


				//disable light
				if(thiscreature.carry_light>=0)
					delete_light(map_main,thiscreature.carry_light);

				//die sound
				if(mod.general_creatures[thiscreature.type].die_sound.size()>0)
				playsound(mod.general_creatures[thiscreature.type].die_sound[randInt(0,mod.general_creatures[thiscreature.type].die_sound.size())],randDouble(0.8f,1.0f),thiscreature.x,thiscreature.y,player_middle_x,player_middle_y);

				//drop carried creature
				if(thiscreature.carried_creature>=0){
					map_main->creature[thiscreature.carried_creature].dead=false;

					float size_dropped=mod.general_creatures[map_main->creature[thiscreature.carried_creature].type].size*map_main->creature[thiscreature.carried_creature].size*general_creature_size;
					float place_x=thiscreature.x+size*0.5f-size_dropped*0.5f;
					float place_y=thiscreature.y+size*0.5f-size_dropped*0.5f;

					map_main->creature[thiscreature.carried_creature].x=place_x;
					map_main->creature[thiscreature.carried_creature].y=place_y;
					map_main->creature[thiscreature.carried_creature].rotation=thiscreature.rotation;
					map_main->creature[thiscreature.carried_creature].rotation_head=thiscreature.rotation;
					map_main->creature[thiscreature.carried_creature].rotation_legs=thiscreature.rotation;

					thiscreature.carried_creature=-1;

				}


				//corpse vanishes when dead
				/*if(mod.general_creatures[map_main->creature[creature].type].corpse_item==-2){
					//map_main->creature[creature].dead=true;
					delete_creature(map_main,creature);
					return;
				}*/
				//reduce anger level of all enemies around the victim
				for(a=0;a<AI_thinkers.size();a++){
					int creature2=AI_thinkers[a];
					if(creature2<=-1)continue;
					if(map_main->creature[creature2].side==thiscreature.side)continue;
					if(sqr(map_main->creature[creature2].x-thiscreature.x)+sqr(map_main->creature[creature2].y-thiscreature.y)>sqr(500))continue;
					map_main->creature[creature2].anger_level-=0.3f;
					if(map_main->creature[creature2].anger_level<0)
						map_main->creature[creature2].anger_level=0;
				}


				thiscreature.die();
			}
		}


		//timed effects block
		if(!thiscreature.killed)
		for(c=0;c<mod.general_creatures[thiscreature.type].timed_block.size();c++){
			Mod::general_creatures_base::timed_effect_block effect=mod.general_creatures[thiscreature.type].timed_block[c];
			if((time_from_beginning-thiscreature.script_timer[c])*1000<effect.interval)continue;

			thiscreature.script_timer[c]=time_from_beginning;

			//go throught all conditions
			bool OK=true;
			for(b=0;b<effect.conditions.size();b++){
				if(!check_condition(effect.conditions[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,false)){
					OK=false;
					break;
				}
			}
			if(!OK)continue;

			//run effects
			for(b=0;b<effect.effects.size();b++){
				run_effect(effect.effects[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,thiscreature.rotation,false);
			}
		}

		//speech
		if(thiscreature.dialog>=0){
			if(time_from_beginning-thiscreature.dialog_timer>mod.dialogs[thiscreature.dialog].duration*0.001f){
				thiscreature.dialog_timer=time_from_beginning;
				thiscreature.dialog=mod.dialogs[thiscreature.dialog].next_line;
			}
		}


		float sneak_multiplier=1;//*(map_main->creature[creature].up+map_main->creature[creature].down);

		//anger level
		if(thiscreature.anger_level>0){
			thiscreature.anger_level-=elapsed*game_speed*0.0001f*AI_ANGER_REDUCE_SPEED;
			if(thiscreature.anger_level<0)
				thiscreature.anger_level=0;
		}
		if(thiscreature.anger_level<0)
			thiscreature.anger_level=0;


		//weapon effects
		bool can_fire_weapon=true;
		for(a=0;a<thiscreature.weapon_effects_amount;a++){

			//effects on creature
			switch(thiscreature.weapon_effects[a].effect_type){
				//0=stun for time
				case 0:
					/*map_main->creature[creature].up=false;
					map_main->creature[creature].down=false;
					map_main->creature[creature].left=false;
					map_main->creature[creature].right=false;
					map_main->creature[creature].fire=false;*/
					sneak_multiplier=sneak_multiplier*thiscreature.weapon_effects[a].parameter0;
					break;
				//1=charm target for time
				case 1:
					break;
				//2=continuous damage parameter1 for time
				case 2:
					break;
				//3=change type for time
				case 3:
					break;
				//4=cannot fire weapon
				case 4:
					//check if the creature has such weapon type
					if((int)thiscreature.weapon_effects[a].parameter0<0){
						can_fire_weapon=false;
					}
					//individual weapons
					if((int)thiscreature.weapon_effects[a].parameter1==0){
						if(mod.general_creatures[thiscreature.type].weapon==(int)thiscreature.weapon_effects[a].parameter0)
						can_fire_weapon=false;
					}
					else if((int)thiscreature.weapon_effects[a].parameter1==1){
						if(mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].weapon_class==(int)thiscreature.weapon_effects[a].parameter0){
							can_fire_weapon=false;
						}
					}
					break;
				//5=continuous damage parameter1 for time, cap at maximum
				case 5:
					break;

			}
		}

		if(!can_fire_weapon){
			thiscreature.fire=false;
		}

		//dead - reset controls
		if(thiscreature.killed){
			thiscreature.left=false;
			thiscreature.right=false;
			thiscreature.up=false;
			thiscreature.down=false;
			thiscreature.fire=false;
			//map_main->creature[creature].sneak=false;
		}

		//map limits, end game creatures cannot hide out of sight
		const bool can_change_area = mod.general_creatures[thiscreature.type].can_change_area;
		const float left_map_limit= can_change_area ? 1 : (grid_size+4 );
		const float top_map_limit= can_change_area ? 1 : (grid_size+4);
		const float right_map_limit= can_change_area ? ((map_main->sizex-1)*(grid_size)-1) : (map_main->sizex*grid_size-grid_size*2-4);
		const float bottom_map_limit= can_change_area ? ((map_main->sizey-1)*(grid_size)-1) : (map_main->sizey*grid_size-grid_size*2-4);
//        float left_map_limit=1;
//		float top_map_limit=1;
//		float right_map_limit=(map_main->sizex-1)*(grid_size)-1;
//		float bottom_map_limit=(map_main->sizey-1)*(grid_size)-1;
//		if(!can_change_area){
//			left_map_limit=grid_size+4;
//			top_map_limit=grid_size+4;
//			right_map_limit=map_main->sizex*grid_size-grid_size*2-4;
//			bottom_map_limit=map_main->sizey*grid_size-grid_size*2-4;
//		}

        assert(!isnan(left_map_limit));
        assert(!isnan(top_map_limit));
        assert(!isnan(right_map_limit));
        assert(!isnan(bottom_map_limit));
        assert(left_map_limit < right_map_limit);
        assert(top_map_limit < bottom_map_limit);

		//fire
		if(!thiscreature.killed){
			if(thiscreature.fire_timer>=0)
				thiscreature.fire_timer-=elapsed*game_speed;
			if(mod.general_creatures[thiscreature.type].weapon<0)
				thiscreature.fire=false;

			if(thiscreature.fire&&can_fire_weapon){
				thiscreature.fire=false;
				if(thiscreature.fire_timer<0){
					thiscreature.animation_timer[1]=mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].fire_rate;
					thiscreature.fire_timer+=mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].fire_rate;


					float shoot_x, shoot_y;

					if(creature==0){
						shoot_x=size*0.5f+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].x*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation)+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].y*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation-pi/2);
						shoot_y=size*0.5f+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].x*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation)+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].y*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation-pi/2);
					}

					else{
						shoot_x=size*0.5f+mod.general_creatures[thiscreature.type].weapon_x*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation)+mod.general_creatures[thiscreature.type].weapon_y*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation-pi/2);
						shoot_y=size*0.5f+mod.general_creatures[thiscreature.type].weapon_x*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation)+mod.general_creatures[thiscreature.type].weapon_y*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation-pi/2);
					}

					bullet bullet_shot=shoot(creature, thiscreature.side,mod.general_creatures[thiscreature.type].weapon,thiscreature.x+shoot_x,thiscreature.y+shoot_y,thiscreature.rotation);

					//shooting was a success
					if(!bullet_shot.dead){
						//shooter falls back
						thiscreature.vx-=bullet_shot.move_x*mod.general_weapons[bullet_shot.type].push_shooter;
						thiscreature.vy-=bullet_shot.move_y*mod.general_weapons[bullet_shot.type].push_shooter;

						//increase anger level of all enemies around the shooter
						for(a=0;a<AI_thinkers.size();a++){
							int creature2=AI_thinkers[a];
							if(creature2<=-1)continue;

							if(creature==0)
								if(map_main->creature[creature2].side==thiscreature.side)continue;
							float hear_distance=mod.general_creatures[map_main->creature[creature2].type].AI_hear_range;
							if(sqr(map_main->creature[creature2].x-thiscreature.x)+sqr(map_main->creature[creature2].y-thiscreature.y)>sqr(hear_distance))continue;
							map_main->creature[creature2].anger_level+=0.3f*mod.general_weapons[bullet_shot.type].AI_hear_volume;
							if(map_main->creature[creature2].anger_level<0)
								map_main->creature[creature2].anger_level=0;
						}

						//increase anger for the shooter
						//map_main->creature[creature].anger_level+=0.2f;
					}
				}
			}
		}





		if(!thiscreature.killed){
			//make sure creature is on map again
			if(thiscreature.x<left_map_limit)thiscreature.x=left_map_limit;
			if(thiscreature.y<top_map_limit)thiscreature.y=top_map_limit;
			if(thiscreature.x>right_map_limit)thiscreature.x=right_map_limit;
			if(thiscreature.y>bottom_map_limit)thiscreature.y=bottom_map_limit;
            assert(!isnan(thiscreature.x));
            assert(!isnan(thiscreature.y));

			//calculate terrain effects
			int terrain_type=map_main->grid[(int)((thiscreature.x+size*0.5f)/grid_size)].grid[(int)((thiscreature.y+size*0.5f)/grid_size)].terrain_type;
			//go throught all effect blocks
			for(a=0;a<mod.terrain_types[terrain_type].effects.size();a++){

				if(time_from_beginning-terrain_timers[terrain_type].subtype[a]<mod.terrain_types[terrain_type].effects[a].interval*0.001f)continue;

				Mod::effect_base effect=mod.terrain_types[terrain_type].effects[a].effect;

				//check conditions
				bool OK=true;
				for(b=0;b<effect.conditions.size();b++){
					if(!check_condition(effect.conditions[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,false)){
						OK=false;
						break;
					}
				}
				if(!OK)continue;

				//run effects
				for(b=0;b<effect.effects.size();b++){
					run_effect(effect.effects[b],&thiscreature,creature,thiscreature.x+size*0.5f,thiscreature.y+size*0.5f,thiscreature.rotation,false);
				}

				if(creature==0)
				if(effect.event_text!="none"){
					//show normal message
					text_manager.message(mod.terrain_types[terrain_type].effects[a].interval+200,200,effect.event_text);
				}
			}

			//Mod::effect temp_effect;
			/*if(has_terrain_effect(map_main,terrain_type,2,&parameter0,&parameter1)){
				sneak_multiplier=sneak_multiplier*parameter0;
			}*/

			//is carrying creature, slow down
			if(thiscreature.carried_creature>=0){
				sneak_multiplier=sneak_multiplier*0.5f;//-(mod.general_creatures[map_main->creature[map_main->creature[creature].carried_creature].type].weight)*0.25f;
				//if(sneak_multiplier<0)sneak_multiplier=0;
				//turn_speed=turn_speed*(mod.general_creatures[map_main->creature[creature].type].weight+mod.general_creatures[map_main->creature[map_main->creature[creature].carried_creature].type].weight)*0.25f;

				//move carried creature
				map_main->creature[thiscreature.carried_creature].x=thiscreature.x;
				map_main->creature[thiscreature.carried_creature].y=thiscreature.y;
				map_main->creature[thiscreature.carried_creature].rotation=thiscreature.rotation;
				map_main->creature[thiscreature.carried_creature].rotation_head=thiscreature.rotation_head;
				map_main->creature[thiscreature.carried_creature].rotation_legs=thiscreature.rotation_legs;
			}

			//store location
			thiscreature.x2=thiscreature.x;
			thiscreature.y2=thiscreature.y;

			//turn
			//if(mod.general_creatures[map_main->creature[creature].type].movement_speed>0){
			//left
			if(thiscreature.left>0){
				thiscreature.rotation_legs-=turn_speed*elapsed*game_speed*thiscreature.left*mod.general_creatures[thiscreature.type].turn_speed;
				//map_main->creature[creature].rotation=map_main->creature[creature].rotation_legs;
			}
			//right
			if(thiscreature.right>0){
				thiscreature.rotation_legs+=turn_speed*elapsed*game_speed*thiscreature.right*mod.general_creatures[thiscreature.type].turn_speed;
				//map_main->creature[creature].rotation=map_main->creature[creature].rotation_legs;
			}
			//}
			//up
			if(thiscreature.up>0){
				thiscreature.backward_forward_speed+=movement_change_speed*elapsed*game_speed*sneak_multiplier;
				if(thiscreature.backward_forward_speed>1)thiscreature.backward_forward_speed=1;
			}
			//down
			if((thiscreature.down>0)||(thiscreature.up<0)){
				thiscreature.backward_forward_speed-=movement_change_speed*elapsed*game_speed*sneak_multiplier;
				if(thiscreature.backward_forward_speed<-1)thiscreature.backward_forward_speed=-1;
			}
			//not up or down
			if((thiscreature.up==0)&&(thiscreature.down==0)){
                // here's the bugger.
				if(fabs(thiscreature.backward_forward_speed) <= fabs(movement_change_speed*elapsed*game_speed))
					thiscreature.backward_forward_speed=0;
				else {
                    thiscreature.backward_forward_speed -= movement_change_speed*elapsed*game_speed
                            *fabs(thiscreature.backward_forward_speed)/thiscreature.backward_forward_speed;
 				}
				//thiscreature.backward_forward_speed=0;
			}

			//keep rotation in 0>x<2pi
			if(thiscreature.rotation_legs<0)thiscreature.rotation_legs+=pi*2;
			if(thiscreature.rotation_legs>2*pi)thiscreature.rotation_legs-=pi*2;
			if(thiscreature.rotation<0)thiscreature.rotation+=pi*2;
			if(thiscreature.rotation>2*pi)thiscreature.rotation-=pi*2;
			if(thiscreature.rotation_head<0)thiscreature.rotation_head+=pi*2;
			if(thiscreature.rotation_head>2*pi)thiscreature.rotation_head-=pi*2;

			//back and forward
			if((thiscreature.backward_forward_speed!=0)||(mod.general_creatures[thiscreature.type].minimum_movement_speed>0)){
                // !!!
				float summed_movement_speed = mod.general_creatures[thiscreature.type].movement_speed
                        * thiscreature.backward_forward_speed * sneak_multiplier;
				if(mod.general_creatures[thiscreature.type].minimum_movement_speed>0){
					if(summed_movement_speed < mod.general_creatures[thiscreature.type].minimum_movement_speed) {
						summed_movement_speed = mod.general_creatures[thiscreature.type].minimum_movement_speed;
                        assert(!isnan(summed_movement_speed));
					}
				}
                // !!!
				thiscreature.x+=sincos.table_cos(thiscreature.rotation_legs-pi/2)*elapsed*game_speed*summed_movement_speed*movement_speed;
				thiscreature.y+=sincos.table_sin(thiscreature.rotation_legs-pi/2)*elapsed*game_speed*summed_movement_speed*movement_speed;
                assert(!isnan(thiscreature.x));
                assert(!isnan(thiscreature.y));
			}

			//pushing
			if(mod.general_creatures[thiscreature.type].movement_speed>0){
				thiscreature.x+=thiscreature.vx*elapsed*game_speed/mod.general_creatures[thiscreature.type].weight;
				thiscreature.y+=thiscreature.vy*elapsed*game_speed/mod.general_creatures[thiscreature.type].weight;
                assert(!isnan(thiscreature.x));
                assert(!isnan(thiscreature.y));
			}

			if(thiscreature.vx>0){
				thiscreature.vx-=elapsed*game_speed*0.001f;
				if(thiscreature.vx<0)
					thiscreature.vx=0;
				//if(map_main->creature[creature].vx>0.5f)map_main->creature[creature].vx=0.5f;
			}
			if(thiscreature.vx<0){
				thiscreature.vx+=elapsed*game_speed*0.001f;
				if(thiscreature.vx>0)
					thiscreature.vx=0;
				//if(map_main->creature[creature].vx<-0.5f)map_main->creature[creature].vx=-0.5f;
			}
			if(thiscreature.vy>0){
				thiscreature.vy-=elapsed*game_speed*0.001f;
				if(thiscreature.vy<0)
					thiscreature.vy=0;
				//if(map_main->creature[creature].vy>0.5f)map_main->creature[creature].vy=0.5f;
			}
			if(thiscreature.vy<0){
				thiscreature.vy+=elapsed*game_speed*0.001f;
				if(thiscreature.vy>0)
					thiscreature.vy=0;
				//if(map_main->creature[creature].vy<-0.5f)map_main->creature[creature].vy=-0.5f;
			}

			//cap
			if(sqr(thiscreature.vx)+sqr(thiscreature.vy)>1){
				float kerroin=sqrt(sqr(thiscreature.vx)+sqr(thiscreature.vy));
				thiscreature.vx=thiscreature.vx/kerroin*1;
				thiscreature.vy=thiscreature.vy/kerroin*1;
			}

			//not too close to other creatures, they push each other
			if((visible)&&(!thiscreature.killed)){
				//make sure creature is on map again
				if(thiscreature.x<left_map_limit)thiscreature.x=left_map_limit;
				if(thiscreature.y<top_map_limit)thiscreature.y=top_map_limit;
				if(thiscreature.x>right_map_limit)thiscreature.x=right_map_limit;
				if(thiscreature.y>bottom_map_limit)thiscreature.y=bottom_map_limit;

                assert(!isnan(thiscreature.x));
                assert(!isnan(thiscreature.y));

				int start_x=(int)(thiscreature.x/grid_size)-2;
				int start_y=(int)(thiscreature.y/grid_size)-2;
				int end_x=(int)(thiscreature.x/grid_size)+2;
				int end_y=(int)(thiscreature.y/grid_size)+2;

				if(start_x<0)start_x=0;
				if(end_x>map_main->sizex-1)end_x=map_main->sizex-1;
				if(start_y<0)start_y=0;
				if(end_y>map_main->sizey-1)end_y=map_main->sizey-1;

				float bound_circle2=32;

				float creature_size=(bound_circle2)*(thiscreature.size*mod.general_creatures[thiscreature.type].size);
				float creature_x=thiscreature.x+creature_size*0.5f/(bound_circle2)*general_creature_size;
				float creature_y=thiscreature.y+creature_size*0.5f/(bound_circle2)*general_creature_size;

				for (a=start_x; a<end_x; a++){
					for (b=start_y; b<end_y; b++){
						if(map_main->grid[a].grid[b].total_creatures>0)
						for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
							if(map_main->grid[a].grid[b].creatures[c]==creature)continue;
							if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].dead)continue;
							if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].killed)continue;//killed
							if((map_main->creature[map_main->grid[a].grid[b].creatures[c]].side==map_main->creature[0].side)&&(creature==0))continue;//friends don't push player

							int creature2=map_main->grid[a].grid[b].creatures[c];

							//collisions prevented in creature specialties of either creature
							int specialty;
							bool creature1_prevented=false;
							for(specialty=0;specialty<mod.general_creatures[thiscreature.type].specialties.size();specialty++){
								if(mod.general_creatures[thiscreature.type].specialties[specialty].number==4){
									if(mod.general_creatures[thiscreature.type].specialties[specialty].parameter1==1){
										creature1_prevented=true;
									}
								}
							}
							if(creature1_prevented)continue;

							bool creature2_prevented=false;
							for(specialty=0;specialty<mod.general_creatures[map_main->creature[creature2].type].specialties.size();specialty++){
								if(mod.general_creatures[map_main->creature[creature2].type].specialties[specialty].number==4){
									if(mod.general_creatures[map_main->creature[creature2].type].specialties[specialty].parameter1==1){
										creature2_prevented=true;
									}
								}
							}
							if(creature2_prevented)continue;

							float creature_size2=(bound_circle2)*(map_main->creature[creature2].size*mod.general_creatures[map_main->creature[creature2].type].size);
							float creature_weight=mod.general_creatures[thiscreature.type].weight;
							float creature2_weight=mod.general_creatures[map_main->creature[creature2].type].weight;
							float creature_x2=map_main->creature[creature2].x+creature_size2*0.5f/(bound_circle2)*general_creature_size;
							float creature_y2=map_main->creature[creature2].y+creature_size2*0.5f/(bound_circle2)*general_creature_size;

							//at the same place
							if((creature_x==creature_x2)&&(creature_y==creature_y2)){
								creature_x2+=randDouble(-10,10);
								creature_y2+=randDouble(-10,10);
							}

							//for player, he can push his friends as he likes
							if(((creature==0)||(creature2==0))&&(thiscreature.side==map_main->creature[creature2].side)){
								//only if the creature can move

								if(creature==0){
									if(mod.general_creatures[map_main->creature[creature2].type].movement_speed>0){
										creature_weight=1000;
										creature2_weight=1;
									}
								}
								if(creature2==0){
									if(mod.general_creatures[thiscreature.type].movement_speed>0){
										creature2_weight=1000;
										creature_weight=1;
									}
								}
							}

							//the magic happens, push the creature
							if(sqr(creature_x-creature_x2)+sqr(creature_y-creature_y2)<sqr(creature_size*0.5f+creature_size2*0.5f)){

								//remember the touch
								if(map_main->creature[creature2].side!=thiscreature.side)
								if(!mod.AI_sides[thiscreature.side].friend_with_side[map_main->creature[creature2].side]){
									map_main->creature[creature2].touched_enemy=creature;
									thiscreature.touched_enemy=creature2;
								}

								//float kerroin=sqrtf(sqr((bound_circle2)*(mod.general_creatures[map_main->creature[creature].type].size+mod.general_creatures[map_main->creature[map_main->grid[a].grid[b].creatures[c]].type].size))/(sqr(map_main->creature[creature].x-map_main->creature[map_main->grid[a].grid[b].creatures[c]].x)+sqr(map_main->creature[creature].y-map_main->creature[map_main->grid[a].grid[b].creatures[c]].y)));
								float kerroin=((creature_size*0.5f+creature_size2*0.5f)/sqrtf(sqr(creature_x-creature_x2)+sqr(creature_y-creature_y2)));
								if(kerroin>2)kerroin=2;

								float move_x=((creature_x-creature_x2)*kerroin-(creature_x-creature_x2));
								float move_y=((creature_y-creature_y2)*kerroin-(creature_y-creature_y2));

								float kerroin_1=kerroin*(creature_weight/(creature_weight+creature2_weight));
								float kerroin_2=kerroin*(creature2_weight/(creature_weight+creature2_weight));

								thiscreature.x+=move_x*kerroin_2;
								thiscreature.y+=move_y*kerroin_2;
								assert(!isnan(thiscreature.x));
                                assert(!isnan(thiscreature.y));

								map_main->creature[creature2].x-=move_x*kerroin_1;
								map_main->creature[creature2].y-=move_y*kerroin_1;
                                assert(!isnan(map_main->creature[creature2].x));
                                assert(!isnan(map_main->creature[creature2].y));

								//if the new position collides, we're back to the old one
								if(creature_will_collide(map_main,&thiscreature)){
									thiscreature.x=thiscreature.x2;
									thiscreature.y=thiscreature.y2;
                                    assert(!isnan(thiscreature.x));
                                    assert(!isnan(thiscreature.y));
								}
								if(creature_will_collide(map_main,&map_main->creature[creature2])){
									map_main->creature[creature2].x=map_main->creature[creature2].x2;
									map_main->creature[creature2].y=map_main->creature[creature2].y2;
                                    assert(!isnan(map_main->creature[creature2].x));
                                    assert(!isnan(map_main->creature[creature2].y));
								}
							}
						}
					}
				}
			}

			if(!thiscreature.killed){

				int range=2;
				if(visible)range=3;

				//make sure creature is on map again
				if(thiscreature.x<left_map_limit)thiscreature.x=left_map_limit;
				if(thiscreature.y<top_map_limit)thiscreature.y=top_map_limit;
				if(thiscreature.x>right_map_limit)thiscreature.x=right_map_limit;
				if(thiscreature.y>bottom_map_limit)thiscreature.y=bottom_map_limit;

                assert(!isnan(thiscreature.x));
                assert(!isnan(thiscreature.y));

				//collisions prevented in creature specialties
				bool prop_prevented=false;
				bool plot_object_prevented=false;
				for(b=0;b<mod.general_creatures[thiscreature.type].specialties.size();b++){
					if(mod.general_creatures[thiscreature.type].specialties[b].number==4){
						if(mod.general_creatures[thiscreature.type].specialties[b].parameter3==1){
							prop_prevented=true;
						}
						if(mod.general_creatures[thiscreature.type].specialties[b].parameter0==1){
							plot_object_prevented=true;
						}
					}
				}

				//collision detection with objects, only if moved or creature is player
				if(!thiscreature.killed)
				if((thiscreature.x2!=thiscreature.x)
					||(thiscreature.y2!=thiscreature.y)
					||(creature==0)){
					int alku_x=(int)(thiscreature.x/grid_size-range);
					int alku_y=(int)(thiscreature.y/grid_size-range);
					int loppu_x=alku_x+range*2;
					int loppu_y=alku_y+range*2;

					if(alku_x<0)alku_x=0;
					if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
					if(alku_y<0)alku_y=0;
					if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

					int hits=0;
					for(a=alku_x;a<loppu_x;a++){
						for(b=alku_y;b<loppu_y;b++){
							//props
							if(!prop_prevented)
							for(c=0;c<map_main->grid[a].grid[b].objects.size();c++){
								if(map_main->object[map_main->grid[a].grid[b].objects[c]].dead)continue;

								if(creature_collision_detection(&thiscreature,&map_main->object[map_main->grid[a].grid[b].objects[c]],true))
									hits++;
							}
							//plot_objects
							if(!plot_object_prevented)
							for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
								if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
								if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=0)continue;

								if(creature_collision_detection(&thiscreature,&map_main->items[map_main->grid[a].grid[b].items[c]],true))
									hits++;
							}
						}

					}
				}
			}

		//make sure creature is on map again
		if(thiscreature.x<left_map_limit)thiscreature.x=left_map_limit;
		if(thiscreature.y<top_map_limit)thiscreature.y=top_map_limit;
		if(thiscreature.x>right_map_limit)thiscreature.x=right_map_limit;
		if(thiscreature.y>bottom_map_limit)thiscreature.y=bottom_map_limit;
        assert(!isnan(thiscreature.x));
        assert(!isnan(thiscreature.y));

		//AI pathfinding is not on, we need to block the creature from entering AI_avoid terrain
		if(!visible){
            int index_x = static_cast<int>((thiscreature.x+size*0.5f)/grid_size);
            assert (index_x >= 0); assert (index_x < map_main->sizex);
            int index_y = static_cast<int>((thiscreature.y+size*0.5f)/grid_size);
            assert (index_y >= 0); assert (index_y < map_main->sizey);
            terrain_type=map_main->grid[index_x].grid[index_y].terrain_type;

            if(mod.terrain_types[terrain_type].AI_avoid){
                thiscreature.x=thiscreature.x2;
                thiscreature.y=thiscreature.y2;
                assert(!isnan(thiscreature.x));
                assert(!isnan(thiscreature.y));
            }
		}

		/*if(map_main->creature[creature].x<0)map_main->creature[creature].x+=grid_size*map_main->sizex;
		if(map_main->creature[creature].y<0)map_main->creature[creature].y+=grid_size*map_main->sizey;
		if(map_main->creature[creature].x>map_main->sizex*grid_size-1)map_main->creature[creature].x-=grid_size*map_main->sizex;
		if(map_main->creature[creature].y>map_main->sizey*grid_size-1)map_main->creature[creature].y-=grid_size*map_main->sizey;
		*/

		//move carried lights
		if(thiscreature.carry_light>=0){
			carry_light(map_main,&thiscreature,thiscreature.carry_light);
		}


	}//end if(!killed)

	//animation
	if(visible){
			//leg animation
			if(!thiscreature.killed){
				int step=0;

				/*
				if(map_main->creature[creature].animation_timer[2]>1000){
					map_main->creature[creature].animation_timer[2]=0;
					step=1;//right leg steps
				}
				if(map_main->creature[creature].animation_timer[2]<0){
					map_main->creature[creature].animation_timer[2]=500;
					step=1;//right leg steps
				}



				map_main->creature[creature].animation_timer[2]+=map_main->creature[creature].backward_forward_speed*elapsed*game_speed*2*mod.general_creatures[map_main->creature[creature].type].leg_animation_speed;

				//left leg steps
				if((map_main->creature[creature].animation_timer[2]>0)&&(map_main->creature[creature].animation_timer[2]-map_main->creature[creature].backward_forward_speed*elapsed*game_speed*2*mod.general_creatures[map_main->creature[creature].type].movement_speed<=0))
					step=-1;*/

				float summed_step_speed=mod.general_creatures[thiscreature.type].leg_animation_speed*thiscreature.backward_forward_speed;
				if(mod.general_creatures[thiscreature.type].minimum_movement_speed>0){
					if(summed_step_speed<mod.general_creatures[thiscreature.type].minimum_movement_speed)
						summed_step_speed=mod.general_creatures[thiscreature.type].minimum_movement_speed;
				}


				thiscreature.animation_timer[2]+=elapsed*game_speed*2*summed_step_speed;
				//going forward
				if(elapsed*game_speed*2*summed_step_speed>0){
					//right leg steps
					if(thiscreature.animation_timer[2]>1000){
						step=1;
						thiscreature.animation_timer[2]=0;
					}
					//left leg steps
					if((thiscreature.animation_timer[2]>500)&&(thiscreature.animation_timer[2]-elapsed*game_speed*2*summed_step_speed<=500)){
						step=-1;
					}
				}
				else{
					//right leg steps
					if(thiscreature.animation_timer[2]<0){
						step=1;
						thiscreature.animation_timer[2]=1000;
					}
					//left leg steps
					if((thiscreature.animation_timer[2]<500)&&(thiscreature.animation_timer[2]+elapsed*game_speed*2*summed_step_speed>=500)){
						step=-1;
					}
				}


				//steps
				if(mod.general_creatures[thiscreature.type].footsteps>0)
				if(step!=0){


					float leg_xx=5*step;
					float leg_yy=5;

					float leg_x=thiscreature.x+size*0.5f+leg_xx*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation_legs)+leg_yy*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation_legs-pi/2);
					float leg_y=thiscreature.y+size*0.5f+leg_xx*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation_legs)+leg_yy*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation_legs-pi/2);

					int terrain_type=map_main->grid[(int)((leg_x)/grid_size)].grid[(int)((leg_y)/grid_size)].terrain_type;

					int particle_number=mod.general_creatures[thiscreature.type].footsteps;

					if(particle_number>=0)
					if(mod.terrain_types[terrain_type].footstep_particle_time>=0)
					if(mod.general_creatures[thiscreature.type].footstep_sounds.size()>0){

						int sound_number=mod.general_creatures[thiscreature.type].footstep_sounds[randInt(0,mod.general_creatures[thiscreature.type].footstep_sounds.size())];
						if(mod.terrain_types[terrain_type].override_footstep_particle>=0)
							particle_number=mod.terrain_types[terrain_type].override_footstep_particle;
						if(mod.terrain_types[terrain_type].override_footstep_sound>=0)
							sound_number=mod.terrain_types[terrain_type].override_footstep_sound;

						make_particle(particle_number, 0, mod.terrain_types[terrain_type].footstep_particle_time, leg_x,leg_y,0,0,thiscreature.rotation_legs);
						playsound(sound_number,1,leg_x,leg_y,player_middle_x,player_middle_y);
					}
				}


				//5,6,7,8,9,8,7,6,5
				if(fabs(thiscreature.animation_timer[2]-500)>400)
					thiscreature.animation[2]=5;
				else if(fabs(thiscreature.animation_timer[2]-500)>300)
					thiscreature.animation[2]=6;
				else if(fabs(thiscreature.animation_timer[2]-500)>200)
					thiscreature.animation[2]=7;
				else if(fabs(thiscreature.animation_timer[2]-500)>100)
					thiscreature.animation[2]=8;
				else if(fabs(thiscreature.animation_timer[2]-500)>0)
					thiscreature.animation[2]=9;
			}
			//torso animation
			//attack
			if(!thiscreature.killed)
			if(thiscreature.animation[1]<10){

				float time_maximum=500;
				if(mod.general_creatures[thiscreature.type].weapon>0)
					time_maximum=mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].fire_rate;
				thiscreature.animation_timer[1]-=elapsed*game_speed;
				if(thiscreature.animation_timer[1]<0)thiscreature.animation_timer[1]=0;

				//1,2,3,4,3,2,1
				//if(fabs(thiscreature.animation_timer[1])==0)
				thiscreature.animation[1]=1;//default

				if(fabs(thiscreature.animation_timer[1])>time_maximum*0.83f)
					thiscreature.animation[1]=2;
				else if(fabs(thiscreature.animation_timer[1])>time_maximum*0.66f)
					thiscreature.animation[1]=3;
				else if(fabs(thiscreature.animation_timer[1])>time_maximum*0.5f)
					thiscreature.animation[1]=4;
				else if(fabs(thiscreature.animation_timer[1])>time_maximum*0.33f)
					thiscreature.animation[1]=3;
				else if(fabs(thiscreature.animation_timer[1])>time_maximum*0.17f)
					thiscreature.animation[1]=2;
			}
	}

	//die
	if(thiscreature.killed)
	if(thiscreature.animation[1]>=10)
	if(thiscreature.animation[1]<15)
	{
		thiscreature.animation_timer[1]-=elapsed*game_speed*mod.general_creatures[thiscreature.type].death_animation_speed;
		if(thiscreature.animation_timer[1]<0){
			thiscreature.animation_timer[1]+=100;
			thiscreature.animation[1]++;
		}

	}
	//corpse vanishes when dead
	if(thiscreature.animation[1]==15){
		if(mod.general_creatures[thiscreature.type].corpse_item==-2){
			//thiscreature.dead=true;
			delete_creature(map_main,creature);
			return true;
		}
	}

	if(!thiscreature.killed)
		thiscreature.animation[0]=0;


	return false;

}

void game_engine::player_controls(int control_type){

	int a,b,c;

	if(override_player_controls>=0)
		control_type=override_player_controls;

	//if player controlled creature is dead now
	if(player_controlled_creature>0){
		if((map_main->creature[player_controlled_creature].killed)||(map_main->creature[player_controlled_creature].dead))
			player_controlled_creature=0;
	}


	//keyboard
	map_main->creature[0].left=0;
	map_main->creature[0].right=0;
	map_main->creature[0].up=0;
	map_main->creature[0].down=0;

	map_main->creature[player_controlled_creature].left=0;
	map_main->creature[player_controlled_creature].right=0;
	map_main->creature[player_controlled_creature].up=0;
	map_main->creature[player_controlled_creature].down=0;
	//map_main->creature[0].sneak=0;

	if(input_override)return;

	if(paused)return;

	//absolute
	float target=-1;
	if(control_type==0){


		if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W))target=0*pi/2;
		if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S))target=2*pi/2;
		if(grim->Key_Down(KEY_LEFT)||grim->Key_Down(KEY_A)){
			target=3*pi/2;
			if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W))target=3.5f*pi/2;
			if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S))target=2.5f*pi/2;
		}
		if(grim->Key_Down(KEY_RIGHT)||grim->Key_Down(KEY_D)){
			target=1*pi/2;
			if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W))target=0.5f*pi/2;
			if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S))target=1.5f*pi/2;
		}
	}

	//relative
	if(control_type==1){
		//float target=-1;
		if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W)){
			target=map_main->creature[player_controlled_creature].rotation;
		}
		if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S)){
			target=map_main->creature[player_controlled_creature].rotation+pi;
			if(target>pi*2)target-=pi*2;
		}
		if(grim->Key_Down(KEY_LEFT)||grim->Key_Down(KEY_A)){
			target=map_main->creature[player_controlled_creature].rotation-pi/2;
			if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W))target+=pi/4;
			if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S))target-=pi/4;
			if(target<0)target+=pi*2;
		}
		if(grim->Key_Down(KEY_RIGHT)||grim->Key_Down(KEY_D)){
			target=map_main->creature[player_controlled_creature].rotation+pi/2;
			if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W))target-=pi/4;
			if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S))target+=pi/4;
			if(target>pi*2)target-=pi*2;
		}
	}

	//vehicle controls
	if(control_type==2){
		target=-1;

		if(grim->Key_Down(KEY_UP)||grim->Key_Down(KEY_W))map_main->creature[player_controlled_creature].up=true;
		if(grim->Key_Down(KEY_DOWN)||grim->Key_Down(KEY_S))map_main->creature[player_controlled_creature].down=true;
		if(grim->Key_Down(KEY_LEFT)||grim->Key_Down(KEY_A)){
			map_main->creature[player_controlled_creature].left=true;
		}
		if(grim->Key_Down(KEY_RIGHT)||grim->Key_Down(KEY_D)){
			map_main->creature[player_controlled_creature].right=true;
		}

	}


	//change rotation
	float dq=target;
	if(target!=-1){

		//is the target direction left or right
		float temp1=map_main->creature[player_controlled_creature].rotation_legs;
		if(map_main->creature[player_controlled_creature].rotation_legs-dq>pi)temp1=temp1-pi*2;
		if(dq-map_main->creature[player_controlled_creature].rotation_legs>pi)dq=dq-pi*2;
			int negaatio=1;
			if (dq<temp1)
			negaatio=-1;

		if(negaatio==1){
			map_main->creature[player_controlled_creature].right=true;
		}
		else{
			map_main->creature[player_controlled_creature].left=true;
		}

		//snap
		if(fabs(map_main->creature[player_controlled_creature].rotation_legs-target)<turn_speed*elapsed*game_speed*mod.general_creatures[map_main->creature[player_controlled_creature].type].turn_speed){
			map_main->creature[player_controlled_creature].right=false;
			map_main->creature[player_controlled_creature].left=false;
			map_main->creature[player_controlled_creature].rotation_legs=target;
		}

		if(fabs(map_main->creature[player_controlled_creature].rotation_legs-target)<1.6f)
			map_main->creature[player_controlled_creature].up=true;
	}


	//mouse
	map_main->creature[player_controlled_creature].rotation=atan2(-camera_y+player_middle_y-mousey,-camera_x+player_middle_x-mousex)-pi/2;
	if(map_main->creature[player_controlled_creature].rotation<0)map_main->creature[player_controlled_creature].rotation+=pi*2;
	if(map_main->creature[player_controlled_creature].rotation>2*pi)map_main->creature[player_controlled_creature].rotation-=pi*2;
	map_main->creature[player_controlled_creature].rotation_head=map_main->creature[player_controlled_creature].rotation;

	//shoot
	if(mouse_left){
		map_main->creature[player_controlled_creature].fire=true;
	}

	//check wield conditions of the wielded weapon, unwield it if it cannot be used anymore
	/*bool OK=true;
	for(b=0;b<mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].wield_conditions.size();b++){
		if(!check_condition(mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].wield_conditions[b],&map_main->creature[0],player_middle_x,player_middle_y,false)){
			OK=false;
			break;
		}
	}
	if(!OK)
		if(player_items[weapon_selected_from_item_list].wielded){
			for(int b=0;b<general_items[player_items[weapon_selected_from_item_list].item].effects.size();b++){
				use_item(player_items[weapon_selected_from_item_list].item, &weapon_selected_from_item_list, general_items[player_items[weapon_selected_from_item_list].item].effects[b].effect, true, false);
			}
		}*/

	//change weapon
		if((mouse_wheel>0)||(key_e&&!key_e2)){
			a=weapon_selected_from_item_list;
			int passes=0;
			while(true){
				passes++;
				a++;
				if(a>=inventory[active_inventory].player_items.size())a=0;
				if(passes>inventory[active_inventory].player_items.size())break;
				Mod::effect effect;
				if(item_has_effect(inventory[active_inventory].player_items[a].item,9,&effect)){
					if(mod.general_creatures[map_main->creature[0].type].weapon!=effect.parameter1){

						//see if the new weapon qualifies
						bool can_use=true;
						for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
							//see if this effect block is the one with the use weapon effect
							bool has_weapon_effect=false;
							for(int c=0;c<mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects.size();c++){
								if(mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects[c].effect_number==9){
									has_weapon_effect=true;

									/*Mod::effect weapon_wear_effect=mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects[c];
									//check wield conditions for the proposed weapon
									bool OK=true;
									for(int d=0;d<mod.general_weapons[weapon_wear_effect.parameter1].wield_conditions.size();d++){
										if(!check_condition(mod.general_weapons[weapon_wear_effect.parameter1].wield_conditions[d],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
											OK=false;
											break;
										}
									}
									if(!OK){
										can_use=false;
										break;
									}

									//find if this weapon class is disabled for player race
									if(mod.general_races[player_race].weapon_classes[mod.general_weapons[weapon_wear_effect.parameter1].weapon_class].can_use==0){
										can_use=false;
										break;
									}*/
								}
							}
							if(!has_weapon_effect)continue;

							//set slots of old weapon temporarily to unused
							if(weapon_selected_from_item_list>=0){
								if(inventory[active_inventory].player_items[weapon_selected_from_item_list].wielded){
									for(int slot=0;slot<mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots.size();slot++){
										inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots[slot]]=-1;
									}
								}
							}

							if(!use_item(inventory[active_inventory].player_items[a].item, &a, mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect, false, false,true,true)){
								can_use=false;
							}

							//set slots of old weapon back to used
							if(weapon_selected_from_item_list>=0){
								if(inventory[active_inventory].player_items[weapon_selected_from_item_list].wielded){
									for(int slot=0;slot<mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots.size();slot++){
										inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots[slot]]=weapon_selected_from_item_list;
									}
								}
							}
						}


						if(!can_use)continue;

						//unwield old weapon
						/*int temp_weapon=weapon_selected_from_item_list;
						if(temp_weapon>=0)
						if(inventory[active_inventory].player_items[temp_weapon].wielded){
							for(b=0;b<mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects.size();b++){
								use_item(inventory[active_inventory].player_items[temp_weapon].item, &temp_weapon, mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects[b].effect, true, false, false,false);
							}
						}*/

						//unwield all weapons
						for(int d=0;d<inventory[active_inventory].player_items.size();d++){
							if(inventory[active_inventory].player_items[d].wielded){

								//see if this item has a weapon effect
								bool has_weapon_effect=false;
								for(b=0;b<mod.general_items[inventory[active_inventory].player_items[d].item].effects.size();b++){
									//see if this effect block is the one with the use weapon effect
									for(c=0;c<mod.general_items[inventory[active_inventory].player_items[d].item].effects[b].effect.effects.size();c++){
										if(mod.general_items[inventory[active_inventory].player_items[d].item].effects[b].effect.effects[c].effect_number==9)
											has_weapon_effect=true;
									}
								}


								//unwield it
								if(has_weapon_effect)
								for(b=0;b<mod.general_items[inventory[active_inventory].player_items[d].item].effects.size();b++){
									use_item(inventory[active_inventory].player_items[d].item, &d, mod.general_items[inventory[active_inventory].player_items[d].item].effects[b].effect, true, false, false,false);
								}
							}
						}

						//run the effects of the new weapon
						for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
							//see if this effect block is the one with the use weapon effect
							bool has_weapon_effect=false;
							for(c=0;c<mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects.size();c++){
								if(mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects[c].effect_number==9)
									has_weapon_effect=true;
							}
							if(!has_weapon_effect)continue;

							if(use_item(inventory[active_inventory].player_items[a].item, &a, mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect, false, false, false,true)){
								//inventory[active_inventory].player_items[b].amount--;
							}
						}

						/*//if the weapon wasn't changed, change back
						if(temp_weapon>=0)
						if((temp_weapon==weapon_selected_from_item_list)||(weapon_selected_from_item_list==-1)){
							for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects.size();b++){
								use_item(inventory[active_inventory].player_items[temp_weapon].item, &temp_weapon, mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects[b].effect, false, false);
							}
							//continue;
						}*/

						break;

					}
				}
			}
		}
		if(mouse_wheel<0){
			a=weapon_selected_from_item_list;
			int passes=0;
			while(true){
				passes++;
				a--;
				if(a<0)a=inventory[active_inventory].player_items.size()-1;
				if(passes>inventory[active_inventory].player_items.size())break;
				//if(inventory[active_inventory].player_items[a].item<0)continue;
				//if(inventory[active_inventory].player_items[a].amount<=0)continue;
				Mod::effect effect;
				if(item_has_effect(inventory[active_inventory].player_items[a].item,9,&effect)){
					if(mod.general_creatures[map_main->creature[0].type].weapon!=effect.parameter1){

						//see if the new weapon qualifies
						bool can_use=true;
						for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
							//see if this effect block is the one with the use weapon effect
							bool has_weapon_effect=false;
							for(c=0;c<mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects.size();c++){
								if(mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects[c].effect_number==9){
									has_weapon_effect=true;

									/*Mod::effect weapon_wear_effect=mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects[c];
									//check wield conditions for the proposed weapon
									bool OK=true;
									for(int d=0;d<mod.general_weapons[weapon_wear_effect.parameter1].wield_conditions.size();d++){
										if(!check_condition(mod.general_weapons[weapon_wear_effect.parameter1].wield_conditions[d],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
											OK=false;
											break;
										}
									}
									if(!OK){
										can_use=false;
										break;
									}

									//find if this weapon class is disabled for player race
									if(mod.general_races[player_race].weapon_classes[mod.general_weapons[weapon_wear_effect.parameter1].weapon_class].can_use==0){
										can_use=false;
										break;
									}*/
								}
							}
							if(!has_weapon_effect)continue;

							//set slots of old weapon temporarily to unused
							if(weapon_selected_from_item_list>=0){
								if(inventory[active_inventory].player_items[weapon_selected_from_item_list].wielded){
									for(int slot=0;slot<mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots.size();slot++){
										inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots[slot]]=-1;
									}
								}
							}

							if(!use_item(inventory[active_inventory].player_items[a].item, &a, mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect, false, false,true,true)){
								can_use=false;
							}

							//set slots of old weapon back to used
							if(weapon_selected_from_item_list>=0){
								if(inventory[active_inventory].player_items[weapon_selected_from_item_list].wielded){
									for(int slot=0;slot<mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots.size();slot++){
										inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].wield_slots[slot]]=weapon_selected_from_item_list;
									}
								}
							}
						}
						if(!can_use)continue;

						//unwield old weapon
						/*int temp_weapon=weapon_selected_from_item_list;
						if(temp_weapon>=0)
						if(inventory[active_inventory].player_items[temp_weapon].wielded){
							for(b=0;b<mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects.size();b++){
								use_item(inventory[active_inventory].player_items[temp_weapon].item, &temp_weapon, mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects[b].effect, true, false, false,false);
							}
							//continue;
						}*/
						//unwield all weapons
						for(int d=0;d<inventory[active_inventory].player_items.size();d++){
							if(inventory[active_inventory].player_items[d].wielded){

								//see if this item has a weapon effect
								bool has_weapon_effect=false;
								for(b=0;b<mod.general_items[inventory[active_inventory].player_items[d].item].effects.size();b++){
									//see if this effect block is the one with the use weapon effect
									for(c=0;c<mod.general_items[inventory[active_inventory].player_items[d].item].effects[b].effect.effects.size();c++){
										if(mod.general_items[inventory[active_inventory].player_items[d].item].effects[b].effect.effects[c].effect_number==9)
											has_weapon_effect=true;
									}
								}


								//unwield it
								if(has_weapon_effect)
								for(b=0;b<mod.general_items[inventory[active_inventory].player_items[d].item].effects.size();b++){
									use_item(inventory[active_inventory].player_items[d].item, &d, mod.general_items[inventory[active_inventory].player_items[d].item].effects[b].effect, true, false, false,false);
								}
							}
						}

						//run the effects of the new weapon
						for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
							//see if this effect block is the one with the use weapon effect
							bool has_weapon_effect=false;
							for(c=0;c<mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects.size();c++){
								if(mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect.effects[c].effect_number==9)
									has_weapon_effect=true;
							}
							if(!has_weapon_effect)continue;

							if(use_item(inventory[active_inventory].player_items[a].item, &a, mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect, false, false, false,true)){
								//inventory[active_inventory].player_items[b].amount--;
							}
						}

						/*//if the weapon wasn't changed, change back
						if(temp_weapon>=0)
						if((temp_weapon==weapon_selected_from_item_list)||(weapon_selected_from_item_list==-1)){
							for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects.size();b++){
								use_item(inventory[active_inventory].player_items[temp_weapon].item, &temp_weapon, mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects[b].effect, false, false);
							}
							//continue;
						}*/

						break;
					}
				}
			}
		}
	//}




	//wielded items
	for(a=0;a<inventory[active_inventory].player_items.size();a++){

		if(mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots.size()>0)
		if(inventory[active_inventory].player_items[a].wielded){

			//check if the conditions are still valid
			bool OK=true;
			//for(c=0;c<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();c++){
			for(c=0;c<1;c++){
				for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects[c].effect.conditions.size();b++){
					if(!check_condition(mod.general_items[inventory[active_inventory].player_items[a].item].effects[c].effect.conditions[b],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
						OK=false;
						break;
					}
				}
			}

			//check wielded item disabling script
			if(mod.general_items[inventory[active_inventory].player_items[a].item].wielded_disabling_script>=0){
				int script_number=mod.general_items[inventory[active_inventory].player_items[a].item].wielded_disabling_script;

				for(b=0;b<mod.scripts[script_number].conditions.size();b++){
					if(!check_condition(mod.scripts[script_number].conditions[b],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
						OK=false;
						break;
					}
				}
			}


			//unuse item if the conditions aren't valid
			if(!OK){
				for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
					int item_number=a;
					use_item(inventory[active_inventory].player_items[a].item, &item_number, mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect, true, false, false,false);
				}
			}


			//run wielded item scripts
			if(mod.general_items[inventory[active_inventory].player_items[a].item].wielded_script>=0)
				run_script(mod.general_items[inventory[active_inventory].player_items[a].item].wielded_script,true,true);
		}
	}


	//stagger mouse
	if(stagger_mouse_time>0){
		stagger_mouse_time-=elapsed*game_speed;
		if(randInt(0,30/(elapsed*game_speed*stagger_mouse_parameter2))==0){
			mousex+=randDouble(-1,1)*stagger_mouse_parameter1;
			mousey+=randDouble(-1,1)*stagger_mouse_parameter1;
		}
	}



}

void game_engine::calculate_quick_keys(bool only_inventory){
	int a,b,c,d;
	//quick keys
	for(a=0;a<quick_keys.size();a++){
		//has a quick key been pressed
		if(key_clicked[translate_key_int(quick_keys[a].key)]){
			//does the player have the item with this quick key
			for(b=0;b<inventory[active_inventory].player_items.size();b++)
			if((inventory[active_inventory].player_items[b].item==quick_keys[a].item_type)&&(inventory[active_inventory].player_items[b].amount>0)){
				vector <int> use_effects;

				key_clicked[translate_key_int(quick_keys[a].key)]=false;

				//find the right effects for it
				for(c=0;c<mod.general_items[inventory[active_inventory].player_items[b].item].effects.size();c++)
				if(mod.general_items[inventory[active_inventory].player_items[b].item].effects[c].quick_key==quick_keys[a].key){
					use_effects.push_back(c);
				}

				//from the inventory we can only call the return from inventory effect
				if(only_inventory){
					bool has_inventory_effect=false;
					/*for(d=0;d<use_effects.size();d++){
						if(mod.general_items[inventory[active_inventory].player_items[b].item].effects[use_effects[d]].effect.effect_base[c].effect_number==62)
							has_inventory_effect=true;
					}*/
					Mod::effect temp_effect;
					if(item_has_effect(inventory[active_inventory].player_items[b].item,62,&temp_effect))
						has_inventory_effect=true;
					if(!has_inventory_effect)continue;
				}


				//if the item is already wielded, unwield it and continue
				if(inventory[active_inventory].player_items[b].wielded){
					for(int d=0;d<use_effects.size();d++){
						use_item(inventory[active_inventory].player_items[b].item, &b, mod.general_items[inventory[active_inventory].player_items[b].item].effects[use_effects[d]].effect, true, false, false,false);
					}
					continue;
				}


				//check if the item can be wielded
				bool OK=true;
				for(d=0;d<use_effects.size();d++){
					if(!use_item(inventory[active_inventory].player_items[b].item, &b, mod.general_items[inventory[active_inventory].player_items[b].item].effects[use_effects[d]].effect, false, true, true,false)){
						OK=false;
					}
				}
				if(OK){

					//go through all the slots the item would use, and see if there are any items being used there
					if(mod.general_items[inventory[active_inventory].player_items[b].item].wield_slots.size()>0){
						for(c=0;c<mod.general_items[inventory[active_inventory].player_items[b].item].wield_slots.size();c++){
							//if the slot has been used
							if(inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[b].item].wield_slots[c]]>=0){
								int item_number=inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[b].item].wield_slots[c]];
								//unuse the item
								for(int e=0;e<mod.general_items[inventory[active_inventory].player_items[item_number].item].effects.size();e++){
									use_item(inventory[active_inventory].player_items[item_number].item, &item_number, mod.general_items[inventory[active_inventory].player_items[item_number].item].effects[e].effect, true, false, false,false);
								}

							}
						}
					}

				}
				else continue;

				//run the effects
				for(d=0;d<use_effects.size();d++){
					if(use_item(inventory[active_inventory].player_items[b].item, &b, mod.general_items[inventory[active_inventory].player_items[b].item].effects[use_effects[d]].effect, false, true, false,false)){
						inventory[active_inventory].player_items[b].amount--;
					}
				}
				if(inventory[active_inventory].player_items[b].amount<=0){
					b=delete_item(b);
				}
			}
		}
	}
}

void game_engine::draw_lights(int layer){//draws light effects (flashlight, explosions
	int i,j;
	float light_x,light_y;
	grim->System_SetState_Blending(true);
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_ONE);


	for(j=0;j<mod.general_lights.size();j++){
		if(mod.general_lights[j].dead)continue;
		resources.Texture_Set(mod.general_lights[j].texture);

		//resources.Texture_Set(temp_texture_number);

		grim->Quads_Begin();
		for(i=0;i<map_main->lights.size();i++){
			if(map_main->lights[i].dead)continue;
			if(map_main->lights[i].type!=j)continue;


			light_x=map_main->lights[i].x;
			light_y=map_main->lights[i].y;




			//it still might not need to be drawn
			float size=map_main->lights[i].size*128;
			if(-camera_x+light_x<-size*1.415f)continue;
			if(-camera_x+light_x>screen_width+size*1.415f)continue;
			if(-camera_y+light_y<-size*1.415f)continue;
			if(-camera_y+light_y>screen_height+size*1.415f)continue;



			//draw
			grim->Quads_SetColor(map_main->lights[i].r,map_main->lights[i].g,map_main->lights[i].b,map_main->lights[i].transparency);
			grim->Quads_SetRotation(map_main->lights[i].rotation);
			grim->Quads_Draw(
				-camera_x+light_x+map_main->lights[i].size*128*0.5f-map_main->lights[i].pulse*map_main->lights[i].size*128*0.5f
				, -camera_y+light_y+map_main->lights[i].size*128*0.5f-map_main->lights[i].pulse*map_main->lights[i].size*128*0.5f
				, size*(map_main->lights[i].pulse)
				, size*(map_main->lights[i].pulse)
				);

		}
		grim->Quads_End();
	}
}


void game_engine::load_setup(const char *filename){//loads setup from file

	FILE *fil;
	char rivi[800]="none";

	fil = fopen(filename,"rt");
	if(fil){
		stripped_fgets(rivi,sizeof(rivi),fil);

			debug.debug_state[0]=1;
			debug.debug_state[1]=0;
			random_seed=randInt(0,10000);
			pop_up_x=768;
			pop_up_y=512;
			pop_up_transparency=1.0f;
			player_control_type=0;
			mouse_speed=0.708156f;
			arrange_alphabetically=0;
			show_journals=1;
			volume_slider[0]=0.996000f;
			volume_slider[1]=0.721046f;
			last_saved_game=-1;
			last_played_mod="Default";

		if(string(rivi)=="none"){
			fclose(fil);
			return;
		}

		debug.debug_state[0]=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
		debug.debug_state[1]=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
		random_seed=atoi(stripped_fgets(rivi,sizeof(rivi),fil));//random seed
		pop_up_x=atof(stripped_fgets(rivi,sizeof(rivi),fil));//pop_up_x
		pop_up_y=atof(stripped_fgets(rivi,sizeof(rivi),fil));//pop_up_y
		pop_up_transparency=atof(stripped_fgets(rivi,sizeof(rivi),fil));//pop_up_transparency
		player_control_type=atoi(stripped_fgets(rivi,sizeof(rivi),fil));//player_control_type
		mouse_speed=atof(stripped_fgets(rivi,sizeof(rivi),fil));//mouse_speed
		arrange_alphabetically=strtobool(stripped_fgets(rivi,sizeof(rivi),fil));
		show_journals=strtobool(stripped_fgets(rivi,sizeof(rivi),fil));
		volume_slider[0]=atof(stripped_fgets(rivi,sizeof(rivi),fil));//volume_slider[0]
		volume_slider[1]=atof(stripped_fgets(rivi,sizeof(rivi),fil));//volume_slider[1]
		last_saved_game=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
		last_played_mod=stripped_fgets(rivi,sizeof(rivi),fil);
		//play_puzzle=atoi(stripped_fgets(rivi,sizeof(rivi),fil));//puzzle difficulty
	}

	fclose(fil);

}

void game_engine::save_setup(const char *filename){//saves setup to file

	FILE *fil;
	//char rivi[800];

	fil = fopen(filename,"wt");
	if(mouse_speed>0)
	if(fil){
		fprintf(fil, "%s\n", "-");//OK line
		fprintf(fil, "%d;//create game initialization debug file\n", debug.debug_state[0]);
		fprintf(fil, "%d;//create game running debug file\n", debug.debug_state[1]);
		fprintf(fil, "%d\n", random_seed);//random seed
		fprintf(fil, "%d\n", (int)pop_up_x);//pop_up_x
		fprintf(fil, "%d\n", (int)pop_up_y);//pop_up_y
		fprintf(fil, "%f\n", pop_up_transparency);//pop_up_transparency
		fprintf(fil, "%d\n", player_control_type);//pop_up_transparency
		fprintf(fil, "%f\n", mouse_speed);//pop_up_transparency
		fprintf(fil, "%d\n", arrange_alphabetically);//arrange_alphabetically
		fprintf(fil, "%d\n", show_journals);
		fprintf(fil, "%f\n", volume_slider[0]);//
		fprintf(fil, "%f\n", volume_slider[1]);//
		fprintf(fil, "%d\n", last_saved_game);
		fprintf(fil, "%s\n", last_played_mod.c_str());

		//fprintf(fil, "%d\n", play_puzzle);

	}

	fclose(fil);

}

float game_engine::calculate_flashlight(float c_x,float c_y, float rotation, float t_x, float t_y,float *distance,float *angle){//returns the light value using the flashlight

	//calculate light
	//per distance
	*distance=sqr(t_x-c_x)+sqr(t_y-c_y);
	//float light=1-0.00000000003f*sqr(*distance);
	float light=1-0.00000000004f*(sqr(sqr(t_x-c_x))+sqr(sqr(t_y-c_y)));
	//per angle
	if(light>0){
		float dq=-atan2(t_x-c_x,t_y-c_y);
		float temp1=rotation;
		if(rotation-dq>pi)temp1=temp1-pi*2;
		if(dq-rotation>pi)dq=dq-pi*2;
		*angle=(float)fabs(dq-temp1);
		//*angle=(float)fabs(temp1-dq);
		light=light*sqr(*angle)*0.2f;
	}
	if(light<0)light=0;

	return light;
}


bullet game_engine::shoot(int from_creature, int side,int type, float startx,float starty,float angle)//fire a bullet
{
	int b,q;


		bullet temp_bullet;
		temp_bullet.dead=true;

		float shooter_move_x=(map_main->creature[from_creature].x-map_main->creature[from_creature].x2)/(elapsed*game_speed);
		float shooter_move_y=(map_main->creature[from_creature].y-map_main->creature[from_creature].y2)/(elapsed*game_speed);


		//find out if the gun point is within a wall
		if(point_will_collide(map_main,startx,starty,true))
			return temp_bullet;



		//reduce ammo energy


			//check conditions
			bool OK=true;
			for(b=0;b<mod.general_weapons[type].wield_conditions.size();b++){
				bool show_text=false;
				if(from_creature==0)show_text=true;
				if(!check_condition(mod.general_weapons[type].wield_conditions[b],&map_main->creature[from_creature],from_creature,startx,starty,show_text)){
					OK=false;
					break;
				}
			}
			if(!OK)return temp_bullet;


			//debug.debug_output("shoot1",3);

			//run fire effects
			bool effect_ran=true;
			for(b=0;b<mod.general_weapons[type].fire_effects.size();b++){
				if(!run_effect(mod.general_weapons[type].fire_effects[b],&map_main->creature[from_creature],from_creature,startx,starty,angle,false)){
					effect_ran=false;
					break;
				}

			}
			if(!effect_ran)
				return temp_bullet;

			//debug.debug_output("shoot2",3);

		//}



		playsound(mod.general_weapons[type].sound_fire,2,startx,starty,player_middle_x,player_middle_y);



		for (q=0; q<mod.general_weapons[type].bullets_at_once; q++){



			memset(&temp_bullet, 0, sizeof(temp_bullet));

			temp_bullet.dead=false;
			temp_bullet.dead_on_next=false;
			temp_bullet.x=startx;
			temp_bullet.y=starty;
			temp_bullet.x2=startx;
			temp_bullet.y2=starty;
			temp_bullet.x3=startx;
			temp_bullet.y3=starty;
			temp_bullet.x4=startx;
			temp_bullet.y4=starty;
			//if bullet doesn't want to move, don't move it
			if(mod.general_weapons[type].bullet_speed==0){
				temp_bullet.move_x=0;
				temp_bullet.move_y=0;
				temp_bullet.shooter_move_x=0;
				temp_bullet.shooter_move_y=0;
			}
			//it moves
			else{
				float random_speed=randDouble(0.90f,1.10f);
				float random_angle=randDouble(-mod.general_weapons[type].spread,mod.general_weapons[type].spread);
				temp_bullet.move_x=sincos.table_cos(angle+random_angle-pi/2)*mod.general_weapons[type].bullet_speed*random_speed;
				temp_bullet.move_y=sincos.table_sin(angle+random_angle-pi/2)*mod.general_weapons[type].bullet_speed*random_speed;
				temp_bullet.shooter_move_x=shooter_move_x;
				temp_bullet.shooter_move_y=shooter_move_y;
			}


			temp_bullet.type=type;
			temp_bullet.side=side;
			temp_bullet.from_creature=from_creature;
			temp_bullet.time=mod.general_weapons[type].time;
			temp_bullet.angle=angle;


			map_main->bullets.push_back(temp_bullet);


		}

	return temp_bullet;
}



//calculates bullet flight
void game_engine::calculate_bullets(void){
	int a,b,c;

	/*
	float alien_damage_difficulty_modifier=0.001f;
	if(game_difficulty_level==0)
		alien_damage_difficulty_modifier=0.000f;
	if(game_difficulty_level==2)
		alien_damage_difficulty_modifier=0.002f;
		*/


	list<bullet>::iterator it;

	if(elapsed*game_speed>0)
	for(it=map_main->bullets.begin(); it!=map_main->bullets.end();){

		if((*it).dead){
			//remove from list
			it=map_main->bullets.erase(it);
			continue;
		}



		if((*it).dead_on_next){
			(*it).dead=true;
			continue;
		}

		//limited time
		if((*it).time!=-1){
			(*it).time-=elapsed*game_speed;
			if((*it).time<0){
				(*it).dead_on_next=true;
			}
		}


		//move
		(*it).x3=(*it).x;
		(*it).y3=(*it).y;
		(*it).x+=(*it).move_x*elapsed*game_speed;
		(*it).y+=(*it).move_y*elapsed*game_speed;
		(*it).x2=((*it).x3+(*it).x)/2;//between
		(*it).y2=((*it).y3+(*it).y)/2;//between


		(*it).x+=(*it).shooter_move_x*elapsed*game_speed;
		(*it).x2+=(*it).shooter_move_x*elapsed*game_speed;
		(*it).x3+=(*it).shooter_move_x*elapsed*game_speed;
		(*it).x4+=(*it).shooter_move_x*elapsed*game_speed;
		(*it).y+=(*it).shooter_move_y*elapsed*game_speed;
		(*it).y2+=(*it).shooter_move_y*elapsed*game_speed;
		(*it).y3+=(*it).shooter_move_y*elapsed*game_speed;
		(*it).y4+=(*it).shooter_move_y*elapsed*game_speed;

		//hits walls
		vector <point2d> wall_collisions=line_will_collide((*it).x,(*it).y,(*it).x3,(*it).y3,true, false, false, true, 2, true, true);
		for (a=0; a<wall_collisions.size(); a++){

			//play bullet hit sound
			playsound(mod.general_weapons[(*it).type].sound_hit,1,wall_collisions[a].x,wall_collisions[a].y,player_middle_x,player_middle_y);

			//stop the bullet
			//if(mod.general_weapons[(*it).type].stop_on_hit==0){
				(*it).dead_on_next=true;
				(*it).x=wall_collisions[a].x;
				(*it).y=wall_collisions[a].y;
				break;
			//}
		}

		//creature collision detection
		{
			vector <collision_base> collisions=list_collisions((*it).x,(*it).y,(*it).x3,(*it).y3,false);

			for (a=0; a<collisions.size(); a++){

				//creatures
				if(collisions[a].type==0){

					//rule out the shooter's side
					if(map_main->creature[collisions[a].subtype].side==(*it).side)continue;

					//rule out the shooter
					if(collisions[a].subtype==(*it).from_creature)continue;


					//bullet collisions prevented
					bool prevented=false;
					for(b=0;b<mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties.size();b++){
						if(mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties[b].number==4){
							if(mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties[b].parameter2==1){
								prevented=true;
								break;
							}
						}
					}
					if(prevented)continue;

					//check immunities
					float damage_multiplier=1;
					bool immune=false;
					for(b=0;b<mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties.size();b++){
						if(mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties[b].number==1){
							if(mod.general_weapons[(*it).type].weapon_class==mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties[b].parameter0)
								if(mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties[b].parameter1==0)
									immune=true;
								else damage_multiplier=mod.general_creatures[map_main->creature[collisions[a].subtype].type].specialties[b].parameter1;
						}
					}
					if(immune)continue;



					//unstun it when any bullet hits it
					bool unstunned=false;
					for(b=0;b<map_main->creature[collisions[a].subtype].weapon_effects_amount;b++){
						if(map_main->creature[collisions[a].subtype].weapon_effects[a].effect_type==0)
						if(map_main->creature[collisions[a].subtype].weapon_effects[a].parameter1==1){
							map_main->creature[collisions[a].subtype].weapon_effects[a].time=0;
							unstunned=true;
						}
					}


					//full anger after a hit
					map_main->creature[collisions[a].subtype].anger_level=1;


					//run bullet hit effects
					float damage=0;
					for(b=0;b<mod.general_weapons[(*it).type].hit_effects.size();b++){

						Mod::effect temp_effect=mod.general_weapons[(*it).type].hit_effects[b];

						//resistances for bar changing effects
						if(temp_effect.effect_number==4){
							temp_effect.parameter1=temp_effect.parameter1*damage_multiplier*randDouble(0.9f,1.1f);

							//reduce damage for player's armor
							if(temp_effect.parameter1<0)
							if(collisions[a].subtype==0){
								temp_effect.parameter1+=armor;
								if(temp_effect.parameter1>-1)temp_effect.parameter1=-1;
							}

							damage+=temp_effect.parameter1;


						}
						else if(temp_effect.effect_number==23){
							temp_effect.parameter1=temp_effect.parameter1*damage_multiplier*randDouble(0.9f,1.1f);

							//reduce damage for player's armor
							if(temp_effect.parameter1<0)
							if(collisions[a].subtype==0){
								temp_effect.parameter1+=armor;
								if(temp_effect.parameter1>-1)temp_effect.parameter1=-1;
							}

							damage+=temp_effect.parameter1;
						}
						else if(temp_effect.effect_number==28){
							temp_effect.parameter3=temp_effect.parameter3*damage_multiplier*randDouble(0.9f,1.1f);

							damage+=5;
						}
						else if(temp_effect.effect_number==44){
							temp_effect.parameter1=temp_effect.parameter1*damage_multiplier*randDouble(0.9f,1.1f);

							//reduce damage for player's armor
							if(temp_effect.parameter1<0)
							if(collisions[a].subtype==0){
								temp_effect.parameter1+=armor;
								if(temp_effect.parameter1>-1)temp_effect.parameter1=-1;
							}

							damage+=temp_effect.parameter1;
						}
						else if(temp_effect.effect_number==53){
							temp_effect.parameter1=temp_effect.parameter1*damage_multiplier*randDouble(0.9f,1.1f);

							//reduce damage for player's armor
							if(temp_effect.parameter1<0)
							if(collisions[a].subtype==0){
								temp_effect.parameter1+=armor;
								if(temp_effect.parameter1>-1)temp_effect.parameter1=-1;
							}

							damage+=temp_effect.parameter1;
						}

						//cannot unstun and stun at the same time
						if(unstunned){
							if(temp_effect.effect_number==1){
								continue;
							}
						}


						run_effect(temp_effect,&map_main->creature[collisions[a].subtype],collisions[a].subtype,collisions[a].x,collisions[a].y,map_main->creature[collisions[a].subtype].rotation,false);
					}

					//run creature hit effects
					for(c=0;c<mod.general_creatures[map_main->creature[collisions[a].subtype].type].hit_block.size();c++){
						Mod::general_creatures_base::effect_block effect=mod.general_creatures[map_main->creature[collisions[a].subtype].type].hit_block[c];

						//go throught all conditions
						bool OK=true;
						for(b=0;b<effect.conditions.size();b++){
							if(!check_condition(effect.conditions[b],&map_main->creature[collisions[a].subtype],collisions[a].subtype,collisions[a].x,collisions[a].y,false)){
								OK=false;
								break;
							}
						}
						if(!OK)continue;

						//run effects
						for(b=0;b<effect.effects.size();b++){
							run_effect(effect.effects[b],&map_main->creature[collisions[a].subtype],collisions[a].subtype,collisions[a].x,collisions[a].y,map_main->creature[collisions[a].subtype].rotation,false);
						}
					}

					//remember the hit
					map_main->creature[collisions[a].subtype].last_bullet_hit_from_creature_number=(*it).from_creature;
					map_main->creature[collisions[a].subtype].last_weapon_to_hit=(*it).type;
					map_main->creature[collisions[a].subtype].last_weapon_to_hit_time=time_from_beginning;

					//blood!
					if(-damage>0){
						int blood=randInt(0,(int)-damage);
						if(blood>100)blood=100;
						for (int q=0; q<blood; q++){
							float random_speed=randDouble(0.1f,0.2f);
							make_particle(mod.general_creatures[map_main->creature[collisions[a].subtype].type].blood,0,randDouble(100,1000),collisions[a].x,collisions[a].y,random_speed*sincos.table_cos((*it).angle+randDouble(-0.6f,0.6f)-pi*0.5f),random_speed*sincos.table_sin((*it).angle+randDouble(-0.6f,0.6f)-pi*0.5f));
						}
					}


					//target falls back
					map_main->creature[collisions[a].subtype].vx+=(*it).move_x*mod.general_weapons[(*it).type].push;
					map_main->creature[collisions[a].subtype].vy+=(*it).move_y*mod.general_weapons[(*it).type].push;



					//play groan sound
					if(mod.general_creatures[map_main->creature[collisions[a].subtype].type].hit_sound.size()>0)
					if(randInt(0,2)==0){
						playsound(mod.general_creatures[map_main->creature[collisions[a].subtype].type].hit_sound[randInt(0,mod.general_creatures[map_main->creature[collisions[a].subtype].type].hit_sound.size())],randDouble(0.5f,0.8f),collisions[a].x,collisions[a].y,player_middle_x,player_middle_y);
					}
				}

				//play bullet hit sound
				playsound(mod.general_weapons[(*it).type].sound_hit,1,collisions[a].x,collisions[a].y,player_middle_x,player_middle_y);

				//stop the bullet
				if(mod.general_weapons[(*it).type].stop_on_hit==0){
					(*it).dead_on_next=true;
					(*it).x=collisions[a].x;
					(*it).y=collisions[a].y;
					break;
				}

			}
			}

			it++;
	}
}


void game_engine::draw_bullets(void){

	int j;
	grim->System_SetState_Blending(true);
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);

	//add all bullets to list
	list <list<bullet>::iterator> trace_list;
	list <list<bullet>::iterator> bullet_list;

	list<bullet>::iterator it_bullets;
	for(it_bullets=map_main->bullets.begin(); it_bullets!=map_main->bullets.end(); it_bullets++){
		if((*it_bullets).dead)continue;

		if(mod.general_weapons[(*it_bullets).type].trace1>0)
			trace_list.push_back(it_bullets);
		if(mod.general_weapons[(*it_bullets).type].size>0)
			bullet_list.push_back(it_bullets);
	}

	list<list<bullet>::iterator>::iterator it;
	for(j=0;j<mod.general_weapons.size();j++){
		if(mod.general_weapons[j].dead)continue;
		//trace
		for(it=trace_list.begin(); it!=trace_list.end(); it++){
			list<bullet>::iterator bullet=*it;
			if((*bullet).type!=j)continue;



			float r,g,b;
			//electric
			if(mod.general_weapons[(*bullet).type].trace1==1){
				r=0;
				g=0;
				b=1;
				text_manager.draw_line(-camera_x+(*bullet).x4,-camera_y+(*bullet).y4,-camera_x+(*bullet).x,-camera_y+(*bullet).y,1,0,1,r,g,b);
			}
			//bullet trace
			else if(mod.general_weapons[(*bullet).type].trace1==2){
				r=1;
				g=1;
				b=1;
				(*bullet).distance=sqr((*bullet).x-map_main->creature[0].x)+sqr((*bullet).y-map_main->creature[0].y);
				float distance_fade=(0.00003f*((*bullet).distance));
				if(distance_fade<1)distance_fade=1;
				float fade=1.0f/distance_fade;
				text_manager.draw_line(-camera_x+(*bullet).x4,-camera_y+(*bullet).y4,-camera_x+(*bullet).x,-camera_y+(*bullet).y,1,0,1*fade,r,g,b);
			}
			//laser
			else if(mod.general_weapons[(*bullet).type].trace1==3){
				r=1;
				g=0;
				b=0;
				text_manager.draw_line(-camera_x+(*bullet).x4,-camera_y+(*bullet).y4,-camera_x+(*bullet).x,-camera_y+(*bullet).y,1,1,1,r,g,b);
			}
			//green beam
			else if(mod.general_weapons[(*bullet).type].trace1==4){
				r=0;
				g=1;
				b=0;
				text_manager.draw_line(-camera_x+(*bullet).x4,-camera_y+(*bullet).y4,-camera_x+(*bullet).x,-camera_y+(*bullet).y,1,0,1,r,g,b);
			}
			//yellow beam
			else if(mod.general_weapons[(*bullet).type].trace1==5){
				r=1;
				g=1;
				b=0;
				text_manager.draw_line(-camera_x+(*bullet).x4,-camera_y+(*bullet).y4,-camera_x+(*bullet).x,-camera_y+(*bullet).y,1,0,1,r,g,b);
			}


		}
		//the bullet
		resources.Texture_Set(mod.general_weapons[j].texture);
		grim->Quads_Begin();
		for(it=bullet_list.begin(); it!=bullet_list.end(); ++it){
			list<bullet>::iterator bullet=*it;

			if((*bullet).type!=j)continue;

			//draw
			float size=mod.general_weapons[(*bullet).type].size*6;
			if(size==0)continue;
			grim->Quads_SetColor(1,1,1,1);
			grim->Quads_SetRotation((*bullet).angle);
			grim->Quads_Draw(-camera_x+(*bullet).x-size/2, -camera_y+(*bullet).y-size/2, size, size);
			grim->Quads_SetColor(1,1,1,0.5f);
			grim->Quads_Draw(-camera_x+(*bullet).x2-size/2, -camera_y+(*bullet).y2-size/2, size, size);

		}
		grim->Quads_End();
	}
}
/*
void game_engine::text_manager.draw_line(float x,float y, float x2, float y2, float width, float fade0, float fade1, float r, float g, float b){
	resources.Texture_Set(line_texture);
	grim->Quads_Begin();
		//grim->Quads_SetColor(1,1,1,1);
		grim->Quads_SetColorVertex(0,r,g,b,fade0);
		grim->Quads_SetColorVertex(1,r,g,b,fade0);
		grim->Quads_SetColorVertex(2,r,g,b,fade1);
		grim->Quads_SetColorVertex(3,r,g,b,fade1);

		float dq=atan2(x-x2,y-y2);
		float sin=sincos.table_sin(dq+pi/2);
		float cos=sincos.table_cos(dq+pi/2);
		grim->Quads_Draw4V(x+sin*width,y+cos*width,x-sin*width,y-cos*width,
			x2-sin*width,y2-cos*width,x2+sin*width,y2+cos*width);
	grim->Quads_End();
}*/

void game_engine::calculate_lights(void){//calculates all the map lighting

	int i,a,b,c;

	//near_fire=false;

	float c_x,c_y;
	float distance,angle;
	float light_value,size;


	//daylight
	{

		daylight[0]=sin(day_timer/mod.general_climates[map_main->climate_number].light_oscillate_time*pi*2-mod.general_climates[map_main->climate_number].light_phase_r-pi/4)*mod.general_climates[map_main->climate_number].light_amplitude_r+0.5f;
		daylight[1]=sin(day_timer/mod.general_climates[map_main->climate_number].light_oscillate_time*pi*2-mod.general_climates[map_main->climate_number].light_phase_g-pi/4)*mod.general_climates[map_main->climate_number].light_amplitude_g+0.5f;
		daylight[2]=sin(day_timer/mod.general_climates[map_main->climate_number].light_oscillate_time*pi*2-mod.general_climates[map_main->climate_number].light_phase_b-pi/4)*mod.general_climates[map_main->climate_number].light_amplitude_b+0.5f;

		//night
		if(daylight[0]<mod.general_climates[map_main->climate_number].light_min_r)daylight[0]=mod.general_climates[map_main->climate_number].light_min_r;
		if(daylight[1]<mod.general_climates[map_main->climate_number].light_min_g)daylight[1]=mod.general_climates[map_main->climate_number].light_min_g;
		if(daylight[2]<mod.general_climates[map_main->climate_number].light_min_b)daylight[2]=mod.general_climates[map_main->climate_number].light_min_b;

		//day
		if(daylight[0]>mod.general_climates[map_main->climate_number].light_max_r)daylight[0]=mod.general_climates[map_main->climate_number].light_max_r;
		if(daylight[1]>mod.general_climates[map_main->climate_number].light_max_g)daylight[1]=mod.general_climates[map_main->climate_number].light_max_g;
		if(daylight[2]>mod.general_climates[map_main->climate_number].light_max_b)daylight[2]=mod.general_climates[map_main->climate_number].light_max_b;

		/*daylight[0]=sincos.table_sin(day_timer/day_speed*pi*2)*0.5f*2+0.5f;
		daylight[1]=daylight[0]-0.3f;
		daylight[2]=daylight[0]-0.3f;

		//night
		if(daylight[0]<0.0f)daylight[0]=0.0f;
		if(daylight[1]<0.0f)daylight[1]=0.0f;
		if(daylight[2]<0.2f)daylight[2]=0.2f;
		//day
		if(daylight[0]>0.7f)daylight[0]=0.7f;
		if(daylight[1]>0.4f)daylight[1]=0.4f;
		if(daylight[2]>0.4f)daylight[2]=0.4f;*/
	}


	//all lights in the scene start at 0
	int draw_start_x=(int)(camera_x/grid_size)-3;
	int draw_start_y=(int)(camera_y/grid_size)-3;
	int draw_end_x=draw_start_x+(int)(screen_width/grid_size)+6;
	int draw_end_y=draw_start_y+(int)(screen_height/grid_size)+6;
	/*if(draw_start_x<0)border_visible=true;
	if(draw_start_y<0)border_visible=true;
	if(draw_end_x>=map_main->sizex)border_visible=true;
	if(draw_end_y>=map_main->sizey)border_visible=true;*/

	if(draw_start_x<0)draw_start_x=0;
	if(draw_end_x>map_main->sizex)draw_end_x=map_main->sizex;
	if(draw_start_y<0)draw_start_y=0;
	if(draw_end_y>map_main->sizey)draw_end_y=map_main->sizey;



	for(a=draw_start_x;a<draw_end_x;a++){
		for(b=draw_start_y;b<draw_end_y;b++){
			//zero object light
			//if(map_main->grid[a].grid[b].total_objects>0)
			for(c=0;c<map_main->grid[a].grid[b].objects.size();c++){
				map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]=daylight[0]+light_addition[0][2]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_r;
				map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]=daylight[1]+light_addition[1][2]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_g;
				map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]=daylight[2]+light_addition[2][2]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_b;
			}
			//zero item light
			for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
				map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]=daylight[0]+light_addition[0][1]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_r;
				map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]=daylight[1]+light_addition[1][1]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_g;
				map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]=daylight[2]+light_addition[2][1]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_b;

			}
			//zero map light (or 1 for lava)
			/*if(has_terrain_effect(map_main,map_main->grid[a].grid[b].terrain_type,3,&parameter0,&parameter1)){
				map_main->grid[a].grid[b].light_rgb[0]=1+light_addition[0][0];
				map_main->grid[a].grid[b].light_rgb[1]=1+light_addition[1][0];
				map_main->grid[a].grid[b].light_rgb[2]=1+light_addition[2][0];
			}
			else*/{
				map_main->grid[a].grid[b].light_rgb[0]=daylight[0]+light_addition[0][0]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_r;
				map_main->grid[a].grid[b].light_rgb[1]=daylight[1]+light_addition[1][0]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_g;
				map_main->grid[a].grid[b].light_rgb[2]=daylight[2]+light_addition[2][0]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_b;
			}

			//zero creature lights
			if(map_main->grid[a].grid[b].total_creatures>0)
			for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
				map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]=daylight[0]+light_addition[0][3]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_r;
				map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]=daylight[1]+light_addition[1][3]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_g;
				map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]=daylight[2]+light_addition[2][3]+mod.terrain_types[map_main->grid[a].grid[b].terrain_type].base_b;
			}



		}
	}

	//goggles on green
	/*if(goggles_active==1){
		for(a=draw_start_x;a<draw_end_x;a++){
			for(b=draw_start_y;b<draw_end_y;b++){


				float light_wave=sincos.table_sin(day_timer*30)*0.01f+0.99f;


				//zero object light
				if(map_main->grid[a].grid[b].total_objects>0)
				for(c=0;c<map_main->grid[a].grid[b].total_objects;c++){
					map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]=0.2f;
					map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]=light_wave;
					map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]=0.2f;
				}
				//zero item light
				for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
					map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]=0.5f;
					map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]=light_wave;
					map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]=0.5f;

				}
				//zero map light
				map_main->grid[a].grid[b].light_rgb[0]=0.4f;
				map_main->grid[a].grid[b].light_rgb[1]=0.7f;
				map_main->grid[a].grid[b].light_rgb[2]=0.4f;

				//zero creature lights
				if(map_main->grid[a].grid[b].total_creatures>0)
				for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
					map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]=0.6f;
					map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]=1;
					map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]=0.6f;
				}
			}
		}
	}*/

	//calculate all lights
	for(i=0;i<map_main->lights.size();i++){
		if(map_main->lights[i].dead){
			//map_main->lights.erase(map_main->lights.begin() + i);
			delete_light(map_main,i);
			i--;
			continue;
		}
		//time
		if(map_main->lights[i].time!=-1){
			map_main->lights[i].time=map_main->lights[i].time-elapsed*game_speed;
			if(map_main->lights[i].time<0)map_main->lights[i].dead=true;
		}




		//the texture is visible by default
		//map_main->lights[i].visible=true;

		//flashlight
		if(mod.general_lights[map_main->lights[i].type].type==0){


			int alku_x=(int)(map_main->lights[i].x2/grid_size-map_main->lights[i].size*3)-2;
			int alku_y=(int)(map_main->lights[i].y2/grid_size-map_main->lights[i].size*3)-2;
			int loppu_x=(int)(map_main->lights[i].x2/grid_size+map_main->lights[i].size*3)+2;
			int loppu_y=(int)(map_main->lights[i].y2/grid_size+map_main->lights[i].size*3)+2;

			//not visible
			if(alku_x>screen_end_x){continue;}
			if(alku_y>screen_end_y){continue;}
			if(loppu_x<screen_start_x){continue;}
			if(loppu_y<screen_start_y){continue;}

			//not all should be calculated
			if(alku_x<screen_start_x-2)alku_x=screen_start_x-2;
			if(alku_y<screen_start_y-2)alku_y=screen_start_y-2;
			if(loppu_x>screen_end_x+2)loppu_x=screen_end_x+2;
			if(loppu_y>screen_end_y+2)loppu_y=screen_end_y+2;

			if(alku_x<0)alku_x=0;
			if(loppu_x>map_main->sizex)loppu_x=map_main->sizex;
			if(alku_y<0)alku_y=0;
			if(loppu_y>map_main->sizey)loppu_y=map_main->sizey;




			for(a=alku_x;a<loppu_x;a++){
				for(b=alku_y;b<loppu_y;b++){
					//objects
					//if(map_main->grid[a].grid[b].total_objects>0)
					for(c=0;c<map_main->grid[a].grid[b].objects.size();c++){
						size=map_main->object[map_main->grid[a].grid[b].objects[c]].size*general_object_size;
						c_x=map_main->object[map_main->grid[a].grid[b].objects[c]].x+size*0.5f;
						c_y=map_main->object[map_main->grid[a].grid[b].objects[c]].y+size*0.5f;
						//if(border_visible)fix_coordinates(&c_x,&c_y);
						light_value=mod.general_lights[map_main->lights[i].type].intensity*calculate_flashlight(map_main->lights[i].x2,map_main->lights[i].y2,map_main->lights[i].rotation,c_x,c_y,&distance,&angle);
						map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]+=light_value*map_main->lights[i].r*light_level;
						map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]+=light_value*map_main->lights[i].g*light_level;
						map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]+=light_value*map_main->lights[i].b*light_level;
					}
					//items
					for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
						size=map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size;
						c_x=map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f;
						c_y=map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f;
						//if(border_visible)fix_coordinates(&c_x,&c_y);
						light_value=mod.general_lights[map_main->lights[i].type].intensity*calculate_flashlight(map_main->lights[i].x2,map_main->lights[i].y2,map_main->lights[i].rotation,c_x,c_y,&distance,&angle);
						map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]+=light_value*map_main->lights[i].r*light_level;
						map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]+=light_value*map_main->lights[i].g*light_level;
						map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]+=light_value*map_main->lights[i].b*light_level;
					}
					//grid
					c_x=a*128.0f;
					c_y=b*128.0f;
					if(border_visible)fix_coordinates(&c_x,&c_y);
					light_value=mod.general_lights[map_main->lights[i].type].intensity*calculate_flashlight(map_main->lights[i].x2,map_main->lights[i].y2,map_main->lights[i].rotation,c_x,c_y,&distance,&angle);
					map_main->grid[a].grid[b].light_rgb[0]+=light_value*map_main->lights[i].r*light_level;
					map_main->grid[a].grid[b].light_rgb[1]+=light_value*map_main->lights[i].g*light_level;
					map_main->grid[a].grid[b].light_rgb[2]+=light_value*map_main->lights[i].b*light_level;
					//creatures
					if(map_main->grid[a].grid[b].total_creatures>0)
					for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
						if(map_main->grid[a].grid[b].creatures[c]==0)continue;//doesn't affect player
						c_x=map_main->creature[map_main->grid[a].grid[b].creatures[c]].x;
						c_y=map_main->creature[map_main->grid[a].grid[b].creatures[c]].y;
						//if(border_visible)fix_coordinates(&c_x,&c_y);
						light_value=mod.general_lights[map_main->lights[i].type].intensity*calculate_flashlight(map_main->lights[i].x2,map_main->lights[i].y2,map_main->lights[i].rotation,c_x,c_y,&distance,&angle);
						map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]+=light_value*map_main->lights[i].r*creature_light_value;
						map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]+=light_value*map_main->lights[i].g*creature_light_value;
						map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]+=light_value*map_main->lights[i].b*creature_light_value;
					}
				}
			}
		}
		//omni
		if(mod.general_lights[map_main->lights[i].type].type==1){


			int alku_x=(int)(map_main->lights[i].x2/grid_size-map_main->lights[i].size*4)-2;
			int alku_y=(int)(map_main->lights[i].y2/grid_size-map_main->lights[i].size*4)-2;
			int loppu_x=(int)(map_main->lights[i].x2/grid_size+map_main->lights[i].size*4)+2;
			int loppu_y=(int)(map_main->lights[i].y2/grid_size+map_main->lights[i].size*4)+2;

			//not visible
			if(alku_x>screen_end_x){continue;}
			if(alku_y>screen_end_y){continue;}
			if(loppu_x<screen_start_x){continue;}
			if(loppu_y<screen_start_y){continue;}

			//not all should be calculated
			if(alku_x<screen_start_x-2)alku_x=screen_start_x-2;
			if(alku_y<screen_start_y-2)alku_y=screen_start_y-2;
			if(loppu_x>screen_end_x+2)loppu_x=screen_end_x+2;
			if(loppu_y>screen_end_y+2)loppu_y=screen_end_y+2;

			if(alku_x<0)alku_x=0;
			if(loppu_x>map_main->sizex)loppu_x=map_main->sizex;
			if(alku_y<0)alku_y=0;
			if(loppu_y>map_main->sizey)loppu_y=map_main->sizey;




			//float distance=sqr(map_main->lights[i].x-c_x)+sqr(map_main->lights[i].y-c_y);
			for(a=alku_x;a<loppu_x;a++){
				for(b=alku_y;b<loppu_y;b++){

					float intensity=(map_main->lights[i].size*0.75f+0.25f);


					//objects
					//if(map_main->grid[a].grid[b].total_objects>0)
					for(c=0;c<map_main->grid[a].grid[b].objects.size();c++){
						size=map_main->object[map_main->grid[a].grid[b].objects[c]].size*general_object_size;
						c_x=map_main->object[map_main->grid[a].grid[b].objects[c]].x+size*0.5f;
						c_y=map_main->object[map_main->grid[a].grid[b].objects[c]].y+size*0.5f;
						//fix_coordinates(&c_x,&c_y);
						//light_value=map_main->lights[i].size*map_main->lights[i].pulse*(1-0.00000000004f*(sqr(sqr(map_main->lights[i].x2-c_x))+sqr(sqr(map_main->lights[i].y2-c_y))));
						light_value=intensity*map_main->lights[i].pulse*(mod.general_lights[map_main->lights[i].type].intensity-0.00000000004f*(sqr(sqr(map_main->lights[i].x2-c_x))+sqr(sqr(map_main->lights[i].y2-c_y))));
						if(light_value>0){
							map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]+=light_value*map_main->lights[i].r;
							map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]+=light_value*map_main->lights[i].g;
							map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]+=light_value*map_main->lights[i].b;
						}
					}
					//items
					for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
						size=map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size;
						c_x=map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f;
						c_y=map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f;
						//fix_coordinates(&c_x,&c_y);
						light_value=intensity*map_main->lights[i].pulse*(mod.general_lights[map_main->lights[i].type].intensity-0.00000000004f*(sqr(sqr(map_main->lights[i].x2-c_x))+sqr(sqr(map_main->lights[i].y2-c_y))));
						if(light_value>0){
							map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]+=light_value*map_main->lights[i].r;
							map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]+=light_value*map_main->lights[i].g;
							map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]+=light_value*map_main->lights[i].b;
						}
					}
					//grid
					c_x=(a)*grid_size+grid_size*0.5f;
					c_y=(b)*grid_size+grid_size*0.5f;
					light_value=intensity*map_main->lights[i].pulse*(mod.general_lights[map_main->lights[i].type].intensity-0.00000000004f*(sqr(sqr(map_main->lights[i].x2-c_x))+sqr(sqr(map_main->lights[i].y2-c_y))));
					if(light_value>0){
						map_main->grid[a].grid[b].light_rgb[0]+=light_value*map_main->lights[i].r*light_level;
						map_main->grid[a].grid[b].light_rgb[1]+=light_value*map_main->lights[i].g*light_level;
						map_main->grid[a].grid[b].light_rgb[2]+=light_value*map_main->lights[i].b*light_level;
					}
					//creatures
					for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
						size=mod.general_creatures[map_main->creature[map_main->grid[a].grid[b].creatures[c]].type].size*map_main->creature[map_main->grid[a].grid[b].creatures[c]].size*general_creature_size;
						c_x=map_main->creature[map_main->grid[a].grid[b].creatures[c]].x+size*0.5f;
						c_y=map_main->creature[map_main->grid[a].grid[b].creatures[c]].y+size*0.5f;
						//fix_coordinates(&c_x,&c_y);
						light_value=intensity*map_main->lights[i].pulse*(mod.general_lights[map_main->lights[i].type].intensity-0.00000000001f*4*(sqr(sqr(map_main->lights[i].x2-c_x))+sqr(sqr(map_main->lights[i].y2-c_y))));
						if(light_value>0){
							map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]+=light_value*map_main->lights[i].r*light_level;
							map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]+=light_value*map_main->lights[i].g*light_level;
							map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]+=light_value*map_main->lights[i].b*light_level;
						}
					}
				}
			}

		}

		//pulsating
		if(mod.general_lights[map_main->lights[i].type].pulsating>0){
			map_main->lights[i].pulse=1+sincos.table_sin(time_from_beginning*mod.general_lights[map_main->lights[i].type].pulsating+map_main->lights[i].pulse_phase)*0.04f*randDouble(0.0f,1.01f);
		}
		if(mod.general_lights[map_main->lights[i].type].particle>=0){
			if(!paused)
			if(randInt(0,mod.general_lights[map_main->lights[i].type].particle_flash_speed/(elapsed*map_main->lights[i].size))==0){
				make_particle(mod.general_lights[map_main->lights[i].type].particle,1,mod.general_lights[map_main->lights[i].type].particle_time+randInt(-250,250),map_main->lights[i].x+randDouble(map_main->lights[i].size*general_object_size*0.3f,map_main->lights[i].size*general_object_size*(1-0.3f)),map_main->lights[i].y+randDouble(map_main->lights[i].size*general_object_size*0.3f,map_main->lights[i].size*general_object_size*(1-0.3f)),randDouble(-0.01f,0.01f),randDouble(-0.01f,0.01f));
				//if(randInt(0,5)==0)
				//	playsound(fire_sound,randDouble(0.6f,1.0f),map_main->lights[i].x,map_main->lights[i].y,player_middle_x,player_middle_y);
			}

		}
	}

	//grids are not too bright
	for(a=draw_start_x;a<draw_end_x;a++){
		for(b=draw_start_y;b<draw_end_y;b++){
			//object lights not too brigh
			//if(map_main->grid[a].grid[b].total_objects>0)
			for(c=0;c<map_main->grid[a].grid[b].objects.size();c++){
				if(map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]>1)map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]=1;
				if(map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]>1)map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]=1;
				if(map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]>1)map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]=1;

				if(map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]<0)map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[0]=0;
				if(map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]<0)map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[1]=0;
				if(map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]<0)map_main->object[map_main->grid[a].grid[b].objects[c]].light_level[2]=0;

			}
			//item lights not too brigh
			for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
				if(map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]>1)map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]=1;
				if(map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]>1)map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]=1;
				if(map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]>1)map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]=1;

				if(map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]<0)map_main->items[map_main->grid[a].grid[b].items[c]].light_level[0]=0;
				if(map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]<0)map_main->items[map_main->grid[a].grid[b].items[c]].light_level[1]=0;
				if(map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]<0)map_main->items[map_main->grid[a].grid[b].items[c]].light_level[2]=0;

			}
			//creature lights not too brigh
			//if(map_main->grid[a].grid[b].total_creatures>0)
			for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]>1)map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]=1;
				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]>1)map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]=1;
				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]>1)map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]=1;

				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]<0)map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[0]=0;
				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]<0)map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[1]=0;
				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]<0)map_main->creature[map_main->grid[a].grid[b].creatures[c]].light[2]=0;

			}

			//grid not too bright
			if(map_main->grid[a].grid[b].light_rgb[0]>1)map_main->grid[a].grid[b].light_rgb[0]=1;
			if(map_main->grid[a].grid[b].light_rgb[1]>1)map_main->grid[a].grid[b].light_rgb[1]=1;
			if(map_main->grid[a].grid[b].light_rgb[2]>1)map_main->grid[a].grid[b].light_rgb[2]=1;

			if(map_main->grid[a].grid[b].light_rgb[0]<0)map_main->grid[a].grid[b].light_rgb[0]=0;
			if(map_main->grid[a].grid[b].light_rgb[1]<0)map_main->grid[a].grid[b].light_rgb[1]=0;
			if(map_main->grid[a].grid[b].light_rgb[2]<0)map_main->grid[a].grid[b].light_rgb[2]=0;

		}
	}


}

//map coordinate checker
void game_engine::fix_coordinates(float *c_x, float *c_y){
	if((camera_x<0)&&(grid_size*map_main->sizex+camera_x<*c_x))
		*c_x-=grid_size*map_main->sizex;
	else if((camera_x+screen_width>grid_size*map_main->sizex)&&(camera_x-grid_size*map_main->sizex+screen_width>*c_x))
		*c_x+=grid_size*map_main->sizex;

	if((camera_y<0)&&(grid_size*map_main->sizey+camera_y<*c_y))
		*c_y-=grid_size*map_main->sizey;
	else if((camera_y+screen_height>grid_size*map_main->sizey)&&(camera_y-grid_size*map_main->sizey+screen_height>*c_y))
		*c_y+=grid_size*map_main->sizey;
}


void game_engine::make_particle(int type, int layer, float time, float x,float y,float vx,float vy, float angle)//particle animation starter
{
	//int a,b;

	if(x-camera_x<0)return;
	if(x-camera_x>screen_width)return;
	if(y-camera_y<0)return;
	if(y-camera_y>screen_height)return;

		particle temp_particle;
		/*a=0;
		b=0;
		alku2:
		a=randInt(0,maximum_particles);
		b=b+1;
		if(b>maximum_particles)map_main->particles[a].dead=true;
		if(!map_main->particles[a].dead)goto alku2;*/

		//ZeroMemory(&map_main->particles[a], sizeof(map_main->particles[a]));
		//map_main->particles[a].dead=false;
		temp_particle.x=x;
		temp_particle.y=y;
		temp_particle.move_x=vx;
		temp_particle.move_y=vy;
		temp_particle.time=time;
		temp_particle.time_start=time;
		//map_main->particles[a].type=type;
		//map_main->particles[a].layer=layer;
		if(angle==-1)
			temp_particle.rotate=randDouble(0,2*pi);
		else
			temp_particle.rotate=angle;

		particles[type].particles_list[layer].push_back(temp_particle);

}

void game_engine::calculate_particles(void){
	int a,b;

	list<particle>::iterator it;

	//for(i=0;i<maximum_particles;i++){
	for(a=0;a<particles.size();a++){
		if(particles[a].dead)continue;
		for(b=0;b<3;b++){
			for(it=particles[a].particles_list[b].begin(); it!=particles[a].particles_list[b].end();){

				(*it).time-=elapsed*game_speed;
				(*it).x+=(*it).move_x*elapsed*game_speed;
				(*it).y+=(*it).move_y*elapsed*game_speed;

				if((*it).time<0){
					//blood stays on the floor for a while
					if(particles[a].stop_when_hit_ground>=0){
						(*it).time=randInt(1000,3000);
						(*it).move_x=0;
						(*it).move_y=0;

						//move to new type
						particles[particles[a].stop_when_hit_ground].particles_list[b].push_back(*it);
						//remove from list
						it=particles[a].particles_list[b].erase(it);
						continue;
					}
					else{
						//remove from list
						it=particles[a].particles_list[b].erase(it);
						continue;
					}
				} else {
					it++;
				}
			}
		}
	}
}

void game_engine::draw_particles(int layer){//draws particles

	//int particle_types=12;
	float particle_type_size;
	float size;
	bool time_changes_size;
	bool unique_rotate;
	float particle_x, particle_y;
	//int i,j;
	int a;
	grim->System_SetState_Blending(true);
	grim->Quads_SetSubset(0,0,1,1);
/*
	if(layer==0){
		grim->System_SetState_BlendSrc(grBLEND_INVSRCCOLOR);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	}
	if(layer==1){
		grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_ONE);
	}*/
/*
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);*/

	//add all particles to list
	/*
	for(i=0;i<maximum_particles;i++){
		if(map_main->particles[i].dead)continue;
		if(map_main->particles[i].layer!=layer)continue;
		particles_list.push_back(i);
	}
	list<int>::iterator it;*/

	list<particle>::iterator it;

	//resources.Texture_Set(particle_texture);
	grim->Quads_SetRotation(0);
	for(a=0;a<particles.size();a++){
		if(particles[a].dead)continue;


		grim->Quads_SetColor(particles[a].r,particles[a].g,particles[a].b,particles[a].alpha);
		time_changes_size=(particles[a].gets_smaller_by_time!=0);
		unique_rotate=particles[a].can_be_rotated==1;
		size=particles[a].size;
		particle_type_size=particles[a].size;
		resources.Texture_Set(particles[a].texture);

		if(particles[a].blend_type==0){
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
		}
		else if(particles[a].blend_type==1){
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_ONE);
		}


		grim->Quads_Begin();

		//for(b=0;b<3;b++){
			for(it=particles[a].particles_list[layer].begin(); it!=particles[a].particles_list[layer].end();){

				particle_x=(*it).x;
				particle_y=(*it).y;


				//it might not need to be drawn
				if((*it).x-camera_x<-500){it=particles[a].particles_list[layer].erase(it);continue;}
				if((*it).x-camera_x>screen_width+500){it=particles[a].particles_list[layer].erase(it);continue;}
				if((*it).y-camera_y<-500){it=particles[a].particles_list[layer].erase(it);continue;}
				if((*it).y-camera_y>screen_height+500){it=particles[a].particles_list[layer].erase(it);continue;}

				if(time_changes_size){
					if(particles[a].gets_smaller_by_time>0){
						size=((*it).time/(*it).time_start)*particles[a].gets_smaller_by_time*particle_type_size;
					}
					if(particles[a].gets_smaller_by_time<0){
						size=particle_type_size+((*it).time/(*it).time_start)*particles[a].gets_smaller_by_time*particle_type_size;
					}

					if(size>particle_type_size)size=particle_type_size;
				}
				if(size<1){it++;continue;}

				if(unique_rotate){
					grim->Quads_SetRotation((*it).rotate);
				}

				//draw
				grim->Quads_Draw(-camera_x+particle_x-size*0.5f, -camera_y+particle_y-size*0.5f, size, size);
				it++;
			}

		grim->Quads_End();
	}
}

void game_engine::draw_mouse(int cursor, float hot_spot_x, float hot_spot_y, float r, float g, float b){//draws mouse


	grim->System_SetState_Blending(true);
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(mouse_texture[cursor]);
	grim->Quads_SetColor(r,g,b,1);
	grim->Quads_Begin();
	//draw
	if(input_override)
		grim->Quads_Draw(override_mousex-hot_spot_x*32, override_mousey-hot_spot_y*32, 32, 32);
	else
		grim->Quads_Draw(mousex-hot_spot_x*32, mousey-hot_spot_y*32, 32, 32);
	grim->Quads_End();


}


void game_engine::create_plot_object(map* map_to_edit, int general_item_class, float x, float y, float angle, float size){//throws in the items to the map

	//int c,i,j;




	int a=general_item_class;
	item temp_item;

	memset(&temp_item, 0, sizeof(temp_item));
	temp_item.dead=false;
	temp_item.event_used=false;
	temp_item.visible=true;
	temp_item.rotation=angle;
	if(angle<0)temp_item.rotation=randDouble(0,2*pi);
	temp_item.light_level[0]=1;
	temp_item.light_level[1]=1;
	temp_item.light_level[2]=1;
	temp_item.create_time=time_from_beginning;//mod.general_plot_objects[a].live_time;
	//effect effect;
	temp_item.base_type=0;
	//temp_item.base_type=plot_object_has_effect(a,3,&effect);//if it's an item or a plot_object

/*	tempstring="Initializing Plot Object ";
	tempstring+=itoa(general_item_class,temprivi,10);
	debug.debug_output(tempstring,1);*/


	//plot_objects
	if(temp_item.base_type==0){
		temp_item.type=mod.general_plot_objects[a].object_definition_number;
		temp_item.item_type=a;
		if(size<=0)
			temp_item.size=randDouble(mod.general_objects[temp_item.type].base_size,mod.general_objects[temp_item.type].base_size+mod.general_objects[temp_item.type].vary_size);
		else temp_item.size=size;
		temp_item.sway_power=randDouble(0,pi);
		temp_item.sway_phase=randDouble(0.1f,8.0f);
		temp_item.amount=1;
		temp_item.interval_timer=time_from_beginning*1000;

	}
	/*
	//items
	if(temp_item.base_type==1){
		temp_item.type=effect.parameter1;
		temp_item.item_type=a;
		temp_item.size=mod.general_items[(int)effect.parameter1].size;
		temp_item.sway_power=0;
		temp_item.sway_phase=0;
		temp_item.amount=effect.parameter2;
	}
	*/


	//random location
	if((x<0)||(y<0)){

		//random, not on unpassable objects or hazardous terrain
		if(mod.general_plot_objects[a].location_type==0){

			int place_bad=100;
			while(place_bad>0){
				place_bad=false;
				temp_item.x=randDouble(grid_size*2,map_to_edit->sizex*grid_size-grid_size*3);
				temp_item.y=randDouble(grid_size*2,map_to_edit->sizey*grid_size-grid_size*3);

				//see if it's on some other terrain type than normal ground
				//if(map_to_edit->grid[(int)(temp_item.x/grid_size)].grid[(int)(temp_item.y/grid_size)].type!=0)
				//float parameter0,parameter1;
				//if(terrain_maps[]map_to_edit->grid[(int)(temp_item.x/grid_size)].grid[(int)(temp_item.y/grid_size)].,4,&parameter0,&parameter1))
				if(mod.terrain_maps[mod.general_areas[map_to_edit->area_type].terrain_map_number].terrain_grid[(int)(temp_item.y/grid_size)].terrain_blocks[(int)(temp_item.x/grid_size)].no_random_items==1)
					place_bad--;
				else if(mod.terrain_types[mod.terrain_maps[mod.general_areas[map_to_edit->area_type].terrain_map_number].terrain_grid[(int)(temp_item.y/grid_size)].terrain_blocks[(int)(temp_item.x/grid_size)].terrain_type].do_not_place_random_objects)
					place_bad--;
				else if(point_will_collide(map_to_edit,temp_item.x-temp_item.size*general_object_size*0.5f,y-temp_item.size*general_object_size*0.5f,false))
					place_bad--;
				else
					place_bad=0;
			}
		}
		//distance from player start
		if(mod.general_plot_objects[a].location_type==1){

			float rand_angle=randDouble(0,2*pi);
			temp_item.x=map_to_edit->creature[0].x+sincos.table_sin(rand_angle)*randDouble(mod.general_plot_objects[a].location_parameter1,mod.general_plot_objects[a].location_parameter2)-temp_item.size*general_object_size*0.5f;
			temp_item.y=map_to_edit->creature[0].y+sincos.table_cos(rand_angle)*randDouble(mod.general_plot_objects[a].location_parameter1,mod.general_plot_objects[a].location_parameter2)-temp_item.size*general_object_size*0.5f;


		}
		//distance from object
		if(mod.general_plot_objects[a].location_type==2){



			int found=-1;
			int d;
			//list all items
			vector <int> temp_items;
			for(d=0;d<map_to_edit->items.size();d++){
				if(map_to_edit->items[d].dead)continue;
				if(map_to_edit->items[d].base_type!=0)continue;
				if(map_to_edit->items[d].item_type!=mod.general_plot_objects[a].location_parameter0)continue;
				temp_items.push_back(d);
			}
			//find the object that this object should be located from
			if(temp_items.size()>0){
				d=randInt(0,temp_items.size());
				found=temp_items[d];
			}

			if(found>=0){
				float rand_angle=randDouble(0,2*pi);
				temp_item.x=map_to_edit->items[found].x+map_to_edit->items[found].size*general_object_size*0.5f+sincos.table_sin(rand_angle)*randDouble(mod.general_plot_objects[a].location_parameter1,mod.general_plot_objects[a].location_parameter2)-temp_item.size*general_object_size*0.5f;
				temp_item.y=map_to_edit->items[found].y+map_to_edit->items[found].size*general_object_size*0.5f+sincos.table_cos(rand_angle)*randDouble(mod.general_plot_objects[a].location_parameter1,mod.general_plot_objects[a].location_parameter2)-temp_item.size*general_object_size*0.5f;
			}

			//no suitable place found
			else
				return;
		}
		//coordinates
		if(mod.general_plot_objects[a].location_type==3){
			temp_item.x=map_to_edit->sizex*grid_size*mod.general_plot_objects[a].location_parameter0-temp_item.size*general_object_size*0.5f;
			temp_item.y=map_to_edit->sizey*grid_size*mod.general_plot_objects[a].location_parameter1-temp_item.size*general_object_size*0.5f;
			temp_item.rotation=map_to_edit->sizey*grid_size*mod.general_plot_objects[a].location_parameter2;
		}
	}
	//preset location
	else{

		//make sure location is within map
		/*if(x<grid_size*1.5f)
			x=grid_size*1.5f;
		if(x>map_to_edit->sizex*grid_size-grid_size*2.5f)
			x=map_to_edit->sizex*grid_size-grid_size*2.5f;
		if(y<grid_size*1.5f)
			y=grid_size*1.5f;
		if(y>map_to_edit->sizey*grid_size-grid_size*2.5f)
			y=map_to_edit->sizey*grid_size-grid_size*2.5f;*/


		temp_item.x=x-temp_item.size*0.5f*general_object_size;
		temp_item.y=y-temp_item.size*0.5f*general_object_size;



	}




	//make sure object is within map
	/*if(temp_item.x<grid_size*1.5f)
		temp_item.x=grid_size*1.5f;
	if(temp_item.x>map_to_edit->sizex*grid_size-grid_size*2.5f)
		temp_item.x=map_to_edit->sizex*grid_size-grid_size*2.5f;
	if(temp_item.y<grid_size*1.5f)
		temp_item.y=grid_size*1.5f;
	if(temp_item.y>map_to_edit->sizey*grid_size-grid_size*2.5f)
		temp_item.y=map_to_edit->sizey*grid_size-grid_size*2.5f;*/

	if(temp_item.x<0)
		return;
	if(temp_item.x>=map_to_edit->sizex*grid_size)
		return;
	if(temp_item.y<0)
		return;
	if(temp_item.y>=map_to_edit->sizey*grid_size)
		return;


	if((time_from_beginning<=0)&&(mod.general_objects[temp_item.type].animation_frames.size()>0)){
		//start with random frame
		temp_item.current_animation_frame=randInt(0,mod.general_objects[temp_item.type].animation_frames.size());

		//set time to current plus random to next frame
		temp_item.animation_frame_time=randDouble(0,mod.general_objects[temp_item.type].animation_frames[temp_item.current_animation_frame].time);
	}
	else{
		//start with random frame
		temp_item.current_animation_frame=0;

		//set time to current plus random to next frame
		temp_item.animation_frame_time=0;
	}




	//place to grid
	int temp_x=(int)(temp_item.x/grid_size);
	int temp_y=(int)(temp_item.y/grid_size);

	temp_item.light_level[0]=map_to_edit->grid[temp_x].grid[temp_y].light_rgb[0];
	temp_item.light_level[1]=map_to_edit->grid[temp_x].grid[temp_y].light_rgb[1];
	temp_item.light_level[2]=map_to_edit->grid[temp_x].grid[temp_y].light_rgb[2];

	map_to_edit->items.push_back(temp_item);
	map_to_edit->grid[temp_x].grid[temp_y].items.push_back(map_to_edit->items.size()-1);


}



void game_engine::create_item(map* map_to_edit, int item_type, int amount, float x, float y, float rotation){//throws in the items to the map

	int i;


	//make sure location is within map
	if(x<grid_size*1.5f)
		x=grid_size*1.5f;
	if(x>map_to_edit->sizex*grid_size-grid_size*2.5f)
		x=map_to_edit->sizex*grid_size-grid_size*2.5f;
	if(y<grid_size*1.5f)
		y=grid_size*1.5f;
	if(y>map_to_edit->sizey*grid_size-grid_size*2.5f)
		y=map_to_edit->sizey*grid_size-grid_size*2.5f;





	//no such item found, create new
	item temp_item;

	memset(&temp_item, 0, sizeof(temp_item));
	temp_item.dead=false;
	temp_item.event_used=false;
	temp_item.visible=true;
	if(rotation==-1)
		temp_item.rotation=randDouble(0,2*pi);
	else
		temp_item.rotation=rotation;
	temp_item.light_level[0]=1;
	temp_item.light_level[1]=1;
	temp_item.light_level[2]=1;
	temp_item.base_type=1;

	temp_item.type=item_type;
	temp_item.item_type=item_type;
	temp_item.size=mod.general_items[item_type].size;
	temp_item.sway_power=0;
	temp_item.sway_phase=0;
	temp_item.amount=amount;

	temp_item.x=x-temp_item.size*0.5f*general_object_size;
	temp_item.y=y-temp_item.size*0.5f*general_object_size;

	//find if such an item exists near the location, and abbreviate it to that if possible
	int temp_x=(int)(temp_item.x/grid_size);
	int temp_y=(int)(temp_item.y/grid_size);
	int nearest=-1;
	float nearest_distance=0;
	for(i=0;i<map_to_edit->grid[temp_x].grid[temp_y].items.size();i++){
		int item=map_to_edit->grid[temp_x].grid[temp_y].items[i];
		if(map_to_edit->items[item].base_type==1)
		if(map_to_edit->items[item].type==item_type){
			float distance=sqr(map_to_edit->items[item].x-temp_item.x)+sqr(map_to_edit->items[item].y-temp_item.y);
			if(nearest==-1){
				nearest=item;
				nearest_distance=distance;
			}
			else{
				if(nearest_distance>distance){
					nearest=item;
					nearest_distance=distance;
				}
			}
		}
	}
	if((nearest>=0)&&(nearest_distance<100)){
		map_to_edit->items[nearest].amount+=amount;
		return;
	}




	//make sure object is within map
	if(temp_item.x<grid_size*1.5f)
		temp_item.x=grid_size*1.5f;
	if(temp_item.x>map_to_edit->sizex*grid_size-grid_size*2.5f)
		temp_item.x=map_to_edit->sizex*grid_size-grid_size*2.5f;
	if(temp_item.y<grid_size*1.5f)
		temp_item.y=grid_size*1.5f;
	if(temp_item.y>map_to_edit->sizey*grid_size-grid_size*2.5f)
		temp_item.y=map_to_edit->sizey*grid_size-grid_size*2.5f;


	//place item to vector
	map_to_edit->items.push_back(temp_item);

	//place to grid
	temp_x=(int)(temp_item.x/grid_size);
	temp_y=(int)(temp_item.y/grid_size);
	map_to_edit->grid[temp_x].grid[temp_y].items.push_back(map_to_edit->items.size()-1);


}


//draws the pop-up window
void game_engine::draw_pop_up(void){

	int a,b;


	//text view mode
	if(pop_up_mode==0){
		draw_text_view();
		return;
	}

	//item view mode
	if(pop_up_mode==3){
		draw_item_view();
		return;
	}


	pop_up_x=(1024-256)*x_multiplier;
	pop_up_y=(768-256)*y_multiplier;

	bool accept_mouse_input=true;//for disabling the right button when the mode changes

	//int mahtuu=6; // (unused)

	//additional info box
	bool additional_info=false;
	string add_info;

	//computer texture
	float small_computer_size=48.0f;
	float transparency=pop_up_transparency;
	float pop_up_leveys=18;
	grim->Quads_SetSubset(0,(256.0f-small_computer_size-18)/256.0f,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(mod.general_races[player_race].interface_texture);
	grim->Quads_SetColor(1,1,1,transparency);
	grim->Quads_Begin();
	//draw small computer
	grim->Quads_Draw(pop_up_x, pop_up_y+(256.0f-small_computer_size-18)*y_multiplier, 256*x_multiplier, (small_computer_size+18)*y_multiplier);
	grim->Quads_End();

	//weapon symbol
	if(weapon_selected_from_item_list>=0){
		resources.Texture_Set(mod.general_items[inventory[active_inventory].player_items[weapon_selected_from_item_list].item].texture);
		//find_texture_coordinates(mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon[map_main->creature[0].weapon_selected]].symbol_number,&x0,&y0,&x1,&y1,4);
		grim->Quads_SetSubset(0,0,1,1);
		grim->Quads_Begin();
		grim->Quads_Draw(pop_up_x+23*x_multiplier, pop_up_y+219*y_multiplier, 32*x_multiplier, 32*y_multiplier);
		grim->Quads_End();
	}

	//item mode button
	bool goto_item_mode=false;
	if((mousex>=pop_up_x+218*x_multiplier)&&(mousex<pop_up_x+244*x_multiplier)){
		if((mousey>=pop_up_y+224*y_multiplier)&&(mousey<pop_up_y+249*y_multiplier)){
			additional_info=true;
			add_info="Right click for inventory";
			//click
			if(!mouse_right&&mouse_right2){
				playsound(UI_game_click[0],1,0,0,0,0);
				goto_item_mode=true;

			}
		}
	}
	//can get to item mode by x or i keys
	/*if((key_x&&!key_x2)||(key_i&&!key_i2)){
		playsound(menu_click[0],1,0,0,0,0);
		goto_item_mode=true;
	}*/
	//we can go to item mode
	if(goto_item_mode&&can_view_inventory){
		previous_pop_up_mode=pop_up_mode;
		pop_up_mode=3;
		accept_mouse_input=false;
	}

	//journal mode button
	if((mousex>=pop_up_x+192*x_multiplier)&&(mousex<pop_up_x+218*x_multiplier)){
		if((mousey>=pop_up_y+224*y_multiplier)&&(mousey<pop_up_y+249*y_multiplier)){
			additional_info=true;
			if(pop_up_mode==1)add_info="Right click to review journal";
			//click
			if(!mouse_right&&mouse_right2){
				playsound(UI_game_click[0],1,0,0,0,0);
				show_text_window(current_showing_entry);
				return;
			}
		}
	}
	if(key_j&&!key_j2){
		playsound(UI_game_click[0],1,0,0,0,0);
		show_text_window(current_showing_entry);
		return;
	}


	//draw the dot on the map
	float dot_size=4;
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	if(player_controlled_creature==0)
	if(can_draw_map)
	if(grim->System_SetRenderTarget(map_main->map_texture)){

		//darken the trails
		if(darken_timer>0){
			darken_timer-=elapsed*game_speed;
		}
		else{
			darken_timer=5000;
			grim->System_SetState_Blending(true);
			grim->Texture_Set(map_main->map_texture_2);
			grim->Quads_SetColor(1,1,1,0.02f);
			grim->Quads_Begin();
				grim->Quads_Draw(0, 0, map_size_x, map_size_y);
			grim->Quads_End();

		}

		//player dot
		resources.Texture_Set(mapdot);
		grim->System_SetState_Blending(true);
		grim->Quads_SetColor(0.3f,1,0,0.8f);
		//current location
		float multiplier_x=map_size_x/((map_main->sizex)*grid_size);
		float multiplier_y=map_size_y/((map_main->sizey)*grid_size);
		float x=(player_middle_x)*multiplier_x-dot_size/2;
		float y=(player_middle_y)*multiplier_y-dot_size/2;
		grim->Quads_Begin();
			grim->Quads_Draw(x, y, dot_size, dot_size);
		grim->Quads_End();
	}
	grim->System_SetRenderTarget(-1);

	//draw the window
	//map
	if(show_radar)
	{
		if(pop_up_mode==0)transparency=0.8f;
		grim->System_SetState_Blending(true);
		resources.Texture_Set(mod.general_races[player_race].interface_texture);
		grim->Quads_SetSubset((17/256.0f),0,1,(256.0f-small_computer_size-15)/256.0f);
		grim->Quads_SetColor(1,1,1,0.4f);
		grim->Quads_Begin();
		grim->Quads_Draw(pop_up_x+(29-pop_up_leveys)*x_multiplier, pop_up_y+26*y_multiplier, (0.76f*256+pop_up_leveys*2)*x_multiplier, (0.76f*256)*y_multiplier);
		grim->Quads_End();

		string show_map_name="";

		if(pop_up_mode==1){

			float map_start_x=24;
			float map_start_y=32;
			float map_widthx=204;
			float map_widthy=173;

			//change map mode
			/*if(!mouse_right&&mouse_right2){
				if((mousex>pop_up_x+map_start_x)&&(mousex<pop_up_x+map_start_x+map_widthx))
					if((mousey>pop_up_y+map_start_y)&&(mousey<pop_up_y+map_start_y+map_widthy)){
						if(map_active)
							map_active=false;
						else{
							//find if the player has the long range scanner
							for(int i=0;i<inventory[active_inventory].player_items.size();i++){
								effect effect;
								if(item_has_effect(inventory[active_inventory].player_items[i].item,20,&effect)){
									use_item(inventory[active_inventory].player_items[i].item,&i,effect,false);
									break;
								}
							}
						}
					}
			}*/

			//long range scanner minimizes map
			if(map_active){
				//left
				if(map_main->near_areas[0]>=0){
					draw_line_map(pop_up_x+(map_start_x)*x_multiplier,pop_up_y+(map_start_y+map_widthy/3)*y_multiplier,map_widthx/3*x_multiplier,map_widthy/3*y_multiplier,map_storage[map_main->near_areas[0]]);
					if((mousex>pop_up_x+(map_start_x)*x_multiplier)&&(mousey>pop_up_y+(map_start_y+map_widthy/3)*y_multiplier)&&(mousex<pop_up_x+(map_start_x+map_widthx/3)*x_multiplier)&&(mousey<pop_up_y+(map_start_y+map_widthy/3+map_widthy/3)*y_multiplier))
						show_map_name=mod.general_areas[map_storage[map_main->near_areas[0]]->area_type].name;
				}
				//right
				if(map_main->near_areas[1]>=0){
					draw_line_map(pop_up_x+(map_start_x+map_widthx/3*2)*x_multiplier,pop_up_y+(map_start_y+map_widthy/3)*y_multiplier,map_widthx/3*x_multiplier,map_widthy/3*y_multiplier,map_storage[map_main->near_areas[1]]);
					if((mousex>pop_up_x+(map_start_x+map_widthx/3*2)*x_multiplier)&&(mousey>pop_up_y+(map_start_y+map_widthy/3)*y_multiplier)&&(mousex<pop_up_x+(map_start_x+map_widthx/3*2+map_widthx/3)*x_multiplier)&&(mousey<pop_up_y+(map_start_y+map_widthy/3+map_widthy/3)*y_multiplier))
						show_map_name=mod.general_areas[map_storage[map_main->near_areas[1]]->area_type].name;
				}
				//up
				if(map_main->near_areas[2]>=0){
					draw_line_map(pop_up_x+(map_start_x+map_widthx/3)*x_multiplier,pop_up_y+(map_start_y)*y_multiplier,map_widthx/3*x_multiplier,map_widthy/3*y_multiplier,map_storage[map_main->near_areas[2]]);
					if((mousex>pop_up_x+(map_start_x+map_widthx/3)*x_multiplier)&&(mousey>pop_up_y+(map_start_y)*y_multiplier)&&(mousex<pop_up_x+(map_start_x+map_widthx/3+map_widthx/3)*x_multiplier)&&(mousey<pop_up_y+(map_start_y+map_widthy/3)*y_multiplier))
						show_map_name=mod.general_areas[map_storage[map_main->near_areas[2]]->area_type].name;
				}
				//down
				if(map_main->near_areas[3]>=0){
					draw_line_map(pop_up_x+(map_start_x+map_widthx/3)*x_multiplier,pop_up_y+(map_start_y+map_widthy/3*2)*y_multiplier,map_widthx/3*x_multiplier,map_widthy/3*y_multiplier,map_storage[map_main->near_areas[3]]);
					if((mousex>pop_up_x+(map_start_x+map_widthx/3)*x_multiplier)&&(mousey>pop_up_y+(map_start_y+map_widthy/3*2)*y_multiplier)&&(mousex<pop_up_x+(map_start_x+map_widthx/3+map_widthx/3)*x_multiplier)&&(mousey<pop_up_y+(map_start_y+map_widthy/3*2+map_widthy/3))*y_multiplier)
						show_map_name=mod.general_areas[map_storage[map_main->near_areas[3]]->area_type].name;
				}
				//middle
				if((mousex>pop_up_x+(map_start_x+map_widthx/3)*x_multiplier)&&(mousey>pop_up_y+(map_start_y+map_widthy/3)*y_multiplier)&&(mousex<pop_up_x+(map_start_x+map_widthx/3+map_widthx/3)*x_multiplier)&&(mousey<pop_up_y+(map_start_y+map_widthy/3+map_widthy/3)*y_multiplier))
					show_map_name=mod.general_areas[map_main->area_type].name;

				map_start_x=map_start_x+map_widthx/3;
				map_start_y=map_start_y+map_widthy/3;
				map_widthx=map_widthx/3;
				map_widthy=map_widthy/3;

				//show_map_name=mod.general_areas[map_main->area_type].name;
			}
			else{
				show_map_name=mod.general_areas[map_main->area_type].name;
			}

			//mouse is on the map, brighten it
			bool brighten_map=false;
			if((mousex>pop_up_x+map_start_x*x_multiplier)&&(mousex<pop_up_x+(map_start_x+map_widthx)*x_multiplier)){
				if((mousey>pop_up_y+map_start_y*y_multiplier)&&(mousey<pop_up_y+(map_start_y+map_widthy)*y_multiplier)){
					brighten_map=true;
				}
			}

			//find correct dimensions of the map to draw
			float center_x=map_start_x+map_widthx/2;
			float center_y=map_start_y+map_widthy/2;
			float proposed_map_widthx=(map_widthy/map_main->sizey)*map_main->sizex;
			float proposed_map_widthy=(map_widthx/map_main->sizex)*map_main->sizey;
			if(proposed_map_widthx<map_widthx)map_widthx=proposed_map_widthx;
			if(proposed_map_widthy<map_widthy)map_widthy=proposed_map_widthy;
			map_start_x=center_x-map_widthx/2;
			map_start_y=center_y-map_widthy/2;
			//204 ,173
			//draw the map
			if(can_draw_map){
				grim->Quads_SetSubset(0,0,1,1);

				if(brighten_map){
					grim->Quads_SetColor(1,1,1,1);
				}
				else{
					grim->Quads_SetColor(1,1,1,0.7f);
				}
				grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
				grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
				grim->Texture_Set(map_main->map_texture);
				grim->Quads_Begin();
					//grim->Quads_Draw(pop_up_x+29-pop_up_leveys, pop_up_y+26, 0.76f*256+pop_up_leveys*2, 0.76f*256);
					grim->Quads_Draw(pop_up_x+map_start_x*x_multiplier, pop_up_y+map_start_y*y_multiplier, map_widthx*x_multiplier, map_widthy*y_multiplier);
				grim->Quads_End();
			}

			//passable edges
			draw_line_map(pop_up_x+map_start_x*x_multiplier,pop_up_y+map_start_y*y_multiplier,map_widthx*x_multiplier,map_widthy*y_multiplier,map_main);

			grim->System_SetState_Blending(true);
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
			grim->Quads_SetRotation(0);
			/*float multiplier_x=(map_widthx-map_start_x)/(map_main->sizex*grid_size)*(1+1.47f*0.1f);
			float multiplier_y=(map_widthy-map_start_y)/(map_main->sizey*grid_size)*(1+2.14f*0.1f);*/
			float multiplier_x=(map_widthx)/((map_main->sizex)*grid_size);
			float multiplier_y=(map_widthy)/((map_main->sizey)*grid_size);
			grim->Quads_SetSubset(0,0,1,1);

			//kill_meter_active=2000;
			//scanner_active=2000;

			//show nearby aliens
			shortest_distance_to_alien=1000000000;
			if(kill_meter_active>0){
				int i,j,k;
				float max_draw_distance=kill_meter_active;

				//check area
				int alku_x=(int)((player_middle_x-max_draw_distance)/grid_size);
				int alku_y=(int)((player_middle_y-max_draw_distance)/grid_size);
				int loppu_x=(int)((player_middle_x+max_draw_distance)/grid_size);
				int loppu_y=(int)((player_middle_y+max_draw_distance)/grid_size);

				if(alku_x<0)alku_x=0;
				if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
				if(alku_y<0)alku_y=0;
				if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;


				//all the creatures here
				for(i=alku_x;i<loppu_x;i++){
					for(j=alku_y;j<loppu_y;j++){
						if(map_main->grid[i].grid[j].total_creatures>0)
						for(k=0;k<map_main->grid[i].grid[j].total_creatures;k++){

							int creature=map_main->grid[i].grid[j].creatures[k];
							if(creature==0)continue;
							if(map_main->creature[creature].killed)continue;
							//if(map_main->creature[creature].side==map_main->creature[0].side)continue;
							if(mod.general_creatures[map_main->creature[creature].type].particle_on_radar<0)continue;
							int particle_type=mod.general_creatures[map_main->creature[creature].type].particle_on_radar;
							float creature_size=mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size;

							dot_size=particles[particle_type].size;

							float x=(map_main->creature[creature].x+creature_size*0.5f)*multiplier_x-dot_size/2;
							float y=(map_main->creature[creature].y+creature_size*0.5f)*multiplier_y-dot_size/2;

							float distance=sqr(map_main->creature[creature].x-(player_middle_x))+sqr(map_main->creature[creature].y-(player_middle_y));

							if(distance<shortest_distance_to_alien)
								shortest_distance_to_alien=distance;

							float alpha=1-sqrtf(distance)/(max_draw_distance);
							if(alpha<0)alpha=0;

							//if((x>map_start_x)&&(x<map_widthx)&&(y>map_start_y)&&(y<map_widthy)){
							//grim->Quads_SetColor(particles[particle_type].r,particles[particle_type].g,particles[particle_type].b,alpha);
							resources.Texture_Set(particles[particle_type].texture);
							grim->Quads_Begin();
							grim->Quads_SetColor(0,0,0,alpha);
							grim->Quads_Draw(pop_up_x+(map_start_x+x-3)*x_multiplier, pop_up_y+(map_start_y+y-3)*y_multiplier, (dot_size+6)*x_multiplier, (dot_size+6)*y_multiplier);
							grim->Quads_SetColor(particles[particle_type].r,particles[particle_type].g,particles[particle_type].b,alpha);
							grim->Quads_Draw(pop_up_x+(map_start_x+x)*x_multiplier, pop_up_y+(map_start_y+y)*y_multiplier, dot_size*x_multiplier, dot_size*y_multiplier);
							grim->Quads_End();
						}
					}
				}
			}

			//scanner
			{
				float max_draw_distance=scanner_active;

				//all the plot items here
				for(int i=0;i<map_main->items.size();i++){

					if(map_main->items[i].dead)continue;

					int show_on_type=0;
					int particle_type=0;
					float size=map_main->items[i].size*general_object_size;

					//it's a plot_object
					if(map_main->items[i].base_type==0){
						if(map_main->items[i].event_used)continue;
						if(map_main->items[i].type==0)continue;//null object visual
						//if(mod.general_plot_objects[map_main->items[i].item_type].event==-1)continue;
						show_on_type=mod.general_plot_objects[map_main->items[i].item_type].show_on_radar;
						particle_type=mod.general_plot_objects[map_main->items[i].item_type].show_on_radar_particle;
					}
					//it's an item
					else if(map_main->items[i].base_type==1){
						show_on_type=mod.general_items[map_main->items[i].type].show_on_radar;
						particle_type=mod.general_items[map_main->items[i].type].show_on_radar_particle;
					}

					if(show_on_type!=0)
					{
						//no scanner and item does not show without one
						if((scanner_active==0)&&(show_on_type==1))continue;

						float x=map_main->items[i].x+size*0.5f;
						float y=map_main->items[i].y+size*0.5f;
						float distance=sqr(x-(player_middle_x))+sqr(y-(player_middle_y));

						if((distance<sqr(max_draw_distance))||(show_on_type==2)){

							float alpha;
							if((show_on_type==1)){
								alpha=1-sqrtf(distance)/(max_draw_distance);
								if(alpha<0)alpha=0;
							}
							else {
								alpha=1;
							}

							dot_size=particles[particle_type].size;
							grim->Quads_SetColor(particles[particle_type].r,particles[particle_type].g,particles[particle_type].b,alpha);
							resources.Texture_Set(particles[particle_type].texture);
							grim->Quads_Begin();
							grim->Quads_SetColor(0,0,0,alpha);
							grim->Quads_Draw(pop_up_x+(map_start_x+x*multiplier_x-dot_size*0.5f-3)*x_multiplier, pop_up_y+(map_start_y+y*multiplier_y-dot_size*0.5f-3)*y_multiplier, (dot_size+6)*x_multiplier, (dot_size+6)*y_multiplier);
							grim->Quads_SetColor(particles[particle_type].r,particles[particle_type].g,particles[particle_type].b,alpha);
							grim->Quads_Draw(pop_up_x+(map_start_x+x*multiplier_x-dot_size*0.5f)*x_multiplier, pop_up_y+(map_start_y+y*multiplier_y-dot_size*0.5f)*y_multiplier, (dot_size)*x_multiplier, (dot_size)*y_multiplier);
							grim->Quads_End();
						}
					}
				}
			}

			//player dot
			dot_size=4;
			resources.Texture_Set(mapdot);
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
			grim->Quads_SetColor(0,0,0,1);
			float x=(player_middle_x)*multiplier_x-dot_size;
			float y=(player_middle_y)*multiplier_y-dot_size;
			grim->Quads_Begin();
				grim->Quads_Draw(pop_up_x+(map_start_x+x-3)*x_multiplier, pop_up_y+(map_start_y+y-3)*y_multiplier, (dot_size*2+6)*x_multiplier, (dot_size*2+6)*y_multiplier);
			grim->Quads_End();

			grim->Quads_SetColor(1,1,1,1);
			grim->Quads_Begin();
				grim->Quads_Draw(pop_up_x+(map_start_x+x)*x_multiplier, pop_up_y+(map_start_y+y)*y_multiplier, (dot_size*2)*x_multiplier, (dot_size*2)*y_multiplier);
			grim->Quads_End();

			//map name
			if(show_map_name!=""){
				text_manager.write(font,show_map_name,1*x_multiplier,pop_up_x+(24+3)*x_multiplier, pop_up_y+(32+3)*y_multiplier,screen_width,screen_height,false,0.8f,1.0f,0.8f,0.9f);
			}
		}
	}

	//small weapon info
	grim->Quads_SetColor(0.8f,1,0.8f,1);
	if(mod.general_creatures[map_main->creature[0].type].weapon>=0){
		text_manager.write_line(font,pop_up_x+(54)*x_multiplier, pop_up_y+(228)*y_multiplier, mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].name,1*x_multiplier);

		//show bullet amount for bullet using weapons
		for(a=0;a<mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].wield_conditions.size();a++){

			//has an item requirement
			if(mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].wield_conditions[a].condition_number==0){
				//find if there is an item with this identifier
				int inventory_slot=-1;
				for(b=0;b<inventory[active_inventory].player_items.size();b++){
					if((inventory[active_inventory].player_items[b].item==mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].wield_conditions[a].condition_parameter0)&&(inventory[active_inventory].player_items[b].amount>0)){
						inventory_slot=b;
						break;
					}
				}

				if(inventory_slot>=0){
					//itoa(inventory[active_inventory].player_items[inventory_slot].amount,temprivi,10);
					sprintf(temprivi,"%d",inventory[active_inventory].player_items[inventory_slot].amount);

					tempstring="Ammo:";
					tempstring+=temprivi;
					text_manager.write_line(font,pop_up_x+20*x_multiplier, pop_up_y+190*y_multiplier, tempstring, 1*x_multiplier);
				}
				else{
					tempstring="Ammo:0";
					text_manager.write_line(font,pop_up_x+20*x_multiplier, pop_up_y+190*y_multiplier, tempstring, 1*x_multiplier);
				}
			}
		}

	}
	else{
		//text_manager.write_line(font,pop_up_x+54, pop_up_y+228, "No weapon",1);
	}
	grim->Quads_SetColor(1,1,1,1);

	//additional info
	if(additional_info){

		int teksti_alku_x=24;
		int teksti_alku_y=30;
		int teksti_loppu_x=200;
		int teksti_loppu_y=250;

		if(!show_radar){
			teksti_alku_y+=183;
			teksti_loppu_y+=183;
		}

		grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
		transparency=pop_up_transparency;
		grim->System_SetState_Blending(true);
		resources.Texture_Set(mod.general_races[player_race].interface_texture);
		grim->Quads_SetSubset((17/256.0f),0,1,(256.0f-small_computer_size-15)/256.0f);
		grim->Quads_SetColor(1,1,1,transparency);
		grim->Quads_Begin();
		if(!show_radar){
			grim->Quads_Draw(pop_up_x+(29-pop_up_leveys)*x_multiplier, pop_up_y+(26)*y_multiplier, (0.76f*256+pop_up_leveys*2)*x_multiplier, (0.76f*256)*y_multiplier);
		}
		else{
			grim->Quads_Draw(pop_up_x+(29-pop_up_leveys)*x_multiplier, pop_up_y+(26-183)*y_multiplier, (0.76f*256+pop_up_leveys*2)*x_multiplier, (0.76f*256)*y_multiplier);
		}
		grim->Quads_End();

		text_manager.write(font,add_info,1*x_multiplier,pop_up_x+(teksti_alku_x)*x_multiplier,pop_up_y+(teksti_alku_y-183)*y_multiplier,pop_up_x+(teksti_loppu_x-15)*x_multiplier,pop_up_y+(teksti_loppu_y-183-30)*y_multiplier,false,0.8f,1.0f,0.8f,1);
		//text_manager.write(font,"aaaa fawe fwe ",0.7f,pop_up_x+teksti_alku_x,pop_up_y+teksti_alku_y+50,pop_up_x+teksti_loppu_x,pop_up_y+teksti_loppu_y,true,1,1,1);
	}
}

//checks the items and objects on the map
void game_engine::calculate_items(void){
	int i,j,k;


	//time plot_objects
	int alku_x=0;
	int alku_y=0;
	int loppu_x=map_main->sizex-1;
	int loppu_y=map_main->sizey-1;
	for(i=alku_x;i<loppu_x;i++){
		for(j=alku_y;j<loppu_y;j++){
			for(k=0;k<map_main->grid[i].grid[j].items.size();k++){
				if(map_main->items[map_main->grid[i].grid[j].items[k]].dead)continue;
				bool must_delete=false;
				if(map_main->items[map_main->grid[i].grid[j].items[k]].base_type==0){


					int item=map_main->grid[i].grid[j].items[k];


					//activate by time
					if(mod.general_plot_objects[map_main->items[map_main->grid[i].grid[j].items[k]].item_type].trigger_event_by==4)
					while(time_from_beginning*1000-map_main->items[item].interval_timer>=mod.general_plot_objects[map_main->items[item].item_type].trigger_event_parameter1){

						map_main->items[item].interval_timer+=mod.general_plot_objects[map_main->items[item].item_type].trigger_event_parameter1;



						//for(j=0;j<mod.general_plot_objects[map_main->items[i].item_type].effects.size();j++){



							//map_main->items[i].parameter0+=elapsed*game_speed*0.001f;

							//effect effect=mod.general_plot_objects[map_main->items[i].item_type].effects[j];


						must_delete=run_plot_object(item);

						if(must_delete){
							delete_plot_object(map_main,item, i, j, k);
							k--;
							break;
						}

						//must happen on every frame, must skip here to not end up in an endless loop
						if(mod.general_plot_objects[map_main->items[item].item_type].trigger_event_parameter1<=0)
							break;

						//}
					}

					//see if it's time to destroy this object
					if(mod.general_plot_objects[map_main->items[item].item_type].live_time>0)
					if(time_from_beginning-map_main->items[item].create_time>mod.general_plot_objects[map_main->items[item].item_type].live_time*0.001f){
						delete_plot_object(map_main,item, i, j, k);
						k--;
						continue;
					}
				}


			}
		}
	}




	//check area
	alku_x=(int)(camera_x/grid_size)-1;
	alku_y=(int)(camera_y/grid_size)-1;
	loppu_x=alku_x+(int)(screen_width/grid_size)+2;
	loppu_y=alku_y+(int)(screen_height/grid_size)+2;

	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;



	//all the plot items here
	for(int pass=0;pass<2;pass++){
		for(i=alku_x;i<loppu_x;i++){
			for(j=alku_y;j<loppu_y;j++){
				for(k=0;k<map_main->grid[i].grid[j].items.size();k++){
					if(map_main->items[map_main->grid[i].grid[j].items[k]].dead)continue;

					int item=map_main->grid[i].grid[j].items[k];



					bool does_something=false;
					int activate_by=0;//trigger event by 0=player click, 1=player near, 2=near or click

					//it's an item
					if(map_main->items[item].base_type==1){
						//no items on second pass
						if(pass==1)continue;

						//check if it's outside borders
						float size=map_main->items[item].size*general_object_size;
						float object_x=map_main->items[item].x+size*0.5f;
						float object_y=map_main->items[item].y+size*0.5f;
						if(object_x<grid_size){map_main->items[item].x=grid_size-size*0.5f;}
						if(object_y<grid_size){map_main->items[item].y=grid_size-size*0.5f;}
						if(object_x>(map_main->sizex-1)*grid_size-grid_size){map_main->items[item].x=(map_main->sizex-1)*grid_size-grid_size-size*0.5f;}
						if(object_y>(map_main->sizey-1)*grid_size-grid_size){map_main->items[item].y=(map_main->sizey-1)*grid_size-grid_size-size*0.5f;}


						does_something=true;
						activate_by=0;//items are always activated by clicking

					}
					//it's a plot_object
					else{
						//no plot_objects on first pass
						if(pass==0)continue;



						//there's an event for this object
						if(!map_main->items[item].event_used)
						if(mod.general_plot_objects[map_main->items[item].item_type].effects.size()>0){
							does_something=true;
							activate_by=mod.general_plot_objects[map_main->items[item].item_type].trigger_event_by;
						}

						//play radiate sound
						//if(!sample[mod.general_plot_objects[map_main->items[item].item_type].sound]->IsSoundPlaying())
						if(mod.general_plot_objects[map_main->items[item].item_type].sound>=0){
							map_main->items[item].sound_timer-=elapsed*game_speed;
							if(map_main->items[item].sound_timer<=0){
								float size=map_main->items[item].size*general_object_size;
								playsound(mod.general_plot_objects[map_main->items[item].item_type].sound,1,map_main->items[item].x+size*0.5f,map_main->items[item].y+size*0.5f,player_middle_x,player_middle_y);
								map_main->items[item].sound_timer=resources.sample[mod.general_plot_objects[map_main->items[item].item_type].sound]->GetSoundLength()*randDouble(0.6f,1.3f);
							}
						}
					}



					if(does_something){
						bool fire_event=false;

						float size=map_main->items[item].size*general_object_size;
						float distance = sqr(map_main->items[item].x+size*0.5f-player_middle_x)+sqr(map_main->items[item].y+size*0.5f-player_middle_y);

						//if event is triggered by coming close
						if((activate_by==1)||(activate_by==2)){
							if(distance<(sqr(0.5f*size)))
								fire_event=true;
						}

						//if the mouse is close enough we can show the name
						bool mouse_on=false;
						if((activate_by==0)||(activate_by==2)){
							//for items
							if(map_main->items[item].base_type==1){
								if(sqr(map_main->items[item].x+size*0.5f-(camera_x+mousex))+sqr(map_main->items[item].y+size*0.5f-(camera_y+mousey))<sqr(size*0.35f))
									mouse_on=true;
							}
							//for plot_objects
							else{
								float point_x=mousex+camera_x;
								float point_y=mousey+camera_y;

								if(point_in_object(point_x,point_y,&map_main->items[item]))
									mouse_on=true;
							}

						}


						if(mouse_on){

							mouse_on_item=true;

							float touch_size=size;
							if(touch_size<80)touch_size=80;
							//float distance = sqr(map_main->items[item].x+size*0.5f-player_middle_x)+sqr(map_main->items[item].y+size*0.5f-player_middle_y);


							if(map_main->items[item].base_type==0)
								tempstring=mod.general_plot_objects[map_main->items[item].item_type].name;
							else if(map_main->items[item].base_type==1){
								tempstring=mod.general_items[map_main->items[item].type].name;
								if(map_main->items[item].amount>1){
									tempstring+=" (";
									//tempstring+=itoa(map_main->items[item].amount,temprivi,10);
									sprintf(temprivi,"%d",map_main->items[item].amount);
									tempstring+=temprivi;
									tempstring+=")";
								}

							}

							//if the player is close enough we can show the use text
							//if(mod.general_plot_objects[map_main->items[item].item_type].event!=8)//item is not a resting place
							if(distance<sqr(touch_size)){
								tempstring+=" (Right click to use/pick up)";
								if(!mouse_right&&mouse_right2){
									mouse_right2=false;
									fire_event=true;
									//use_item(map_main->items[item].item_type, map_main->creature[0].x, map_main->creature[0].y);
								}
							}
							else
								tempstring+=" (Too far to use)";

							text_manager.message(1000,1000,tempstring);

						}



						//we can finally do something
						if(fire_event){

							//find event type
								//it's a plot_object
								if(map_main->items[item].base_type==0){

									bool must_delete=run_plot_object(item);

									if(must_delete){
										delete_plot_object(map_main,item, i, j, k);
										k--;
									}


								}

								//it's an item
								else if(map_main->items[item].base_type==1){
									give_item(map_main->items[item].type,map_main->items[item].amount,time_from_beginning, true);

									//event has been used now, delete the object
									playsound(pick_up,1,0,0,0,0);
									delete_plot_object(map_main,item, i, j, k);
									k--;
								}

						}
					}
				}
			}
		}
	}
}

//changes the sizes of objects to reflect the object infos
void game_engine::vary_object_sizes(map* map_to_edit){
	int k;
	for(k=0;k<map_to_edit->object.size();k++){
		map_to_edit->object[k].size=randDouble(mod.general_objects[map_to_edit->object[k].type].base_size,mod.general_objects[map_to_edit->object[k].type].base_size+mod.general_objects[map_to_edit->object[k].type].vary_size);

		//if(mod.general_objects[map_to_edit->object[k].type].swing>0){
			map_to_edit->object[k].sway_phase=randDouble(0,pi);
			map_to_edit->object[k].sway_power=randDouble(0.1f,8.0f);
		//}
	}
}

//give player an item
bool game_engine::give_item(int item_number, int amount, float time, bool arrange){
	int a;

	if(item_number>=mod.general_items.size())
		return false;
	if(item_number<0)
		return false;

	//giving an item
	if(amount>0){
		//show text for first time give
		if(!seen_item_text[item_number]){
			if(mod.general_items[item_number].event_text!="none"){
				record_message(3,item_number);
				seen_item_text[item_number]=true;
				if(show_journals)show_text_window(journal_records.size()-1);
			}
		}

		//find if there already is an item with this identifier
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			if((inventory[active_inventory].player_items[a].item==item_number)&&(inventory[active_inventory].player_items[a].amount>0)){
				inventory[active_inventory].player_items[a].time_stamp=time;
				inventory[active_inventory].player_items[a].amount+=amount;
				arrange_item_list(false);
				return true;
			}
		}

		//no item found, put it to list
		item_list_object temp_item;
		temp_item.item=item_number;
		temp_item.time_stamp=time;
		temp_item.amount=amount;
		temp_item.wielded=false;
		//temp_item.used=false;
		inventory[active_inventory].player_items.push_back(temp_item);
	}
	//taking away
	if(amount<0){
		//find the item with this identifier
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			if((inventory[active_inventory].player_items[a].item==item_number)){
				//if there are enough items
				if(inventory[active_inventory].player_items[a].amount+amount<0)
					return false;

				//inventory[active_inventory].player_items[a].time_stamp=time;
				inventory[active_inventory].player_items[a].amount+=amount;
				/*if(inventory[active_inventory].player_items[a].amount<=0){
					delete_item(a);
				}*/
				arrange_item_list(false);
				return true;
			}
		}
		return false;
	}


	arrange_item_list(false);
	return true;


}

//deletes a plot_object from map
void game_engine::delete_plot_object(map *edit_map, int number, int i, int j, int k){

	if(k<0)return;
	if(k>=edit_map->grid[i].grid[j].items.size())return;

	edit_map->grid[i].grid[j].items.erase(edit_map->grid[i].grid[j].items.begin()+k);

	//edit_map->items.erase(edit_map->items.begin() + number);
	edit_map->items[number].dead=true;

}

//deletes an item from list
int game_engine::delete_item(int number){

	if(number<0)return 0;
	if(number>=inventory[active_inventory].player_items.size())return number;

	//set slots to unused
	if(inventory[active_inventory].player_items[number].wielded)
	for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[number].item].wield_slots.size();b++){
		inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[number].item].wield_slots[b]]=-1;
	}


	inventory[active_inventory].player_items.erase(inventory[active_inventory].player_items.begin() + number);

	//update slot used references
	for(int a=0;a<inventory[active_inventory].slot_used_by.size();a++){
		if(inventory[active_inventory].slot_used_by[a]>number)
			inventory[active_inventory].slot_used_by[a]--;

		else if(inventory[active_inventory].slot_used_by[a]==number)
			inventory[active_inventory].slot_used_by[a]=-1;
	}

	calculate_weight();

	return number-1;

}


//use item, returns true if item should be deleted after use
bool game_engine::use_item(int general_item_number,int *item_number_in_list, Mod::effect_base effect, bool unuse, bool output, bool just_asking, bool check_slots){

	int b;

	//check if player class cannot use this item class
	for(b=0;b<mod.general_races[player_race].disabled_item_classes.size();b++){
		if(mod.general_races[player_race].disabled_item_classes[b]==mod.general_items[general_item_number].item_class){
			if(output)text_manager.message(5000,2000,mod.general_races[player_race].disabled_item_classes_text[b]);
			return false;
		}
	}

	//is wieldable
	if(mod.general_items[general_item_number].wield_slots.size()>0){

		//if not undoing, check if slots are free
		if(!unuse){

			//if all needed slot are not available, cannot use
			for(b=0;b<mod.general_items[general_item_number].wield_slots.size();b++){

				//there is no such slot
				if(mod.general_items[general_item_number].wield_slots[b]>=mod.general_races[player_race].slots.size())return false;

				//race doesn't have the slot
				if(!mod.general_races[player_race].slots[mod.general_items[general_item_number].wield_slots[b]].active){
					if(output)text_manager.message(3000,1000,"You cannot use this.");
					return false;
				}

				//if the slot has been used
				if(!just_asking||check_slots)//if we're just asking if it can be used, we're not interested if the slot is free or not
				if(inventory[active_inventory].slot_used_by[mod.general_items[general_item_number].wield_slots[b]]>=0){
					if(output)text_manager.message(3000,1000,"Slot not available.");
					return false;
				}
			}
		}

		//if undoing, check if we have used this at all
		if(unuse){
			if(!inventory[active_inventory].player_items[*item_number_in_list].wielded)
				return false;
		}

	}

	//go throught all conditions
	bool OK=true;
	if(!unuse)
	for(b=0;b<effect.conditions.size();b++){
		if(!check_condition(effect.conditions[b],&map_main->creature[0],0,player_middle_x,player_middle_y,mod.general_items[general_item_number].show_condition_help)){
			OK=false;
			break;
		}
	}
	if(!OK){
		if(effect.event_failure_text!="none"){
			if(output)text_manager.message(5000,2000,effect.event_failure_text);
		}
		return false;
	}

	//has a weapon changing effect, see if the wield conditions are true
	if(!unuse)
	for(b=0;b<effect.effects.size();b++){
		if(effect.effects[b].effect_number==9){

			Mod::effect weapon_wear_effect=effect.effects[b];
			//check wield conditions for the proposed weapon
			bool OK=true;
			for(int d=0;d<mod.general_weapons[weapon_wear_effect.parameter1].wield_conditions.size();d++){
				if(!check_condition(mod.general_weapons[weapon_wear_effect.parameter1].wield_conditions[d],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
					OK=false;
					break;
				}
			}
			if(!OK){
				return false;
			}

			//find if this weapon class is disabled for player race
			if(mod.general_races[player_race].weapon_classes[mod.general_weapons[weapon_wear_effect.parameter1].weapon_class].can_use==0){
				return false;
			}
		}
	}

	//if we're just asking if this item can be used, return now
	if(just_asking)
		return true;

	//let's run the effects
	bool effect_ran=true;
	for(b=0;b<effect.effects.size();b++){
		effect_ran=run_effect(effect.effects[b],&map_main->creature[0],0,player_middle_x,player_middle_y,map_main->creature[0].rotation,unuse);
		if(!unuse)if(!effect_ran)break;
	}

	//find item slot again, as something may have messed it
	for(int i=0;i<inventory[active_inventory].player_items.size();i++){
		if(general_item_number==inventory[active_inventory].player_items[i].item){
			*item_number_in_list=i;
			break;
		}
	}

	//some effect failed
	if(!effect_ran){
		if(effect.event_failure_text!="none"){
			if(output)text_manager.message(5000,2000,effect.event_failure_text);
		}
		//set slots to unused
		for(b=0;b<mod.general_items[general_item_number].wield_slots.size();b++){
			inventory[active_inventory].slot_used_by[mod.general_items[general_item_number].wield_slots[b]]=-1;
		}
		inventory[active_inventory].player_items[*item_number_in_list].wielded=false;
		return false;
	}

	//if we're undoing, let's return immediately
	if(unuse){
		//set slots to unused
		for(b=0;b<mod.general_items[general_item_number].wield_slots.size();b++){
			inventory[active_inventory].slot_used_by[mod.general_items[general_item_number].wield_slots[b]]=-1;
		}
		inventory[active_inventory].player_items[*item_number_in_list].wielded=false;
		return false;
	}






	//message
	if(output)
	if(effect.event_text!="none"){
		//show normal message
		if(effect.event_text.length()<150){
			text_manager.message(5000,2000,effect.event_text);
		}
		//show journal entry
		else{
			log_text=effect.event_text;
			if(pop_up_mode!=0)
				previous_pop_up_mode=pop_up_mode;
			pop_up_mode=0;
		}
	}

	//play the sound
	if(output)playsound(effect.sound,1,0,0,0,0);

	//mark that item has been used
	//inventory[active_inventory].player_items[*item_number_in_list].used=true;

	//return whether the item should vanish or not
	if(effect.vanish_after_used==1){
		return true;
	}

	//set the item to used mode
	if(mod.general_items[general_item_number].wield_slots.size()>0){
		inventory[active_inventory].player_items[*item_number_in_list].wielded=true;
		//set slots to used
		for(b=0;b<mod.general_items[general_item_number].wield_slots.size();b++){
			inventory[active_inventory].slot_used_by[mod.general_items[general_item_number].wield_slots[b]]=*item_number_in_list;
		}
	}
	return false;

}

//runs effect
bool game_engine::run_effect(Mod::effect effect, creature_base *creature, int creature_number, float x, float y, float angle, bool undo){


	int a;
	bool arrange_needed=false;

	//if(mod.general_items[general_item_number].use_parameter1>0)

	bool return_value=false;
	switch ((int)effect.effect_number){
		case 0:
			//0=nothing
			return_value=true;
			break;
		case 1:
			//1=multiply speed by parameter3 for time parameter1 with creature visual effect from weapon parameter2, parameter4=disable speed change when bullet hits (0=no/1=yes)
			if(!undo){
				creature_base::weapon_effect temp_effect;
				temp_effect.effect_type=0;
				temp_effect.time=effect.parameter1;
				temp_effect.parameter0=effect.parameter3;
				temp_effect.parameter1=effect.parameter4;


				temp_effect.weapon_type=effect.parameter2;
				creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
				creature->weapon_effects_amount++;
				if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
					creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;

				return_value=true;
			}
			break;
		case 2:
			//2=start alien attack
			if(!undo){
				//radio_ending_timer=60000;
				alien_attack=true;

				return_value=true;
			}


			break;
		case 3:
			//3=drop item parameter1, amount parameter2, random area size parameter3 pixels, maximum amount of similar items in area parameter4 (0=infinite)
			if(!undo){

				//no area requirement
				if(effect.parameter3<=0){
					create_item(map_main,effect.parameter1,effect.parameter2,x,y,angle);
				}
				//area requirement
				else{
					//maximum amount requirement
					bool can_drop=true;
					if(effect.parameter4>0){
						int alku_x=(int)((x-effect.parameter4)/grid_size)-3;
						int alku_y=(int)((y-effect.parameter4)/grid_size)-3;
						int loppu_x=(int)((x+effect.parameter4)/grid_size)+3;
						int loppu_y=(int)((y+effect.parameter4)/grid_size)+3;

						if(alku_x<0)alku_x=0;
						if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
						if(alku_y<0)alku_y=0;
						if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

						//count amount of dispensed objects nearby
						int objects_here=0;
						for(int a=alku_x;a<loppu_x;a++){
							for(int b=alku_y;b<loppu_y;b++){
								for(int c=0;c<map_main->grid[a].grid[b].items.size();c++){
									if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=1)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
									float size=map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f<x-effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f>x+effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f<y-effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f>y+effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].item_type==effect.parameter1)
										objects_here++;
								}
							}
						}

						if(objects_here>=effect.parameter4)
							can_drop=false;
					}
					//now drop the item
					if(can_drop){
						create_item(map_main,effect.parameter1,effect.parameter2,x+randDouble(-effect.parameter3,effect.parameter3),y+randDouble(-effect.parameter3,effect.parameter3),angle);
					}



				}

				return_value=true;
			}

			break;
		case 4:
			//4=increase creature's bar parameter3 with parameter1 (if parameter2=1, don't increase over maximum or decrease below minimum)
			if(!undo){
				return_value=false;

				if(creature->bars[(int)effect.parameter3].active){

					//don't top at maximum
					if(effect.parameter2!=1){
						set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].value+effect.parameter1);
						return_value=true;
					}
					else{
						//increasing
						if(effect.parameter1>0){
							if(creature->bars[(int)effect.parameter3].value<creature->bars[(int)effect.parameter3].maximum){
								if(creature->bars[(int)effect.parameter3].maximum-creature->bars[(int)effect.parameter3].value<effect.parameter1){
									effect.parameter1=creature->bars[(int)effect.parameter3].maximum-creature->bars[(int)effect.parameter3].value;
								}
								set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].value+effect.parameter1);
								return_value=true;
							}
						}
						//decreasing
						else if(effect.parameter1<0){
							if(creature->bars[(int)effect.parameter3].value>creature->bars[(int)effect.parameter3].minimum){
								if(creature->bars[(int)effect.parameter3].minimum-creature->bars[(int)effect.parameter3].value>effect.parameter1){
									effect.parameter1=creature->bars[(int)effect.parameter3].minimum-creature->bars[(int)effect.parameter3].value;
								}
								set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].value+effect.parameter1);
								return_value=true;
							}
						}
					}

					//make sure no bar is below minimum
					if(creature->bars[(int)effect.parameter3].value<creature->bars[(int)effect.parameter3].minimum)
						set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].minimum);

				}
			}
			break;
		case 5:
			//5=activate scanner with distance parameter1
			if(!undo){
				if(scanner_active!=effect.parameter1){
					scanner_active=effect.parameter1;
				}
				else{
					scanner_active=0;
				}
			}
			else{
				scanner_active=0;
			}
			return_value=true;
			break;
		case 6:
			//6=set targeting beam, type parameter1 (0=disabled, 1=normal, 2=turns green when hits enemy), length = parameter2 + weapon length * parameter3
			if(!undo){
				beam_type=effect;
				beam_active=true;
			}
			else{
				beam_active=false;
			}
			return_value=true;
			break;
		case 7:
			//7=set light parameter1 size parameter2 to creature (if parameter1=-1, disable light) attached to (parameter3, 0=hands, 1=legs, 2=head)
			if(creature->dead)return false;
			if(!undo){
				//disable light
				if(effect.parameter1<0){
					if(creature->carry_light>=0)
						delete_light(map_main,creature->carry_light);
				}
				//enable light
				else{
					//delete old light
					if(creature->carry_light>=0)
						delete_light(map_main,creature->carry_light);

					//now activate it
						int type=effect.parameter1;
						creature->carry_light=map_main->create_light(creature->x,creature->y,type,effect.parameter2,mod.general_lights[type].r,mod.general_lights[type].g,mod.general_lights[type].b,mod.general_lights[type].a,-1);
						creature->light_attached_to=effect.parameter3;
						creature->carry_light_size=effect.parameter2;
						creature->carry_light_type=type;
						carry_light(map_main,creature,creature->carry_light);

					}
			}
			else{
				if(creature->carry_light>=0)
					delete_light(map_main,creature->carry_light);
			}
			return_value=true;
			break;
		case 8:
			//8=set light level addition, parameter1=type (0=map tiles, 1=items/plot_objects, 2=props, 3=creatures), parameter2=r, parameter3=g, parameter4=b
			if(!undo){
				light_addition[0][(int)effect.parameter1]+=effect.parameter2;
				light_addition[1][(int)effect.parameter1]+=effect.parameter3;
				light_addition[2][(int)effect.parameter1]+=effect.parameter4;
				calculate_lights();
			}
			else{
				light_addition[0][(int)effect.parameter1]-=effect.parameter2;
				light_addition[1][(int)effect.parameter1]-=effect.parameter3;
				light_addition[2][(int)effect.parameter1]-=effect.parameter4;

			}
			return_value=true;
			break;
		case 9:
			//9=select gun parameter1
			arrange_needed=true;
			if(!undo){

				Mod::specialty temp_specialty;

				//check wield conditions
				bool OK=true;
				for(int b=0;b<mod.general_weapons[effect.parameter1].wield_conditions.size();b++){
					if(!check_condition(mod.general_weapons[effect.parameter1].wield_conditions[b],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
						OK=false;
						break;
					}
				}
				if(!OK){
					return_value=false;
					break;
				}

				//find if this weapon class is disabled for player race
				if(mod.general_races[player_race].weapon_classes[mod.general_weapons[effect.parameter1].weapon_class].can_use==0){
					return_value=false;
					break;
				}

				//disable old weapon if there is one
				if(weapon_selected_from_item_list>=0){

					int temp_weapon=weapon_selected_from_item_list;
					if(inventory[active_inventory].player_items[temp_weapon].wielded){
						for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects.size();b++){
							use_item(inventory[active_inventory].player_items[temp_weapon].item, &temp_weapon, mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects[b].effect, true, false, false,false);
						}
					}
				}

				//select new weapon
				mod.general_creatures[map_main->creature[0].type].weapon=effect.parameter1;

				return_value=true;
			}
			else{
				mod.general_creatures[map_main->creature[0].type].weapon=-1;
				weapon_selected_from_item_list=-1;

				return_value=true;
			}

			break;
		case 10:
			//10=drop creature number parameter1 side parameter2 (-1=same side) tactic parameter3 (-1=default) tactic2 parameter4 (-1=default). Player can switch between a friendly creature's tactics by right clicking the creature. Don't forget to set parameter3 to -1 if you're not planning on changing the default tactic specified in creatures.dat.
			if(!undo){

				float angle;
				if(creature==NULL)angle=randDouble(0,2*pi);
				else angle=creature->rotation;
				int side=effect.parameter2;
				if(side==-1)
					side=creature->side;

				//find right place
				float bound_circle2=32;
				float creature_size=(bound_circle2)*(creature->size*mod.general_creatures[creature->type].size);
				float creature_size2=(bound_circle2)*(1*mod.general_creatures[effect.parameter1].size);
				float distance=creature_size*0.5f+creature_size2*0.5f;
				float place_x=sincos.table_cos(angle-pi/2)*distance;
				float place_y=sincos.table_sin(angle-pi/2)*distance;

				spawn_creature(side,effect.parameter3,effect.parameter4,x+place_x,y+place_y,angle,effect.parameter1,map_main);

				//initialize creature places
				//creatures_checked_on=0;
				map_main->check_creatures();

			}

			return_value=true;

			break;
		case 11:
			//11=change maximum bar parameter1 amount by parameter2
			if(!undo){
				creature->bars[(int)effect.parameter1].maximum+=effect.parameter2;
				//maximum_battery_energy+=effect.parameter1;
				//battery_energy+=effect.parameter1;
				return_value=true;
			}
			else{
				creature->bars[(int)effect.parameter1].maximum-=effect.parameter2;
				return_value=true;
			}

			break;
		case 12:
			//12=change armor level	to parameter1
			if(!undo){
				armor=effect.parameter1;
			}
			else{
				if(armor==effect.parameter1)
					armor=0;
			}
			return_value=true;

			break;
		case 13:
			//13=enable creature detector with distance parameter1
			if(!undo){
				if(kill_meter_active!=effect.parameter1)
					kill_meter_active=effect.parameter1;
				else kill_meter_active=0;
			}
			else{
				kill_meter_active=0;
			}
			return_value=true;

			break;
		case 14:
			//14=play sound parameter1 (from sounds.dat) with volume parameter2
			if(!undo){
				playsound(preloaded_sounds[(int)effect.parameter1],effect.parameter2,x,y,player_middle_x,player_middle_y);
			}
			return_value=true;

			break;
		case 15:
			//15=change creature into creature number parameter1 for time parameter2 (-1 for infinite) with creature visual effect from weapon parameter3, parameter4 (0=reset bars, 1=don't reset bars)
			if(!undo){
				//first find if we're already transformed into something
				int previous_type=creature->type;
				for(int a=0;a<creature->weapon_effects_amount;a++){
					//effects on creature
					if(creature->weapon_effects[a].effect_type==3){
						//time elapsed, return to normal side
						if(creature->weapon_effects[a].time>0){
							previous_type=creature->weapon_effects[a].parameter0;
						}
					}
				}
				creature_base::weapon_effect temp_effect;
				temp_effect.effect_type=3;
				temp_effect.time=effect.parameter2;
				temp_effect.parameter0=previous_type;
				creature->type=effect.parameter1;
				creature->tactic[0]=mod.general_creatures[creature->type].AI_tactics[0];
				creature->tactic[1]=mod.general_creatures[creature->type].AI_tactics[1];

				//creature specialties
				bool reset_bars=true;
				if(effect.parameter4==1)
					reset_bars=false;
				initialize_creature_specialties(creature,map_main,reset_bars);
				AI_initiate_behavior_parameters(creature);




				//record the effect only if the change is not permanent
				if(effect.parameter2>=0){
					temp_effect.weapon_type=effect.parameter3;
					creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
					creature->weapon_effects_amount++;
					if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
						creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;
				}


			}
			return_value=true;
			break;
		case 16:
			//16=give item parameter1 amount parameter2, parameter3=inventory number
			arrange_needed=true;
			if(!undo){
				{
					int previous_inventory=active_inventory;
					active_inventory=(int)effect.parameter3;
					return_value=give_item(effect.parameter1,effect.parameter2,time_from_beginning, true);
					active_inventory=previous_inventory;
				}
			}

			break;
		case 17:
			//17=set creature's bar parameter3 to parameter1
			if(!undo){
				return_value=false;
				if(creature->bars[(int)effect.parameter3].active){
					set_bar(creature,(int)effect.parameter3,effect.parameter1);
					return_value=true;

				}
			}
			break;
		case 18:
			//18=increase player's body temperature by parameter1
			if(!undo){
				body_temperature+=(float)effect.parameter1/100.0f;
				return_value=true;
			}
			break;
		case 19:
			//19=drop plot_object parameter1, object size parameter2, random area size parameter3 pixels, maximum amount of similar items in area parameter4 (0=infinite)
			if(!undo){

				//no area requirement
				if(effect.parameter3<=0){
					create_plot_object(map_main,effect.parameter1,x,y,angle,effect.parameter2);
				}
				//area requirement
				else{
					//maximum amount requirement
					bool can_drop=true;
					if(effect.parameter4>0){
						int alku_x=(int)((x-effect.parameter3)/grid_size)-3;
						int alku_y=(int)((y-effect.parameter3)/grid_size)-3;
						int loppu_x=(int)((x+effect.parameter3)/grid_size)+3;
						int loppu_y=(int)((y+effect.parameter3)/grid_size)+3;

						if(alku_x<0)alku_x=0;
						if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
						if(alku_y<0)alku_y=0;
						if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

						//count amount of dispensed objects nearby
						int objects_here=0;
						for(int a=alku_x;a<loppu_x;a++){
							for(int b=alku_y;b<loppu_y;b++){
								for(int c=0;c<map_main->grid[a].grid[b].items.size();c++){
									if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=0)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
									float size=map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f<x-effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f>x+effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f<y-effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f>y+effect.parameter3)continue;
									if(map_main->items[map_main->grid[a].grid[b].items[c]].item_type==effect.parameter1)
										objects_here++;
								}
							}
						}

						if(objects_here>=effect.parameter4)
							can_drop=false;
					}
					//now drop the item
					if(can_drop){
						create_plot_object(map_main,effect.parameter1,x+randDouble(-effect.parameter3,effect.parameter3),y+randDouble(-effect.parameter3,effect.parameter3),angle,effect.parameter2);
					}



				}

				return_value=true;
			}
			break;
		case 20:
			//20=enable large map
			if(!undo){
				map_active=true;
			}
			else{
				map_active=false;
			}
			return_value=true;

			break;


		case 21:
			//21=teleport to area parameter1 (if -1, game finds the right area) plot_object parameter2, parameter3: 0=don't transfer other creatures 1=transfer other creatures
			if(!undo){

				transfer_enemies_when_changing_map=true;
				if(effect.parameter3==0)
					transfer_enemies_when_changing_map=false;

				int map_change=effect.parameter1;
				return_value=false;
				float new_x;
				float new_y;

				//need to find the right map
				if(effect.parameter1==-1){
					for(int i=0;i<map_storage.size();i++){
						//find plot_object
						for(int a=0;a<map_storage[i]->items.size();a++){
							if(map_storage[i]->items[a].dead)continue;
							if((map_storage[i]->items[a].base_type==0)&&(map_storage[i]->items[a].item_type==(int)effect.parameter2)){
								new_x=map_storage[i]->items[a].x+general_object_size*map_storage[i]->items[a].size*0.5f;
								new_y=map_storage[i]->items[a].y+general_object_size*map_storage[i]->items[a].size*0.5f;
								map_change=i;
								break;
							}
						}
					}
				}
				//map specified
				else{
					new_x=randDouble(map_storage[effect.parameter1]->sizex*grid_size*0.2f,map_storage[effect.parameter1]->sizex*grid_size*0.8f);
					new_y=randDouble(map_storage[effect.parameter1]->sizey*grid_size*0.2f,map_storage[effect.parameter1]->sizey*grid_size*0.8f);
					//find plot_object
					for(int a=0;a<map_storage[effect.parameter1]->items.size();a++){
						if(map_storage[effect.parameter1]->items[a].dead)continue;
						if((map_storage[effect.parameter1]->items[a].base_type==0)&&(map_storage[effect.parameter1]->items[a].item_type==effect.parameter2)){
							new_x=map_storage[effect.parameter1]->items[a].x+general_object_size*map_storage[effect.parameter1]->items[a].size*0.5f;
							new_y=map_storage[effect.parameter1]->items[a].y+general_object_size*map_storage[effect.parameter1]->items[a].size*0.5f;
						}
					}
				}

				//request map change
				if(map_change>=0){
					change_map_to=map_change;
					change_map_player_x=new_x;
					change_map_player_y=new_y;

					return_value=true;
				}

			}

			break;

		case 22:
			//22=change side to parameter1 target for time parameter2 (-1=infinite) with creature visual effect from weapon parameter3
			if(!undo){
				creature_base::weapon_effect temp_effect;
				temp_effect.effect_type=1;
				temp_effect.time=effect.parameter2;
				temp_effect.parameter0=creature->side;
				creature->side=effect.parameter1;

				//record the effect only if the change is not permanent
				if(effect.parameter2>=0){
					temp_effect.weapon_type=effect.parameter3;
					creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
					creature->weapon_effects_amount++;
					if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
						creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;
				}

				return_value=true;
			}
			break;

		case 23:
			//23=continuously increase bar parameter3 by parameter1 for time parameter4 with creature visual effect from weapon parameter2
			if(!undo){
				creature_base::weapon_effect temp_effect;
				temp_effect.effect_type=2;
				temp_effect.time=effect.parameter4;
				temp_effect.parameter0=effect.parameter1;
				temp_effect.parameter1=effect.parameter3;

				temp_effect.weapon_type=effect.parameter2;
				creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
				creature->weapon_effects_amount++;
				if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
					creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;

				return_value=true;
			}
			break;

		case 24:
			//24=fire weapon parameter1, times parameter2, (parameter3, 0=normal direction, 1=towards nearest enemy, 2=random direction, 3=shoot in direction parameter4 from the creature, 4=shoot in absolute direction parameter4), parameter4=shoot direction (only applicable if parameter3=3 or 4)
			if(!undo){
				float fire_angle;
				//normal fire angle
				if(effect.parameter3==0)
					fire_angle=angle;
				//nearest enemy
				if(effect.parameter3==1){
					int nearest_enemy=-1;
					//the creature has touched someone, that is the nearest enemy
					if(creature->touched_enemy>=0)
						nearest_enemy=creature->touched_enemy;
					//we need to find the nearest enemy
					else{
						float nearest_distance=0;
						for(int a=0;a<map_main->creature.size();a++){
							if(a==creature_number)continue;//self
							if(map_main->creature[a].dead)continue;
							if(map_main->creature[a].killed)continue;
							if(mod.AI_sides[map_main->creature[creature_number].side].friend_with_side[map_main->creature[a].side])continue;
							float enemy_x=map_main->creature[a].x+mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size*0.5f;
							float enemy_y=map_main->creature[a].y+mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size*0.5f;
							float distance=sqr(x-enemy_x)+sqr(y-enemy_y);
							if((nearest_enemy==-1)||(distance<nearest_distance)){
								nearest_enemy=a;
								nearest_distance=distance;
							}

						}
					}
					float enemy_x=map_main->creature[nearest_enemy].x+mod.general_creatures[map_main->creature[nearest_enemy].type].size*map_main->creature[nearest_enemy].size*general_creature_size*0.5f;
					float enemy_y=map_main->creature[nearest_enemy].y+mod.general_creatures[map_main->creature[nearest_enemy].type].size*map_main->creature[nearest_enemy].size*general_creature_size*0.5f;
					fire_angle=-atan2(x-enemy_x,y-enemy_y);
				}
				//shoot in direction
				if(effect.parameter3==3){
					fire_angle=angle+effect.parameter4;
				}

				//shoot in absolute direction
				if(effect.parameter3==4){
					fire_angle=effect.parameter4;
				}


				for(a=0;a<effect.parameter2;a++){
					//random fire angle
					if(effect.parameter3==2)
						fire_angle=randDouble(0,2*pi);
					if(fire_angle<0)
						fire_angle+=2*pi;
					if(fire_angle>2*pi)
						fire_angle-=2*pi;
					shoot(creature_number,creature->side,effect.parameter1,x,y,fire_angle);
				}

				return_value=true;
			}
			break;
		case 25:
			//25=show animation parameter1, and if (parameter2, 0=continue game, 1=end game)
			if(!undo){
				if(effect.parameter2==1){
					game_running=false;
					play_animated_sequence(effect.parameter1,5);
				}
				else if(effect.parameter2==0){
					//game_running=false;
					play_animated_sequence(effect.parameter1,0);
				}

				return_value=true;
			}
			break;
		case 26:
			//26=fire particle parameter1 times parameter2 with parameters (spread, speed, time) taken from weapon number parameter3
			if(!undo){
				if((effect.parameter3<mod.general_weapons.size())&&(effect.parameter3>=0))
					for(a=0;a<effect.parameter2;a++){

						float rand_angle=angle+randDouble(-mod.general_weapons[effect.parameter3].spread,mod.general_weapons[effect.parameter3].spread);
						float vx=mod.general_weapons[effect.parameter3].bullet_speed*sincos.table_cos(rand_angle-pi*0.5f);
						float vy=mod.general_weapons[effect.parameter3].bullet_speed*sincos.table_sin(rand_angle-pi*0.5f);

						make_particle(effect.parameter1,1,mod.general_weapons[effect.parameter3].time+randDouble(-20,20),x,y,vx,vy);
					}

				return_value=true;
			}
			break;

		case 27:
			//27=make light number parameter1 for time parameter2 (-1=infinite) size min parameter3, max parameter4
			if(!undo){
				int light=map_main->create_light(x,y,effect.parameter1,randDouble(effect.parameter3,effect.parameter4),mod.general_lights[effect.parameter1].r,mod.general_lights[effect.parameter1].g,mod.general_lights[effect.parameter1].b,mod.general_lights[effect.parameter1].a,effect.parameter2);
				map_main->lights[light].rotation=angle;

				return_value=true;
			}
			break;
		case 28:
			//28=bar parameter1 increased by bar parameter2 * parameter3
			if(!undo){
				//set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter1].value+creature->bars[(int)effect.parameter2].value*effect.parameter3);
				set_bar(creature,(int)effect.parameter1,creature->bars[(int)effect.parameter1].value+creature->bars[(int)effect.parameter2].value*effect.parameter3);

				return_value=true;
			}
			break;
		case 29:
			//29=run script parameter1, parameter2=check conditions (0=no, 1=yes)
			if(!undo){
				bool check=true;
				if(effect.parameter2==0)check=false;
				run_script((int)effect.parameter1,check,false);
				return_value=true;
			}
			break;
		case 30:
			//30=set creature's eat item amount to parameter1
			if(!undo){
				creature->eat_counter=(int)effect.parameter1;
				return_value=true;
			}
			break;
		case 31:
			//31=kill creature (set all bars to minimum)
			if(!undo){
				for(a=0;a<maximum_bars;a++)
					creature->bars[a].value=creature->bars[a].minimum;
				return_value=true;
			}
			break;
		case 32:
			//32=change creature's anger level to parameter1 (between 0 and 1)
			if(!undo){
				creature->anger_level=effect.parameter1;
				return_value=true;
			}
			break;
		case 33:
			//33=stagger mouse by parameter1, speed parameter2 for time parameter3
			if(!undo){
				stagger_mouse_parameter1=effect.parameter1;
				stagger_mouse_parameter2=effect.parameter2;
				stagger_mouse_time=effect.parameter3;
				return_value=true;
			}
			break;
		case 34:
			//34=change player race into parameter1
			arrange_needed=true;
			if(!undo&&(effect.parameter1>=0)){
				old_player_race=player_race;
				player_race=effect.parameter1;

			}
			else{
				if(old_player_race>=0){
					player_race=old_player_race;
				}
			}

			//restore some stats
			maximum_carry_weight=mod.general_races[player_race].maximum_carry_weight;
			map_main->creature[0].type=mod.general_races[player_race].creature_number;
			player_normal_speed=mod.general_creatures[map_main->creature[0].type].movement_speed;

			//carried light
			{
				if(map_main->creature[0].carry_light>=0)delete_light(map_main,map_main->creature[0].carry_light);
				for(int specialty=0;specialty<mod.general_creatures[map_main->creature[0].type].specialties.size();specialty++){
					//add carried lights
					if(mod.general_creatures[map_main->creature[0].type].specialties[specialty].number==2){
						int type=mod.general_creatures[map_main->creature[0].type].specialties[specialty].parameter0;
						map_main->creature[0].carry_light=map_main->create_light(map_main->creature[0].x,map_main->creature[0].y,type,mod.general_creatures[map_main->creature[0].type].specialties[specialty].parameter1,mod.general_lights[type].r,mod.general_lights[type].g,mod.general_lights[type].b,mod.general_lights[type].a,-1);
						carry_light(map_main,&map_main->creature[0],map_main->creature[0].carry_light);
					}
				}
			}

			//unwield weapon
			{
				int temp_weapon=weapon_selected_from_item_list;
				if(temp_weapon>=0)
				if(inventory[active_inventory].player_items[temp_weapon].wielded){
					for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects.size();b++){
						use_item(inventory[active_inventory].player_items[temp_weapon].item, &temp_weapon, mod.general_items[inventory[active_inventory].player_items[temp_weapon].item].effects[b].effect, true, false, false,false);
					}
				}
				weapon_selected_from_item_list=-1;
				mod.general_creatures[map_main->creature[0].type].weapon=-1;
			}

			return_value=true;
			break;
		case 35:
			//35=activate/disable script parameter1, (parameter2: 0=disable, 1=activate)
			if(!undo){
				script_info[(int)effect.parameter1].dead=true;
				if(effect.parameter2==1){
					script_info[(int)effect.parameter1].dead=false;
					script_info[(int)effect.parameter1].timer=0;
					script_info[(int)effect.parameter1].script_calculated_on=time_from_beginning;
				}
				return_value=true;
			}
			break;
		case 36:
			//36=shake screen power parameter1 time parameter2 milliseconds
			if(!undo){
				screen_shake_time=effect.parameter2;
				screen_shake_power=effect.parameter1;
			}
			else{
				screen_shake_time=-1;
			}
			return_value=true;
			break;
		case 37:
			//37=start rain for time parameter1 (0=stop rain)
			if(!undo){
				map_main->rain_timer=effect.parameter1;
				return_value=true;
			}
			break;
		case 38:
			//38=change game speed to parameter1 (don't set it to negative)
			if(!undo){
				if(effect.parameter1>=0){
					game_speed=effect.parameter1;
					temp_speed=game_speed;
				}
			}
			else{
				game_speed=1;
				temp_speed=game_speed;
			}
			return_value=true;
			break;
		case 39:
			//39=show bar parameter1, (parameter2, 1=show, 0=hide)
			if(!undo){
				if(effect.parameter2==0)
					mod.general_bars[effect.parameter1].visible=false;
				if(effect.parameter2==1)
					mod.general_bars[effect.parameter1].visible=true;
				return_value=true;
			}
			else{
				mod.general_bars[effect.parameter1].visible=false;
				return_value=true;
			}
			break;
		case 40:
			//40=destroy plot_objects parameter1 from area size parameter2 pixels (not entirely accurate, use with care)
			if(!undo){

				int alku_x=(int)((x-effect.parameter2)/grid_size);
				int alku_y=(int)((y-effect.parameter2)/grid_size);
				int loppu_x=(int)((x+effect.parameter2)/grid_size)+1;
				int loppu_y=(int)((y+effect.parameter2)/grid_size)+1;

				if(alku_x<0)alku_x=0;
				if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
				if(alku_y<0)alku_y=0;
				if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

				//count amount of dispensed objects nearby
				for(int a=alku_x;a<loppu_x;a++){
					for(int b=alku_y;b<loppu_y;b++){
						for(int c=0;c<map_main->grid[a].grid[b].items.size();c++){
							if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=0)continue;
							if(map_main->items[map_main->grid[a].grid[b].items[c]].item_type==effect.parameter1){
								delete_plot_object(map_main,map_main->grid[a].grid[b].items[c], a, b, c);
								c--;
							}
						}
					}
				}
				return_value=true;
			}
			break;
		case 41:
			//41=destroy items parameter1 from area size parameter2 pixels (not entirely accurate, use with care), parameter3=maximum amount of items to delete (0=all)
			if(!undo){

				int alku_x=(int)((x-effect.parameter2)/grid_size);
				int alku_y=(int)((y-effect.parameter2)/grid_size);
				int loppu_x=(int)((x+effect.parameter2)/grid_size)+1;
				int loppu_y=(int)((y+effect.parameter2)/grid_size)+1;

				if(alku_x<0)alku_x=0;
				if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
				if(alku_y<0)alku_y=0;
				if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

				//count amount of dispensed objects nearby
				int deleted_amount=0;
				for(int a=alku_x;a<loppu_x;a++){
					for(int b=alku_y;b<loppu_y;b++){
						for(int c=0;c<map_main->grid[a].grid[b].items.size();c++){
							if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=1)continue;
							if(map_main->items[map_main->grid[a].grid[b].items[c]].item_type==effect.parameter1){

								int amount_was=map_main->items[map_main->grid[a].grid[b].items[c]].amount;
								for(int d=0;d<amount_was;d++){
									map_main->items[map_main->grid[a].grid[b].items[c]].amount--;
									deleted_amount++;

									if(effect.parameter3>0)if(deleted_amount>=effect.parameter3)break;
								}

								if(map_main->items[map_main->grid[a].grid[b].items[c]].amount<=0){
									delete_plot_object(map_main,map_main->grid[a].grid[b].items[c], a, b, c);
									c--;
								}
							}
							if(effect.parameter3>0)if(deleted_amount>=effect.parameter3)break;
						}
						if(effect.parameter3>0)if(deleted_amount>=effect.parameter3)break;
					}
					if(effect.parameter3>0)if(deleted_amount>=effect.parameter3)break;
				}
				return_value=true;
			}
			break;
		case 42:
			//42=increase maximum carry weight by parameter1
			arrange_needed=true;
			if(!undo){
				maximum_carry_weight+=effect.parameter1;
				return_value=true;
			}
			else{
				maximum_carry_weight-=effect.parameter1;
				return_value=true;
			}
			break;
		case 43:
			//43=return creature to previous frame position
			if(!undo){
				creature->x=creature->x2;
				creature->y=creature->y2;
				return_value=true;
			}
			break;
		case 44:
			//44=increase player's bar parameter3 with parameter1 (if parameter2=1, don't increase over maximum or decrease below minimum)
			if(!undo){

				return_value=false;

				if(map_main->creature[0].bars[(int)effect.parameter3].active){

					//don't top at maximum
					if(effect.parameter2!=1){
						set_bar(&map_main->creature[0],(int)effect.parameter3,map_main->creature[0].bars[(int)effect.parameter3].value+effect.parameter1);
						return_value=true;
					}
					else{
						//increasing
						if(effect.parameter1>0){
							if(map_main->creature[0].bars[(int)effect.parameter3].value<map_main->creature[0].bars[(int)effect.parameter3].maximum){
								if(map_main->creature[0].bars[(int)effect.parameter3].maximum-map_main->creature[0].bars[(int)effect.parameter3].value<effect.parameter1){
									effect.parameter1=map_main->creature[0].bars[(int)effect.parameter3].maximum-map_main->creature[0].bars[(int)effect.parameter3].value;
								}
								set_bar(&map_main->creature[0],(int)effect.parameter3,map_main->creature[0].bars[(int)effect.parameter3].value+effect.parameter1);
								return_value=true;
							}
						}
						//decreasing
						else if(effect.parameter1<0){
							if(map_main->creature[0].bars[(int)effect.parameter3].value>map_main->creature[0].bars[(int)effect.parameter3].minimum){
								if(map_main->creature[0].bars[(int)effect.parameter3].minimum-map_main->creature[0].bars[(int)effect.parameter3].value>effect.parameter1){
									effect.parameter1=map_main->creature[0].bars[(int)effect.parameter3].minimum-map_main->creature[0].bars[(int)effect.parameter3].value;
								}
								set_bar(&map_main->creature[0],(int)effect.parameter3,map_main->creature[0].bars[(int)effect.parameter3].value+effect.parameter1);
								return_value=true;
							}
						}
					}

					//make sure no bar is below minimum
					if(map_main->creature[0].bars[(int)effect.parameter3].value<map_main->creature[0].bars[(int)effect.parameter3].minimum)
						set_bar(&map_main->creature[0],(int)effect.parameter3,map_main->creature[0].bars[(int)effect.parameter3].minimum);

				}
			}
			break;
		case 45:
			//45=prevent creature from using weapon parameter1 (-1=all weapons) for time parameter2, with creature visual effect from weapon parameter3, parameter4: 0=individual weapons 1=weapon classes
			if(!undo){
				creature_base::weapon_effect temp_effect;
				temp_effect.effect_type=4;
				temp_effect.time=effect.parameter2;
				temp_effect.parameter0=effect.parameter1;
				temp_effect.parameter1=effect.parameter4;


				temp_effect.weapon_type=effect.parameter3;
				creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
				creature->weapon_effects_amount++;
				if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
					creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;

				return_value=true;
			}
			break;
		case 46:
			//46=change creature's AI tactic parameter1 (0=primary, 1=secondary) to parameter2 for time parameter3 with creature visual effect from weapon parameter4
			if(!undo){
				//time limited
				if(effect.parameter3>0){
					creature_base::weapon_effect temp_effect;
					temp_effect.effect_type=6;
					temp_effect.time=effect.parameter3;
					temp_effect.parameter0=(int)effect.parameter1;
					temp_effect.parameter1=creature->tactic[(int)effect.parameter1];

					creature->tactic[(int)effect.parameter1]=effect.parameter2;

					temp_effect.weapon_type=effect.parameter4;
					creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
					creature->weapon_effects_amount++;
					if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
						creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;
				}
				//infinite
				else{
					creature->tactic[(int)effect.parameter1]=effect.parameter2;
				}

				return_value=true;
			}
			/*else{
				creature->tactic[(int)effect.parameter1]=mod.general_creatures[creature->type].AI_tactics[(int)effect.parameter1];
				return_value=true;
			}*/
			AI_initiate_behavior_parameters(creature);
			break;
		case 47:
			//47=start dialog parameter1 (from dialogs.dat)
			if(!undo){
				creature->dialog=(int)effect.parameter1;
				creature->dialog_timer=time_from_beginning;
				return_value=true;
			}
			break;
		case 48:
			//48=pick up nearest creature of (parameter1, 0=type, 1=class) parameter2, maximum distance parameter3, side parameter4 (-1=same, -2=any)
			if(!undo&&(creature->carried_creature==-1)){
				int nearest=-1;
				float nearest_distance=10;
				//find nearest of type
				for(int a=0;a<map_main->creature.size();a++){
					if(a==creature_number)continue;
					if(map_main->creature[a].dead)continue;
					//if(map_main->creature[a].killed)continue;

					if((int)effect.parameter1==0)
						if(map_main->creature[a].type!=(int)effect.parameter2)continue;
					if((int)effect.parameter1==1)
						if(mod.general_creatures[map_main->creature[a].type].creature_class!=(int)effect.parameter2)continue;
					if((int)effect.parameter4==-1)
						if(map_main->creature[a].side!=creature->side)continue;
					if((int)effect.parameter4>=0)
						if(map_main->creature[a].side!=(int)effect.parameter4)continue;

					float size=mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size;
					float distance=sqr(x-(map_main->creature[a].x+size*0.5f))+sqr(y-(map_main->creature[a].y+size*0.5f));
					if(distance>sqr(effect.parameter3))continue;
					if((nearest==-1)||(distance<nearest_distance)){
						nearest=a;
						nearest_distance=distance;
					}
				}

				//make carried creature vanish
				if(nearest>=0){
					map_main->creature[nearest].dead=true;
					creature->carried_creature=nearest;
					if(map_main->creature[nearest].carry_light>=0)
						delete_light(map_main,map_main->creature[nearest].carry_light);

					return_value=true;
				}
			}
			else {
				map_main->creature[creature->carried_creature].dead=false;
				//float size=mod.general_creatures[map_main->creature[creature->carried_creature].type].size*map_main->creature[creature->carried_creature].size*general_creature_size;
				//find right place
				float bound_circle2=32;
				float creature_size=(bound_circle2)*(creature->size*mod.general_creatures[creature->type].size);
				float creature_size2=(bound_circle2)*(mod.general_creatures[map_main->creature[creature->carried_creature].type].size*map_main->creature[creature->carried_creature].size);
				float distance=creature_size*0.5f+creature_size2*0.5f;
				float size=mod.general_creatures[map_main->creature[creature->carried_creature].type].size*map_main->creature[creature->carried_creature].size*general_creature_size;
				float place_x=x+sincos.table_cos(angle-pi/2)*distance-size*0.5f;
				float place_y=y+sincos.table_sin(angle-pi/2)*distance-size*0.5f;

				//find if there's something blocking the drop
				/*vector <point2d> collisions=line_will_collide(x,y,place_x,place_y,true);
				if(collisions.size()==0){*/

					map_main->creature[creature->carried_creature].x=place_x;
					map_main->creature[creature->carried_creature].y=place_y;
					map_main->creature[creature->carried_creature].rotation=angle;
					map_main->creature[creature->carried_creature].rotation_head=angle;
					map_main->creature[creature->carried_creature].rotation_legs=angle;

					//return the carried light
					if(map_main->creature[creature->carried_creature].carry_light>=0){
						int type=map_main->creature[creature->carried_creature].carry_light_type;
						map_main->creature[creature->carried_creature].carry_light=map_main->create_light(map_main->creature[creature->carried_creature].x,map_main->creature[creature->carried_creature].y,type,map_main->creature[creature->carried_creature].carry_light_size,mod.general_lights[type].r,mod.general_lights[type].g,mod.general_lights[type].b,mod.general_lights[type].a,-1);
						carry_light(map_main,&map_main->creature[creature->carried_creature],map_main->creature[creature->carried_creature].carry_light);
					}

					creature->carried_creature=-1;
					return_value=true;

					map_main->check_creatures();
				//}

			}
			break;
		case 49:
			//49=attach camera to (parameter1, -1=player, 0=this creature [distance parameter2 from head], 1=this position), for time parameter3 (-1=infinite)
			if(!undo){
				attach_camera_time=effect.parameter3;
				if((int)effect.parameter1==-1){
					attach_camera_type=0;
					attach_camera_parameter1=0;
					attach_camera_parameter2=effect.parameter2;
				}
				if((int)effect.parameter1==0){
					attach_camera_type=0;
					attach_camera_parameter1=creature_number;
					attach_camera_parameter2=effect.parameter2;
				}
				if((int)effect.parameter1==1){
					attach_camera_type=1;
					attach_camera_parameter1=x;
					attach_camera_parameter2=y;
				}
				return_value=true;
			}
			else{
				attach_camera_type=0;
				attach_camera_time=-1;
				attach_camera_parameter1=0;
				attach_camera_parameter2=0;
				return_value=true;
			}
			break;
		case 50:
			//50=player controls nearest creature of (parameter1, -1=revert to player, 0=type, 1=class) parameter2, maximum distance parameter3, side parameter4 (-1=same, -2=any)
			if(!undo){
				if((int)effect.parameter1==-1){
					player_controlled_creature=0;
					return_value=true;
				}
				else{
					int nearest=-1;
					float nearest_distance=0;
					//find nearest of type
					for(int a=0;a<map_main->creature.size();a++){
						if(a==creature_number)continue;
						if(map_main->creature[a].dead)continue;
						//if(map_main->creature[a].killed)continue;

						if((int)effect.parameter1==0)
							if(map_main->creature[a].type!=(int)effect.parameter2)continue;
						if((int)effect.parameter1==1)
							if(mod.general_creatures[map_main->creature[a].type].creature_class!=(int)effect.parameter2)continue;
						if((int)effect.parameter4==-1)
							if(map_main->creature[a].side!=creature->side)continue;
						if((int)effect.parameter4>=0)
							if(map_main->creature[a].side!=(int)effect.parameter4)continue;

						float size=mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size;
						float distance=sqr(x-(map_main->creature[a].x+size*0.5f))+sqr(y-(map_main->creature[a].y+size*0.5f));
						if(distance>sqr(effect.parameter3))continue;
						if((nearest==-1)||(distance<nearest_distance)){
							nearest=a;
							nearest_distance=distance;
						}
					}
					if(nearest>=0){
						player_controlled_creature=nearest;
						return_value=true;
					}
				}

			}
			else{
				player_controlled_creature=0;
				return_value=true;
			}
			break;
		case 51:
			{
			//51=nearest creature of (parameter1, 0=type, 1=class) parameter2, maximum distance parameter3, side parameter4 (-1=same, -2=any) picks up this creature
			int nearest=-1;
			float nearest_distance=0;
			//find nearest of type
			for(int a=0;a<map_main->creature.size();a++){
				if(a==creature_number)continue;
				if(map_main->creature[a].dead)continue;
				//if(map_main->creature[a].killed)continue;

				if((int)effect.parameter1==0)
					if(map_main->creature[a].type!=(int)effect.parameter2)continue;
				if((int)effect.parameter1==1)
					if(mod.general_creatures[map_main->creature[a].type].creature_class!=(int)effect.parameter2)continue;
				if((int)effect.parameter4==-1)
					if(map_main->creature[a].side!=creature->side)continue;
				if((int)effect.parameter4>=0)
					if(map_main->creature[a].side!=(int)effect.parameter4)continue;

				float size=mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size;
				float distance=sqr(x-(map_main->creature[a].x+size*0.5f))+sqr(y-(map_main->creature[a].y+size*0.5f));
				if(distance>sqr(effect.parameter3))continue;
				if((nearest==-1)||(distance<nearest_distance)){
					nearest=a;
					nearest_distance=distance;
				}
			}
			if(!undo){
				//make carried creature vanish
				if((nearest>=0)&&(map_main->creature[nearest].carried_creature==-1)){
					creature->dead=true;
					map_main->creature[nearest].carried_creature=creature_number;
					if(creature->carry_light>=0)
						delete_light(map_main,creature->carry_light);
					return_value=true;
				}
			}
			else{
				if(nearest>=0)
				if(map_main->creature[nearest].carried_creature>=0){
					creature->dead=false;
					float bound_circle2=32;
					float creature_size=(bound_circle2)*(creature->size*mod.general_creatures[creature->type].size);
					float creature_size2=(bound_circle2)*(mod.general_creatures[map_main->creature[nearest].type].size*map_main->creature[nearest].size);
					float distance=creature_size*0.5f+creature_size2*0.5f;
					float size=mod.general_creatures[creature->type].size*creature->size*general_creature_size;
					float place_x=x+sincos.table_cos(map_main->creature[nearest].rotation-pi/2)*distance-size*0.5f;
					float place_y=y+sincos.table_sin(map_main->creature[nearest].rotation-pi/2)*distance-size*0.5f;

						creature->x=place_x;
						creature->y=place_y;
						creature->rotation=map_main->creature[nearest].rotation;
						creature->rotation_head=map_main->creature[nearest].rotation;
						creature->rotation_legs=map_main->creature[nearest].rotation;

						//return the carried light
						if(creature->carry_light>=0){
							int type=creature->carry_light_type;
							creature->carry_light=map_main->create_light(creature->x,creature->y,type,creature->carry_light_size,mod.general_lights[type].r,mod.general_lights[type].g,mod.general_lights[type].b,mod.general_lights[type].a,-1);
							carry_light(map_main,creature,creature->carry_light);
						}


						map_main->creature[nearest].carried_creature=-1;
						return_value=true;
				}

			}
			}
			break;




		case 52:
			//52=active inventory=parameter1 (0 to 10)
			if(!undo){
				int active_inventory_was=active_inventory;
				active_inventory=(int)effect.parameter1;
				if(active_inventory>10)active_inventory=10;
				if(active_inventory<0)active_inventory=0;

				if(active_inventory_was!=active_inventory){

					int temp_inventory=active_inventory;
					active_inventory=active_inventory_was;

					//unwielded wielded items
					for(a=0;a<inventory[active_inventory_was].player_items.size();a++){

						if(mod.general_items[inventory[active_inventory_was].player_items[a].item].wield_slots.size()>0)
						if(inventory[active_inventory_was].player_items[a].wielded){
							for(int b=0;b<mod.general_items[inventory[active_inventory_was].player_items[a].item].effects.size();b++){
								int item_number=a;
								use_item(inventory[active_inventory_was].player_items[a].item, &item_number, mod.general_items[inventory[active_inventory_was].player_items[a].item].effects[b].effect, true, false, false,false);
							}
						}
					}
					active_inventory=temp_inventory;
				}

				return_value=true;
			}
			else{
				active_inventory=0;
			}
			break;
		case 53:
			//53=continuously increase bar parameter3 by parameter1 for time parameter4 with creature visual effect from weapon parameter2, cap at maximum
			if(!undo){
				creature_base::weapon_effect temp_effect;
				temp_effect.effect_type=5;
				temp_effect.time=effect.parameter4;
				temp_effect.parameter0=effect.parameter1;
				temp_effect.parameter1=effect.parameter3;

				temp_effect.weapon_type=effect.parameter2;
				creature->weapon_effects[creature->weapon_effects_amount]=temp_effect;
				creature->weapon_effects_amount++;
				if(creature->weapon_effects_amount>=MAXIMUM_WEAPON_EFFECTS)
					creature->weapon_effects_amount=MAXIMUM_WEAPON_EFFECTS-1;

				return_value=true;
			}
			break;
		case 54:
			//54=make creature vanish for time parameter1 milliseconds
			if(!undo){
				creature->dead=true;
				creature->vanish_timer=effect.parameter1;

				return_value=true;
			}
			break;
		case 55:
			//55=make mouse (parameter1, 0=invisible, 1=visible)
			if(!undo){
				if(effect.parameter1==0)
					mouse_visible=false;
				if(effect.parameter1==1)
					mouse_visible=true;
				return_value=true;
			}
			else{
				mouse_visible=true;
			}
			break;
		case 56:
			//56=override control type (parameter1, 0=absolute, 1=relative, 2=vehicle)
			if(!undo){
				override_player_controls=effect.parameter1;
				return_value=true;
			}
			else{
				override_player_controls=-1;
			}
			break;
		case 57:
			//57=skip to day of journal (parameter1, 0=next day, 1 and above=skip to day number
			if(!undo){
				if(effect.parameter1>0){
					day_number=effect.parameter1-1;
					if(record_message(0,day_number)){//record the journal showing
						if(show_journals)
							show_text_window(journal_records.size()-1);
					}
				}
				if(effect.parameter1==0){
					day_number+=1;
					if(record_message(0,day_number)){//record the journal showing
						if(show_journals)
							show_text_window(journal_records.size()-1);
					}
				}
				return_value=true;
			}
			else{
			}
			break;
		case 58:
			//58=player (parameter1, 0=cannot, 1=can) view the inventory
			if(!undo){
				if(effect.parameter1==0)
					can_view_inventory=false;
				if(effect.parameter1==1)
					can_view_inventory=true;
				return_value=true;
			}
			else{
				can_view_inventory=true;
			}
			break;
		case 59:
			//59=use player's item parameter1 if there is one
			//find if we have such an item
			{
				int item_number=-1;
				for(a=0;a<inventory[active_inventory].player_items.size();a++){
					if(inventory[active_inventory].player_items[a].item==effect.parameter1){
						item_number=a;
						break;
					}
				}
				//no such item
				if(item_number==-1){
					return_value=false;
					break;
				}


				//slots already taken, unwield whatever is there
				if(!undo)
				for(int slot=0;slot<mod.general_items[inventory[active_inventory].player_items[item_number].item].wield_slots.size();slot++){
					int item_number_used=inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[item_number].item].wield_slots[slot]];

					//an item is already in the slot, unwield it
					if(item_number_used>=0){

						for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[item_number_used].item].effects.size();b++){
							use_item(inventory[active_inventory].player_items[item_number_used].item, &item_number_used, mod.general_items[inventory[active_inventory].player_items[item_number_used].item].effects[b].effect, true, false, false,false);
						}
					}
				}

				//use the item
				if(!undo){
					if(!inventory[active_inventory].player_items[item_number].wielded){

						for(int a=0;a<mod.general_items[inventory[active_inventory].player_items[item_number].item].effects.size();a++){
							use_item(inventory[active_inventory].player_items[item_number].item, &item_number, mod.general_items[inventory[active_inventory].player_items[item_number].item].effects[a].effect, false, false, false, true);
						}
					}
				}
				else{
					//if the item is already wielded, unwield it and continue
					if(inventory[active_inventory].player_items[item_number].wielded){
						for(int a=0;a<mod.general_items[inventory[active_inventory].player_items[item_number].item].effects.size();a++){
							use_item(inventory[active_inventory].player_items[item_number].item, &item_number, mod.general_items[inventory[active_inventory].player_items[item_number].item].effects[a].effect, true, false, false,false);
						}
					}
				}
				return_value=true;
				break;
			}
		case 60:
			//60=player (parameter1, 0=cannot, 1=can) drop items
			if(!undo){
				if(effect.parameter1==0)
					can_drop_items=false;
				if(effect.parameter1==1)
					can_drop_items=true;
				return_value=true;
			}
			else{
				can_drop_items=true;
			}
			break;
		case 61:
			//61=play song number parameter1 from music.dat
			if(!undo){
				play_music_file(effect.parameter1,&last_played_music);
				return_value=true;
			}
			else{
				return_value=true;
			}
			break;
		case 62:
			//62=(parameter1, 0=close, 1=open) inventory
			if(!undo){
				//we can go to item mode
				if((effect.parameter1==1)&&can_view_inventory){
					if((pop_up_mode!=3)){
						previous_pop_up_mode=pop_up_mode;
						pop_up_mode=3;
					}
					else{
						combine_item=-1;
						pop_up_mode=1;
					}
				}
				if((effect.parameter1==0)){
					combine_item=-1;
					pop_up_mode=1;
				}
				return_value=true;
			}
			else{
				return_value=true;
			}
			break;
		case 63:
			//63=set random value (between parameter1 and parameter2) to bar parameter3, (parameter4, 0=set value, 1=add to bar) (this effect caps at bar min & max)
			if(!undo){
				//find random value
				float random_value=randDouble(effect.parameter1,effect.parameter2);
				//adding to bar
				if(effect.parameter4==1){
					if(creature->bars[(int)effect.parameter3].active){
						set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].value+random_value);
						return_value=true;
					}
				}
				//setting to a bar
				if(effect.parameter4==0){
					if(creature->bars[(int)effect.parameter3].active){
						set_bar(creature,(int)effect.parameter3,random_value);
						return_value=true;
					}
				}

				//caps
				if(creature->bars[(int)effect.parameter3].value<creature->bars[(int)effect.parameter3].minimum)
					set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].minimum);
				if(creature->bars[(int)effect.parameter3].value>creature->bars[(int)effect.parameter3].maximum)
					set_bar(creature,(int)effect.parameter3,creature->bars[(int)effect.parameter3].maximum);

				return_value=true;
			}
			else{
				return_value=true;
			}
			break;
		case 64:
			//64=(parameter1, 0=hide, 1=show) radar
			if(!undo){
				if(effect.parameter1==0)
					show_radar=false;
				if(effect.parameter1==1)
					show_radar=true;
			}
			else{
				show_radar=false;
			}
			return_value=true;

			break;
		case 65:
			//65=fire weapon parameter1 (creature is affected by weapon kickback), times parameter2, (parameter3, 0=normal direction, 1=towards nearest enemy, 2=random direction, 3=shoot in direction parameter4 from the creature, 4=shoot in absolute direction parameter4), parameter4=shoot direction (only applicable if parameter3=3 or 4)
			if(!undo){
				float fire_angle;
				//normal fire angle
				if(effect.parameter3==0)
					fire_angle=angle;
				//nearest enemy
				if(effect.parameter3==1){
					int nearest_enemy=-1;
					//the creature has touched someone, that is the nearest enemy
					if(creature->touched_enemy>0)
						nearest_enemy=creature->touched_enemy;
					//we need to find the nearest enemy
					else{
						float nearest_distance=0;
						for(int a=0;a<map_main->creature.size();a++){
							if(a==creature_number)continue;//self
							if(map_main->creature[a].dead)continue;
							if(map_main->creature[a].killed)continue;
							if(mod.AI_sides[map_main->creature[creature_number].side].friend_with_side[map_main->creature[a].side])continue;
							float enemy_x=map_main->creature[a].x+mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size*0.5f;
							float enemy_y=map_main->creature[a].y+mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size*0.5f;
							float distance=sqr(x-enemy_x)+sqr(y-enemy_y);
							if((nearest_enemy==-1)||(distance<nearest_distance)){
								nearest_enemy=a;
								nearest_distance=distance;
							}

						}
					}
					float enemy_x=map_main->creature[nearest_enemy].x+mod.general_creatures[map_main->creature[nearest_enemy].type].size*map_main->creature[nearest_enemy].size*general_creature_size*0.5f;
					float enemy_y=map_main->creature[nearest_enemy].y+mod.general_creatures[map_main->creature[nearest_enemy].type].size*map_main->creature[nearest_enemy].size*general_creature_size*0.5f;
					fire_angle=-atan2(x-enemy_x,y-enemy_y);
				}
				//shoot in direction
				if(effect.parameter3==3){
					fire_angle=angle+effect.parameter4;
				}

				//shoot in absolute direction
				if(effect.parameter3==4){
					fire_angle=effect.parameter4;
				}


				for(a=0;a<effect.parameter2;a++){
					//random fire angle
					if(effect.parameter3==2)
						fire_angle=randDouble(0,2*pi);
					if(fire_angle<0)
						fire_angle+=2*pi;
					if(fire_angle>2*pi)
						fire_angle-=2*pi;
					bullet bullet_shot=shoot(creature_number,creature->side,effect.parameter1,x,y,fire_angle);


					//shooting was a success
					if(!bullet_shot.dead){
						//shooter falls back
						creature->vx-=bullet_shot.move_x*mod.general_weapons[bullet_shot.type].push_shooter;
						creature->vy-=bullet_shot.move_y*mod.general_weapons[bullet_shot.type].push_shooter;
					}
				}

				return_value=true;
			}
			break;
		case 66:
			//66=fire weapon parameter1 (creature is affected by weapon kickback), times parameter2, fire location x offset parameter3 (0=player race default), fire location y offset parameter4 (0=player race default)
			if(!undo){
				float fire_angle=angle;
				float shoot_x=0;
				float shoot_y=0;

				float size=mod.general_creatures[creature->type].size*creature->size*general_creature_size;

				if((creature_number==0)&&(mod.general_creatures[map_main->creature[0].type].weapon>0)){
					shoot_x=(mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].x+effect.parameter3)*(size/general_creature_size)*sincos.table_cos(creature->rotation)+(mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].y+effect.parameter4)*(size/general_creature_size)*sincos.table_cos(creature->rotation-pi/2);
					shoot_y=(mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].x+effect.parameter3)*(size/general_creature_size)*sincos.table_sin(creature->rotation)+(mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].weapon_class].y+effect.parameter4)*(size/general_creature_size)*sincos.table_sin(creature->rotation-pi/2);
				}

				else{
					shoot_x=(mod.general_creatures[creature->type].weapon_x+effect.parameter3)*(size/general_creature_size)*sincos.table_cos(creature->rotation)+(mod.general_creatures[creature->type].weapon_y+effect.parameter4)*(size/general_creature_size)*sincos.table_cos(creature->rotation-pi/2);
					shoot_y=(mod.general_creatures[creature->type].weapon_x+effect.parameter3)*(size/general_creature_size)*sincos.table_sin(creature->rotation)+(mod.general_creatures[creature->type].weapon_y+effect.parameter4)*(size/general_creature_size)*sincos.table_sin(creature->rotation-pi/2);
				}

				for(a=0;a<effect.parameter2;a++){

					bullet bullet_shot=shoot(creature_number,creature->side,effect.parameter1,x+shoot_x,y+shoot_y,fire_angle);

					//shooting was a success
					if(!bullet_shot.dead){
						//shooter falls back
						creature->vx-=bullet_shot.move_x*mod.general_weapons[bullet_shot.type].push_shooter;
						creature->vy-=bullet_shot.move_y*mod.general_weapons[bullet_shot.type].push_shooter;
					}
				}

				return_value=true;
			}
			break;
		case 67:
			//67=push creature, parameter1=force,(parameter3, 0=normal direction, 1=towards nearest enemy, 2=random direction, 3=direction parameter4 from the creature, 4=absolute direction parameter4), parameter4=direction (only applicable if parameter3=3 or 4)
			if(!undo){
				float fire_angle=angle;

				//normal fire angle
				if(effect.parameter3==0)
					fire_angle=angle;
				//nearest enemy
				if(effect.parameter3==1){
					int nearest_enemy=-1;
					//the creature has touched someone, that is the nearest enemy
					if(creature->touched_enemy>=0)
						nearest_enemy=creature->touched_enemy;
					//we need to find the nearest enemy
					else{
						float nearest_distance=0;
						for(int a=0;a<map_main->creature.size();a++){
							if(a==creature_number)continue;//self
							if(map_main->creature[a].dead)continue;
							if(map_main->creature[a].killed)continue;
							if(mod.AI_sides[map_main->creature[creature_number].side].friend_with_side[map_main->creature[a].side])continue;
							float enemy_x=map_main->creature[a].x+mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size*0.5f;
							float enemy_y=map_main->creature[a].y+mod.general_creatures[map_main->creature[a].type].size*map_main->creature[a].size*general_creature_size*0.5f;
							float distance=sqr(x-enemy_x)+sqr(y-enemy_y);
							if((nearest_enemy==-1)||(distance<nearest_distance)){
								nearest_enemy=a;
								nearest_distance=distance;
							}

						}
					}
					float enemy_x=map_main->creature[nearest_enemy].x+mod.general_creatures[map_main->creature[nearest_enemy].type].size*map_main->creature[nearest_enemy].size*general_creature_size*0.5f;
					float enemy_y=map_main->creature[nearest_enemy].y+mod.general_creatures[map_main->creature[nearest_enemy].type].size*map_main->creature[nearest_enemy].size*general_creature_size*0.5f;
					fire_angle=-atan2(x-enemy_x,y-enemy_y);
				}
				//shoot in direction
				if(effect.parameter3==3){
					fire_angle=angle+effect.parameter4;
				}

				//shoot in absolute direction
				if(effect.parameter3==4){
					fire_angle=effect.parameter4;
				}

				//random fire angle
				if(effect.parameter3==2)
					fire_angle=randDouble(0,2*pi);


				if(fire_angle<0)
					fire_angle+=2*pi;
				if(fire_angle>2*pi)
					fire_angle-=2*pi;


				//find angles
				float move_x=sincos.table_cos(fire_angle-pi/2);
				float move_y=sincos.table_sin(fire_angle-pi/2);


				//creature falls back
				creature->vx+=move_x*effect.parameter1;
				creature->vy+=move_y*effect.parameter1;


				return_value=true;
			}
			break;

		case 68:
			//68=drop plot_object parameter1, parameter2=x, parameter3=y, parameter4=rotation of dropped plot_object
			if(!undo){
				float drop_x=0;
				float drop_y=0;

				drop_x=x+(effect.parameter2)*sincos.table_cos(angle)+(effect.parameter3)*sincos.table_cos(angle-pi/2);
				drop_y=y+(effect.parameter2)*sincos.table_sin(angle)+(effect.parameter3)*sincos.table_sin(angle-pi/2);

				create_plot_object(map_main,effect.parameter1,drop_x,drop_y,angle+effect.parameter4,-1);

				return_value=true;
			}
			break;
		case 69:
			//69=add player's bar parameter1 to creature's bar parameter2
			if(!undo){
				return_value=false;
				if((creature->bars[(int)effect.parameter2].active)&&(map_main->creature[0].bars[(int)effect.parameter1].active)){
					set_bar(creature,(int)effect.parameter2,creature->bars[(int)effect.parameter2].value+map_main->creature[0].bars[(int)effect.parameter1].value);
					return_value=true;
				}
			}
			break;







	}

	//we've done something, best to rearrange the item list
	if(return_value&&arrange_needed)
		arrange_item_list(false);

	return return_value;

}


void game_engine::draw_item_view(void){

	calculate_quick_keys(true);

	int a,b;

	//computer texture
	grim->System_SetState_Blending(true);
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(item_view);
	grim->Quads_SetColor(1,1,1,1.0f);
	grim->Quads_Begin();
		//draw small computer
		grim->Quads_Draw(screen_width-640, screen_height-480, 640, 480);
	grim->Quads_End();

	//arrange item list
	if(!item_list_arranged){
		arrange_item_list(false);
		item_list_arranged=true;
	}

	float item_dialog_x=screen_width-640;
	float item_dialog_y=screen_height-480;

	int right_line=83;


	//return to game by mouse
	if(sqr(mousex-item_dialog_x-575)+sqr(mousey-item_dialog_y-427)<sqr(23)){
		text_manager.write_line(font,mousex-170, mousey-6,"Click to exit",1);
		if(!mouse_left&&mouse_left2){
			playsound(UI_game_click[0],1,0,0,0,0);
			combine_item=-1;
			pop_up_mode=1;
		}
	}
	//return to game by x or i or escape
	//if((key_x&&!key_x2)||(key_i&&!key_i2)||(!key_escape&&key_escape2&&(combine_item==-1))){
	if(!key_escape&&key_escape2&&(combine_item==-1)){
		playsound(UI_game_click[0],1,0,0,0,0);
		combine_item=-1;
		key_escape2=false;
		pop_up_mode=1;
	}
	//return to game by right click
	if(!mouse_right&&mouse_right2){
		playsound(UI_game_click[0],1,0,0,0,0);
		combine_item=-1;
		pop_up_mode=1;
	}


	//use item by mouse
	int can_use_item=-1;
	vector <int> use_effects;


	//dismantle selected item by b
	int dismantle_item=-1;
	if(combine_item!=-1)
	if(key_b&&!key_b2){
		dismantle_item=combine_item;
	}

	//drop selected item by d
	int drop_item=-1;
	/*if(combine_item!=-1)
	if(key_d&&!key_d2){
		drop_item=combine_item;
	}*/




	int mahtuu=13;//montako rivi� mahtuu listaan





	float item_list_x=50;
	float item_list_y=85;
	char temprivi2[100];

			const int rivi_korkeus=25;

			int rivi=0;
			int ohirivit=0;
			int total_items=0;//montako yhteens�

			grim->System_SetState_Blending(true);
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
			grim->Quads_SetRotation(0);

			bool rullaus=false;

			grim->Quads_SetColor(0.8f,1,0.8f,1);
			text_manager.write_line(font,item_dialog_x+55, item_dialog_y+55, "Select Item:",1.5f);
			//text_manager.write_line(font,item_dialog_x+250, item_dialog_y+55, "Item Description:",1.5f);
			//text_manager.write_line(font,item_dialog_x+250, item_dialog_y+250, "Combination:",1.5f);




			grim->Quads_SetColor(0.8f,1,0.8f,0.8f);
			//oikea viiva
			text_manager.draw_line(item_dialog_x+240+right_line,item_dialog_y+37,item_dialog_x+240+right_line,item_dialog_y+406,1.5f,0.5f,0.5f,1,1,1);
			//vasen viiva
			text_manager.draw_line(item_dialog_x+83,item_dialog_y+80,item_dialog_x+83,item_dialog_y+406,1.5f,0.5f,0.5f,1,1,1);



			string header="Item Description:";
			int item_description=-1;

			//list all items
			bool mouse_on_any_item=false;
			for(a=0;a<inventory[active_inventory].player_items.size();a++){

				if(inventory[active_inventory].player_items[a].item<=-1)continue;
				if(inventory[active_inventory].player_items[a].amount<=0)continue;
				if(mod.general_items[inventory[active_inventory].player_items[a].item].visible_in_inventory==0)continue;
				//if(inventory[active_inventory].player_items[a].wielded)continue;
				total_items++;
				if(total_items>=mahtuu){rullaus=true;}
				if(rivi>=mahtuu){continue;}
				if(item_list_place>ohirivit){ohirivit++;continue;}


				//item name
				tempstring=mod.general_items[inventory[active_inventory].player_items[a].item].name;
				int max_length=22;
				if(inventory[active_inventory].player_items[a].amount>1)max_length-=4;
				if(inventory[active_inventory].player_items[a].amount>10)max_length-=1;
				if(tempstring.length()>max_length){
					tempstring=tempstring.substr(0,max_length);
					tempstring+="..";
				}
				if(inventory[active_inventory].player_items[a].amount>1){
					//itoa(inventory[active_inventory].player_items[a].amount,temprivi2,10);
					sprintf(temprivi2,"%d",inventory[active_inventory].player_items[a].amount);
					tempstring+=" (";
					tempstring+=temprivi2;
					tempstring+=")";
				}
				grim->Quads_SetColor(1,1,1,1);
				int text_right_x=text_manager.write_line(font,item_dialog_x+item_list_x+34,item_dialog_y+item_list_y+rivi*rivi_korkeus, tempstring,1);

				//weight
				//itoa(mod.general_items[inventory[active_inventory].player_items[a].item].weight*inventory[active_inventory].player_items[a].amount,temprivi,10);
				sprintf(temprivi,"%d",(int)(mod.general_items[inventory[active_inventory].player_items[a].item].weight*inventory[active_inventory].player_items[a].amount));
				text_manager.write_line(font,item_dialog_x+item_list_x,item_dialog_y+item_list_y+rivi*rivi_korkeus,temprivi,1);


				//small picture
				if(text_right_x<item_dialog_x+240+right_line-rivi_korkeus-4){
					grim->Quads_SetColor(1,1,1,1);
					resources.Texture_Set(mod.general_items[inventory[active_inventory].player_items[a].item].texture);
					grim->Quads_SetSubset(0,0,1,1);
					grim->Quads_Begin();
						grim->Quads_Draw(item_dialog_x+240+right_line-rivi_korkeus-4,item_dialog_y+item_list_y+rivi*rivi_korkeus-3, rivi_korkeus, rivi_korkeus);
					grim->Quads_End();
				}



				//mouse is on this
				bool mouse_on_this=false;
				if((mousex>item_dialog_x+83)&&(mousex<item_dialog_x+240+right_line-10)
				&&(mousey>=item_dialog_y+item_list_y+rivi*rivi_korkeus)&&(mousey<item_dialog_y+item_list_y+(rivi+1)*rivi_korkeus))
				{

					mouse_on_any_item=true;
					mouse_on_this=true;
				}


				//use this item
				if((mouse_on_this&&combine_item==-1)||(combine_item==a)){

					for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
						if(key_clicked[translate_key_int(mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].use_key)]){
							//key_clicked[translate_key_int(mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].use_key)]=false;
							can_use_item=a;
							use_effects.push_back(b);
							combine_item=-1;
						}
					}
				}

				//drop or break this item
				if((mouse_on_this&&combine_item==-1)||(combine_item==a)){
					if(!key_d&&key_d2){
						if(can_drop_items){
							key_d2=false;
							drop_item=a;
							combine_item=-1;
						}
						else
							text_manager.message(3000,1000,"Cannot drop items.");
					}
					if(!key_b&&key_b2){
						key_b2=false;
						dismantle_item=a;
						combine_item=-1;
					}
				}


				//find item description
				if((mouse_on_this&&(combine_item==-1))){
					item_description=inventory[active_inventory].player_items[a].item;
				}

				//find the combine result if there is one
				if((mouse_on_this)&&(combine_item!=-1)){
					Mod::combines combination;
					bool combination_found=false;
					if(item_has_combination(inventory[active_inventory].player_items[combine_item].item,inventory[active_inventory].player_items[a].item,&combination))combination_found=true;
					if(item_has_combination(inventory[active_inventory].player_items[a].item,inventory[active_inventory].player_items[combine_item].item,&combination))combination_found=true;

					//combination found
					if(combination_found){
						header="Combination Result:";
						item_description=combination.combine_results[0].combines_to;
					}
					else{
						header="Doesn't Combine";
						item_description=-2;
					}
				}





				//select the combined items
				if(mouse_on_this)
					if(!mouse_left&&mouse_left2){
						mouse_left=false;
						mouse_left2=false;

						playsound(UI_game_click[1],1,0,0,0,0);
						if(combine_item==-1){
							combine_item=a;
						}
						//combine with this
						else if(combine_item>-1){
							//find out if these can be combined
							if(combine_item!=a){

								Mod::combines combination;
								bool combination_found=false;
								if(item_has_combination(inventory[active_inventory].player_items[combine_item].item,inventory[active_inventory].player_items[a].item,&combination))combination_found=true;
								if(item_has_combination(inventory[active_inventory].player_items[a].item,inventory[active_inventory].player_items[combine_item].item,&combination))combination_found=true;


								//combine
								if(combination_found){
									show_slider(2,a,combine_item,-1,-1,combination.combine_puzzle_difficulty);
									text_manager.message(combination.combine_puzzle_difficulty*1000,1000,"Combining...");
									combine_item=-1;
								}
								//doesn't combine, inform player of failure
								else{
									text_manager.message(3000,1000,"Unable to combine these items");
								}
							}
						}
					}
				rivi++;
			}

			//help text
			bool mouse_on_slot=false;
			if((combine_item==-1)&&(!mouse_on_any_item)&&((mousex<item_dialog_x+10)||(mousey<item_dialog_y+10)||(mousex>item_dialog_x+640-10)||(mousey>item_dialog_y+480-10))){
				/*grim->Quads_SetColor(0.8f,1,0.8f,1);
				text_manager.write_line(font,item_dialog_x+250, item_dialog_y+55, "Inventory:",1.5f);*/
				grim->Quads_SetColor(1,1,1,1);
				text_manager.write(font,"To use an item, select it, then press the U-key. \\   To combine an item with another item, click the item from the list, then click on the item you want to combine it with. \\   To break down an item, select it from the list and press B-key. \\   To drop an item, select it from the list, then press D-key.",1,item_dialog_x+250+right_line,item_dialog_y+55,screen_width-130,screen_height,false,1,1,1,0.8f);
			}


			//ragdoll
			else if(((item_description==-1)||(item_description==-2))&&(mod.general_races[player_race].rag_doll>=0)){
				float rag_doll_x=329;
				float rag_doll_y=107;
				grim->Quads_SetColor(1,1,1,1);
				resources.Texture_Set(mod.general_races[player_race].rag_doll);
				grim->Quads_SetSubset(0,0,1,1);
				grim->Quads_Begin();
					grim->Quads_Draw(item_dialog_x+rag_doll_x,item_dialog_y+rag_doll_y, 256, 256);
				grim->Quads_End();


				//slots
				for(a=0;a<mod.general_races[player_race].slots.size();a++){
					if(mod.general_races[player_race].slots[a].active){

						//light the slots the item goes to
						bool can_place_item_here=false;
						if(combine_item>-1){
							for(b=0;b<mod.general_items[inventory[active_inventory].player_items[combine_item].item].wield_slots.size();b++){
								if(mod.general_items[inventory[active_inventory].player_items[combine_item].item].wield_slots[b]==a)
									can_place_item_here=true;
							}
						}

						//mouse on it
						bool mouse_on=false;
						if((mousex>item_dialog_x+rag_doll_x+mod.general_races[player_race].slots[a].x)
						&&(mousey>item_dialog_y+rag_doll_y+mod.general_races[player_race].slots[a].y)
						&&(mousex<item_dialog_x+rag_doll_x+mod.general_races[player_race].slots[a].x+64)
						&&(mousey<item_dialog_y+rag_doll_y+mod.general_races[player_race].slots[a].y+64)){
							mouse_on=true;
							mouse_on_slot=true;
							//place item here
							if(combine_item>-1){
								if(!mouse_left&&mouse_left2){
									mouse_left=false;
									mouse_left2=false;

									//check if correct slot and not the same item
									if((can_place_item_here)&&(inventory[active_inventory].slot_used_by[a]!=combine_item)){

										//check if new item can be wielded
										bool OK=true;
										for(b=0;b<mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects.size();b++){
											if(!use_item(inventory[active_inventory].player_items[combine_item].item, &combine_item, mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects[b].effect, false, true, true,false))
												OK=false;
										}
										if(OK){
											int item_number=inventory[active_inventory].slot_used_by[a];

											//an item is already in the slot, unwield it
											if(item_number>=0){
												for(b=0;b<mod.general_items[inventory[active_inventory].player_items[item_number].item].effects.size();b++){
													use_item(inventory[active_inventory].player_items[item_number].item, &item_number, mod.general_items[inventory[active_inventory].player_items[item_number].item].effects[b].effect, true, false, false,false);
												}
											}

											//now wield the new item
											for(b=0;b<mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects.size();b++){
												use_item(inventory[active_inventory].player_items[combine_item].item, &combine_item, mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects[b].effect, false, true, false,false);
											}

											combine_item=-1;
										}
									}
								}
							}
						}

						if(can_place_item_here){
							//something already in slot
							if((inventory[active_inventory].slot_used_by[a]>=0)&&(inventory[active_inventory].slot_used_by[a]!=combine_item)){
								if(mouse_on)
									grim->Quads_SetColor(1.0f,0.0f,0.0f,1.0f);
								else
									grim->Quads_SetColor(1.0f,0.0f,0.0f,0.8f);
							}
							else{
								if(mouse_on)
									grim->Quads_SetColor(0.5f,1.0f,0.5f,1.0f);
								else
									grim->Quads_SetColor(0.5f,1.0f,0.5f,0.8f);
							}
						}
						else if(mouse_on)
							grim->Quads_SetColor(1,1,1,0.75f);
						else
							grim->Quads_SetColor(1,1,1,0.7f);


						resources.Texture_Set(mod.general_races[player_race].slots[a].texture);
						grim->Quads_SetSubset(0,0,1,1);
						grim->Quads_Begin();
							grim->Quads_Draw(item_dialog_x+rag_doll_x+mod.general_races[player_race].slots[a].x,item_dialog_y+rag_doll_y+mod.general_races[player_race].slots[a].y, 64, 64);
						grim->Quads_End();
					}
				}

				//items in use
				for(a=0;a<inventory[active_inventory].player_items.size();a++){
					if(inventory[active_inventory].player_items[a].item<=-1)continue;
					if(inventory[active_inventory].player_items[a].amount<=0)continue;
					if(!inventory[active_inventory].player_items[a].wielded)continue;

					//all slots
					for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots.size();b++){

						//draw icon
						grim->Quads_SetColor(1,1,1,1);
						resources.Texture_Set(mod.general_items[inventory[active_inventory].player_items[a].item].texture);
						grim->Quads_SetSubset(0,0,1,1);
						grim->Quads_Begin();
							grim->Quads_Draw(item_dialog_x+rag_doll_x+mod.general_races[player_race].slots[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]].x+2,item_dialog_y+rag_doll_y+mod.general_races[player_race].slots[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]].y+2, 60, 60);
						grim->Quads_End();

						//mouse on it
						if((mousex>item_dialog_x+rag_doll_x+mod.general_races[player_race].slots[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]].x)
						&&(mousey>item_dialog_y+rag_doll_y+mod.general_races[player_race].slots[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]].y)
						&&(mousex<item_dialog_x+rag_doll_x+mod.general_races[player_race].slots[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]].x+64)
						&&(mousey<item_dialog_y+rag_doll_y+mod.general_races[player_race].slots[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]].y+64)){
							if(!mouse_left&&mouse_left2){
								mouse_left=false;
								mouse_left2=false;
								playsound(UI_game_click[1],1,0,0,0,0);
								combine_item=a;
								combine_item_from_wield=true;
							}
						}

					}
				}


			}


			//item description
			if(item_description!=-1){
				grim->Quads_SetColor(0.8f,1,0.8f,1);
				text_manager.write_line(font,item_dialog_x+250+right_line, item_dialog_y+55, header,1.5f);


				//item symbol
				if(item_description>=0){
					grim->Quads_SetColor(1,1,1,1);
					text_manager.write_line(font,item_dialog_x+250+right_line, item_dialog_y+55+30, mod.general_items[item_description].name,1.1f);

					grim->Quads_SetColor(1,1,1,1);
					resources.Texture_Set(mod.general_items[item_description].texture);
					grim->Quads_SetSubset(0,0,1,1);
					grim->Quads_Begin();
						grim->Quads_Draw(item_dialog_x+410, item_dialog_y+300, 100, 100);
					grim->Quads_End();

					text_manager.write(font,mod.general_items[item_description].description,1,item_dialog_x+250+right_line,item_dialog_y+85+30,screen_width-130,screen_height,false,1,1,1,0.8f);
				}
			}




			//jos tarvitaan sivuille rullia
			if(rullaus){
				grim->System_SetState_Blending(true);
				resources.Texture_Set(mod.general_races[player_race].interface_texture);
				//ylempi nuoli
				if(item_list_place>0){
					if((mousex>item_dialog_x+232+right_line)&&(mousex<item_dialog_x+232+16+right_line)&&(mousey>item_dialog_y+31)&&(mousey<item_dialog_y+31+16)){
						grim->Quads_SetColor(1,1,1,1);
						if(mouse_left&&!mouse_left2){
							playsound(UI_game_click[0],1,0,0,0,0);
							mouse_on_any_item=true;
							item_list_place-=1;
							if(item_list_place<0)item_list_place=0;
						}
					}
				}
				grim->Quads_SetColor(1,1,1,1);
				grim->Quads_SetSubset(0,0,(16/256.0f),(16/256.0f));
				grim->Quads_Begin();
				grim->Quads_Draw(item_dialog_x+232+right_line, item_dialog_y+31, 16, 16);
				grim->Quads_End();

				//alempi nuoli
				if(item_list_place<total_items-mahtuu){
					if((mousex>item_dialog_x+232+right_line)&&(mousex<item_dialog_x+232+16+right_line)&&(mousey>item_dialog_y+437-42)&&(mousey<item_dialog_y+437+16-42)){
						grim->Quads_SetColor(1,1,1,1);
						if(mouse_left&&!mouse_left2){
							playsound(UI_game_click[0],1,0,0,0,0);
							mouse_on_any_item=true;
							item_list_place+=1;
							if(item_list_place>total_items-mahtuu)item_list_place=total_items-mahtuu;
						}
					}
				}
				grim->Quads_SetColor(1,1,1,1);
				grim->Quads_SetSubset(0,(32/256.0f),(16/256.0f),(48/256.0f));
				grim->Quads_Begin();
				grim->Quads_Draw(item_dialog_x+232+right_line, item_dialog_y+437-42, 16, 16);
				grim->Quads_End();

				//viiva
				float pituus=((float)mahtuu/total_items)*(437-47-42);
				float alku=((float)item_list_place/total_items)*(437-47-42);
				text_manager.draw_line(item_dialog_x+240+right_line,item_dialog_y+47+alku,item_dialog_x+240+right_line,item_dialog_y+47+alku+pituus,8,1,1,1,1,1);
				if((mousex>item_dialog_x+232+right_line)&&(mousex<item_dialog_x+232+16+right_line)&&(mousey>item_dialog_y+31+16)&&(mousey<item_dialog_y+437-42)){
					if(mouse_left){
						mouse_on_any_item=true;
						item_list_place=(mousey-item_dialog_y-47)/(437-47-42)*total_items-mahtuu/2;


					}
				}


				//rulla
				int rulla=mouse_wheel;
				int rullanopeus=total_items/50.0f;
				if(rullanopeus<1)rullanopeus=1;
				if(rulla<0)item_list_place+=rullanopeus;
				if(rulla>0)item_list_place-=rullanopeus;


				if(item_list_place<0)
					item_list_place=0;
				if(item_list_place>total_items-mahtuu)
					item_list_place=total_items-mahtuu;
			}


			//combine items
			if(combine_item>-1){


				//item name
				grim->Quads_SetColor(0.8f,1,0.8f,1);
				text_manager.write_line(font,mousex+28-16, mousey+10-16, mod.general_items[inventory[active_inventory].player_items[combine_item].item].name,1);

				grim->Quads_SetColor(1,1,1,1);

				/*//item symbol
				resources.Texture_Set(items_texture);
				find_texture_coordinates(mod.general_items[inventory[active_inventory].player_items[combine_item]].symbol_number,&x0,&y0,&x1,&y1,4);
				grim->Quads_SetSubset(x0,y0,x1,y1);
				grim->Quads_Begin();
				grim->Quads_Draw(mousex-16, mousey-16, 32, 32);
				grim->Quads_End();*/


				if (!key_escape&&key_escape2){
					key_escape2=false;
					combine_item=-1;
				}
			}

	//player clicked with combine_item, but not on any item
	if(mouse_left&&!mouse_left2){
		if(!mouse_on_slot&&!mouse_on_any_item&&(combine_item!=-1)){

			//see if the item is wielded
			if(inventory[active_inventory].player_items[combine_item].wielded&&combine_item_from_wield){
				//mouse on the left side of the inventory
				if(mousex<item_dialog_x+240+right_line){
					//unwield the item
					for(b=0;b<mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects.size();b++){
						use_item(inventory[active_inventory].player_items[combine_item].item, &combine_item, mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects[b].effect, true,false, false,false);
					}
					combine_item_from_wield=false;
				}
			}

			playsound(UI_game_click[1],1,0,0,0,0);
			combine_item=-1;
		}
	}

	//use item
	if(can_use_item>=0){
		bool remove_item=false;
		int item_type=inventory[active_inventory].player_items[can_use_item].item;

		for(int b=0;b<use_effects.size();b++){
			//not currently wielded
			if(!inventory[active_inventory].player_items[can_use_item].wielded){
				if(use_item(inventory[active_inventory].player_items[can_use_item].item, &can_use_item, mod.general_items[inventory[active_inventory].player_items[can_use_item].item].effects[use_effects[b]].effect, false,true, false,false)){
					remove_item=true;
				}
			}
			//wielded, unwield it
			else{
				use_item(inventory[active_inventory].player_items[can_use_item].item, &can_use_item, mod.general_items[inventory[active_inventory].player_items[can_use_item].item].effects[use_effects[b]].effect, true,true, false,false);
				playsound(UI_game_click[0],1,0,0,0,0);
			}
		}
		//now find the item slot again as it may have been deleted
		int item_found=-1;
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			if(inventory[active_inventory].player_items[a].item==item_type){
				item_found=a;
				break;
			}
		}
		if(item_found>=0){
			if(remove_item)
				inventory[active_inventory].player_items[item_found].amount--;

			if(inventory[active_inventory].player_items[item_found].amount<=0)
				delete_item(item_found);
		}
		combine_item=-1;
		arrange_item_list(false);
	}

	//weight
	tempstring="Carry Weight:";
	tempstring+=FloatToText(carry_weight,1);
	tempstring+="  Maximum:";
	tempstring+=FloatToText(maximum_carry_weight,1);
	text_manager.write_line(font,item_dialog_x+138,item_dialog_y+420,tempstring,1);



	//break up item
	if(dismantle_item>=0){
		//see if there's a combination that produces this item
		bool items_found=false;
		for(a=0;a<mod.general_items.size();a++){
			if(mod.general_items[a].dead)continue;
			for(int b=0;b<mod.general_items[a].combinations.size();b++){
				if(mod.general_items[a].combinations[b].combine_results.size()>1)continue;
				if(mod.general_items[a].combinations[b].combine_results[0].combines_amount>1)continue;//cannot uncombine if more than one was result
				if(mod.general_items[a].combinations[b].can_be_broken_up==0)continue;
				if(mod.general_items[a].combinations[b].combine_results[0].combines_to==inventory[active_inventory].player_items[dismantle_item].item){

					combine_item=-1;

					playsound(pick_up,1,0,0,0,0);

					items_found=true;


					give_item(mod.general_items[a].combinations[b].combines_with,1,time_from_beginning+randInt(1,10),true);
					give_item(a,1,time_from_beginning+randInt(1,10),true);


					//this must be last
					inventory[active_inventory].player_items[dismantle_item].amount--;
					//unuse the item
					if(inventory[active_inventory].player_items[dismantle_item].amount<=0){
						for(int c=0;c<mod.general_items[inventory[active_inventory].player_items[dismantle_item].item].effects.size();c++){
							//if(inventory[active_inventory].player_items[dismantle_item].used){
								use_item(inventory[active_inventory].player_items[dismantle_item].item,&dismantle_item,mod.general_items[inventory[active_inventory].player_items[dismantle_item].item].effects[c].effect,true,false, false,false);
							//}
						}

						delete_item(dismantle_item);
					}

					arrange_item_list(false);

					text_manager.message(3000,2000,"Item broken up");
				}
				if(items_found)break;
			}
			if(items_found)break;
		}

		//no combination found
		if(!items_found){
			text_manager.message(3000,2000,"Cannot break up");
		}
	}

	//drop item
	if(drop_item>=0){

		//find if the player race has some specialty preventing drop
		bool OK=true;
		for(a=0;a<mod.general_races[player_race].specialties.size();a++){
			if(!mod.general_races[player_race].specialties[a].difficulty[game_difficulty_level])continue;
			//specialty found
			if(mod.general_races[player_race].specialties[a].number==4){

				if(mod.general_races[player_race].specialties[a].parameter0==inventory[active_inventory].player_items[drop_item].item){
					OK=false;
					text_manager.message(2000,2000,"Cannot drop item");
				}
			}
			//specialty found
			if(mod.general_races[player_race].specialties[a].number==0){

				if(mod.general_races[player_race].specialties[a].parameter0==mod.general_items[inventory[active_inventory].player_items[drop_item].item].item_class){
					OK=false;
					text_manager.message(2000,2000,"Cannot drop item");
				}
			}
		}

		if(OK){
			int amount;
			//only one item, drop it
			if(inventory[active_inventory].player_items[drop_item].amount<=1){
				amount=1;

				inventory[active_inventory].player_items[drop_item].amount-=amount;
				create_item(map_main,inventory[active_inventory].player_items[drop_item].item,amount,player_middle_x,player_middle_y,-1);
				for(int c=0;c<mod.general_items[inventory[active_inventory].player_items[drop_item].item].effects.size();c++){
					use_item(inventory[active_inventory].player_items[drop_item].item,&drop_item, mod.general_items[inventory[active_inventory].player_items[drop_item].item].effects[c].effect,true,false, false, false);//unuse
				}
				if(inventory[active_inventory].player_items[drop_item].amount<=0)
					delete_item(drop_item);

				arrange_item_list(false);

			}

			//more, show a bar
			else if(inventory[active_inventory].player_items[drop_item].amount>1){
				//amount=(int)((inventory[active_inventory].player_items[drop_item].amount/3.0f)*2);
				show_slider(1,drop_item,0,mousex,mousey,1);
			}

		}


	}


}


void game_engine::draw_text_view(void){
	//computer texture
	grim->System_SetState_Blending(true);
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(item_view);
	grim->Quads_SetColor(1,1,1,1.0f);
	grim->Quads_Begin();
		//draw small computer
		grim->Quads_Draw(screen_width-640, screen_height-480, 640, 480);
	grim->Quads_End();


	float item_dialog_x=screen_width-640;
	float item_dialog_y=screen_height-480;


	//return to game by mouse
	if(sqr(mousex-item_dialog_x-575)+sqr(mousey-item_dialog_y-427)<sqr(23)){
		text_manager.write_line(font,mousex-170, mousey-6,"Click to continue",1);
		if((mouse_right&&!mouse_right2)||(!mouse_left&&mouse_left2)){
			playsound(UI_game_click[0],1,0,0,0,0);
			pop_up_mode=previous_pop_up_mode;
		}
	}
	//return to game by x or i or escape or enter or j
	if((key_enter&&!key_enter2)||(key_j&&!key_j2)||(key_x&&!key_x2)||(key_i&&!key_i2)||(!key_escape&&key_escape2&&(combine_item==-1))){
		playsound(UI_game_click[0],1,0,0,0,0);
		key_escape2=false;
		pop_up_mode=previous_pop_up_mode;
	}
	//return to game by right click
	if(!mouse_right&&mouse_right2){
		playsound(UI_game_click[0],1,0,0,0,0);
		pop_up_mode=previous_pop_up_mode;
	}

	float vertical_place=75;
	if(log_text.length()>800)
		vertical_place=50;

	//draw the text
	text_manager.write(font,log_text,1,item_dialog_x+80, item_dialog_y+vertical_place,screen_width-120,screen_height,false,1,1,1,1);


	//log entry number
	text_manager.write_line(font,item_dialog_x+129, item_dialog_y+17,"Log Entry ",1);
	//itoa(current_showing_entry+1,temprivi,10);
	sprintf(temprivi,"%d",current_showing_entry+1);
	text_manager.write_line(font,item_dialog_x+218, item_dialog_y+17,temprivi,1);



	//back
	if(current_showing_entry>0)
	{

		if((mousex>item_dialog_x+270)&&(mousey>item_dialog_y+17)&&(mousex<item_dialog_x+270+45)&&(mousey<item_dialog_y+17+15)){
			text_manager.write_line(font,item_dialog_x+270-2, item_dialog_y+17-1,"Back",1.1f);
			if(!mouse_left&&mouse_left2){
				playsound(UI_game_click[0],1,0,0,0,0);
				show_text_window(current_showing_entry-1);
			}
		}
		else
			text_manager.write_line(font,item_dialog_x+270, item_dialog_y+17,"Back",1);
	}

	//forward
	if(current_showing_entry<journal_records.size()-1)
	{

		if((mousex>item_dialog_x+335)&&(mousey>item_dialog_y+17)&&(mousex<item_dialog_x+335+73)&&(mousey<item_dialog_y+17+15)){
			text_manager.write_line(font,item_dialog_x+335-3, item_dialog_y+17-1,"Forward",1.1f);
			if(!mouse_left&&mouse_left2){
				playsound(UI_game_click[0],1,0,0,0,0);
				show_text_window(current_showing_entry+1);
			}
		}
		else
			text_manager.write_line(font,item_dialog_x+335, item_dialog_y+17,"Forward",1);
	}

	//end
	if(current_showing_entry<journal_records.size()-1)
	{

		if((mousex>item_dialog_x+435)&&(mousey>item_dialog_y+17)&&(mousex<item_dialog_x+435+35)&&(mousey<item_dialog_y+17+15)){
			text_manager.write_line(font,item_dialog_x+435-2, item_dialog_y+17-1,"End",1.1f);
			if(!mouse_left&&mouse_left2){
				playsound(UI_game_click[0],1,0,0,0,0);
				show_text_window(journal_records.size()-1);
			}
		}
		else
			text_manager.write_line(font,item_dialog_x+435, item_dialog_y+17,"End",1);
	}



}

bool game_engine::show_journal(int day, int race){

	disable_input_override();

	FILE *fil;
	char rivi[4000];

	//int creature_infos=0;
	bool day_found=false;

	string filename="data/"+mod.mod_name+"/"+mod.general_races[race].journal_name;
	fil = fopen(filename.c_str(),"rt");
	int lines=0;
	if(fil){

		//header text
		//log_text=mod.general_creatures[map_main->creature[0].type].name;
		log_text=stripped_fgets(rivi,sizeof(rivi),fil);
		log_text+=" ";
		int start_date=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
		int date_type=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
		//stardate date
		if(date_type==0){
			//itoa(day+start_date,temprivi,10);
			sprintf(temprivi,"%d",day+start_date);
			log_text+=temprivi;
		}
		//calendar date
		else if(date_type==1){
			int date=start_date+day;

			int l = date + 68569 + 2415019;
			int n = int(( 4 * l ) / 146097);
					l = l - int(( 146097 * n + 3 ) / 4);
			int i = int(( 4000 * ( l + 1 ) ) / 1461001);
				l = l - int(( 1461 * i ) / 4) + 31;
			int j = int(( 80 * l ) / 2447);
			int Day = l - int(( 2447 * j ) / 80);
				l = int(j / 11);
			int Month = j + 2 - ( 12 * l );
			int Year = 100 * ( n - 49 ) + i + l;

			//itoa(Day,temprivi,10);
			sprintf(temprivi,"%d",Day);
			log_text+=temprivi;
			log_text+="/";
			//itoa(Month,temprivi,10);
			sprintf(temprivi,"%d",Month);
			log_text+=temprivi;
			log_text+="/";
			//itoa(Year,temprivi,10);
			sprintf(temprivi,"%d",Year);
			log_text+=temprivi;
		}
		log_text+=" (Day ";
		//itoa(day+1,temprivi,10);
		sprintf(temprivi,"%d",day+1);
		log_text+=temprivi;
		log_text+=") \\ ";//linefeed


		//find day entry
		while(strcmp(stripped_fgets(rivi,sizeof(rivi),fil),"end_of_file")!=0){
			//right day found
			if(lines==day){
				log_text+=rivi;
				tempstring=rivi;
				if((tempstring=="")||(tempstring=="none"))
					day_found=false;
				else
					day_found=true;
				break;
			}

			lines++;
		}

		fclose(fil);
	}


	return day_found;
	//pop_up_mode=0;

}
bool game_engine::record_message(int type, int parameter0){

	//if it's a journal note, find out if it exists, and don't record if it doesn't
	if(type==0){
		if(!show_journal(parameter0, player_race))return false;
	}
	//it's any other type, find out if it's actually none

	//if(records>=500)records=0;
	journal_record temp_record;
	temp_record.record_type=type;
	temp_record.record_parameter0=parameter0;
	temp_record.record_parameter1=player_race;
	journal_records.push_back(temp_record);

	return true;
}

void game_engine::show_text_window(int entry_number){

	disable_input_override();

	bool OK=false;

	if(entry_number<0)return;
	if(entry_number>=journal_records.size())return;

	//show journal text
	if(journal_records[entry_number].record_type==0){
		OK=show_journal(journal_records[entry_number].record_parameter0, journal_records[entry_number].record_parameter1);
	}
	//plot_object found text
	else if(journal_records[entry_number].record_type==1){
		OK=true;
		for(int i=0;i<mod.general_plot_objects[journal_records[entry_number].record_parameter0].effects.size();i++){
			if(mod.general_plot_objects[journal_records[entry_number].record_parameter0].effects[i].event_text!="null")
				log_text=mod.general_plot_objects[journal_records[entry_number].record_parameter0].effects[i].event_text;
		}
	}
	//change map text
	else if(journal_records[entry_number].record_type==2){
		OK=true;
		log_text=mod.general_areas[journal_records[entry_number].record_parameter0].on_enter_text;
	}
	//item found text
	else if(journal_records[entry_number].record_type==3){
		OK=true;
		log_text=mod.general_items[journal_records[entry_number].record_parameter0].event_text;
	}
	//item found text
	else if(journal_records[entry_number].record_type==4){
		OK=true;
		log_text=mod.scripts[journal_records[entry_number].record_parameter0].message;
	}

	if(OK){
		current_showing_entry=entry_number;
		if(pop_up_mode!=0)
			previous_pop_up_mode=pop_up_mode;
		pop_up_mode=0;
	}


}

void game_engine::draw_targeting_beam(void){

	//6=set targeting beam, type parameter1 (0=disabled, 1=normal, 2=turns green when hits enemy), length = parameter2 + weapon length * parameter3

	if(weapon_selected_from_item_list<0)return;

	const int creature=0;
    creature_base& thiscreature = map_main->creature[creature];

	float size=mod.general_creatures[thiscreature.type].size*thiscreature.size*general_creature_size;
	float shoot_x=thiscreature.x+size*0.5f+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].weapon_class].x*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation)+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].weapon_class].y*(size/general_creature_size)*sincos.table_cos(thiscreature.rotation-pi/2);
	float shoot_y=thiscreature.y+size*0.5f+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].weapon_class].x*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation)+mod.general_races[player_race].weapon_classes[mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].weapon_class].y*(size/general_creature_size)*sincos.table_sin(thiscreature.rotation-pi/2);


	float range=beam_type.parameter3*mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].time*mod.general_weapons[mod.general_creatures[thiscreature.type].weapon].bullet_speed+beam_type.parameter2;
	float shoot_at_x=shoot_x+range*sincos.table_cos(thiscreature.rotation-pi/2);
	float shoot_at_y=shoot_y+range*sincos.table_sin(thiscreature.rotation-pi/2);

	//find if there's an enemy at the path
	bool enemy_found=false;
	if(beam_type.parameter1==2){
		vector <collision_base> collisions=list_collisions(shoot_x,shoot_y,shoot_at_x,shoot_at_y,false);
		for(int a=0;a<collisions.size();a++){
			if(collisions[a].type==0
                && collisions[a].subtype>0
                && map_main->creature[collisions[a].subtype].side!=0
			    && mod.general_creatures[map_main->creature[collisions[a].subtype].type].particle_on_radar>=0) {
                    enemy_found=true;
            }
		}
	}


	if(enemy_found)
		text_manager.draw_line(shoot_x-camera_x,shoot_y-camera_y,shoot_at_x-camera_x,shoot_at_y-camera_y,1.2f,0.4f,0.3f,0,1,0);
	else
		text_manager.draw_line(shoot_x-camera_x,shoot_y-camera_y,shoot_at_x-camera_x,shoot_at_y-camera_y,1,0.4f,0.3f,1,0,0);



}

int game_engine::arrange_item_list_callback(const void *c, const void *d){


	int i,j;

	item_list_object *a = (item_list_object *)c;
	item_list_object *b = (item_list_object *)d;


	if(a->item<=-1){return -1;}
	if(b->item<=-1){return 1;}
	if(a->amount<=0){return -1;}
	if(b->amount<=0){return 1;}

	if(a->item>=engine->mod.general_items.size()){return -1;}
	if(b->item>=engine->mod.general_items.size()){return 1;}

	//is a weapon
	bool weapon_1=false;
	bool weapon_2=false;
	for(i=0;i<engine->mod.general_items[a->item].effects.size();i++){
		for(j=0;j<engine->mod.general_items[a->item].effects[i].effect.effects.size();j++){
			if(engine->mod.general_items[a->item].effects[i].effect.effects[j].effect_number==9){
				weapon_1=true;
			}
		}
	}
	for(i=0;i<engine->mod.general_items[b->item].effects.size();i++){
		for(j=0;j<engine->mod.general_items[b->item].effects[i].effect.effects.size();j++){
			if(engine->mod.general_items[b->item].effects[i].effect.effects[j].effect_number==9){
				weapon_2=true;
			}
		}
	}


	if(weapon_1&&weapon_2){
		if(engine->mod.general_items[a->item].identifier==engine->mod.general_items[b->item].identifier)return 0;
		if(engine->mod.general_items[a->item].identifier>engine->mod.general_items[b->item].identifier)return 1;
		if(engine->mod.general_items[a->item].identifier<engine->mod.general_items[b->item].identifier)return -1;
	}
	if(weapon_1&&!weapon_2)return -1;
	if(!weapon_1&&weapon_2)return 1;

	//neither a weapon, arrange by time or alphabet
	if(!weapon_1&&!weapon_2){
		if(engine->arrange_alphabetically){
			if(engine->mod.general_items[a->item].name>engine->mod.general_items[b->item].name)return 1;
			if(engine->mod.general_items[a->item].name<engine->mod.general_items[b->item].name)return -1;
			return 0;
		}
		else{
			if(a->time_stamp>b->time_stamp)return 1;
			if(a->time_stamp<b->time_stamp)return -1;
			if(a->time_stamp==b->time_stamp)return 0;
			return 0;
		}
	}



	return 0;

}


void game_engine::arrange_item_list(bool frame_start){

	int a,b;

	//called from anywhere but the frame start
	if(!frame_start)
		arrange_item_list_called=true;

	//called from frame start, now we rearrange
	if(frame_start&&arrange_item_list_called){
		arrange_item_list_called=false;

		//see if any items need to be deleted
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			if(inventory[active_inventory].player_items[a].amount<=0){
				delete_item(a);
				a--;
			}
		}

		//find guns, arrange them to the beginning
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			for(b=0;b<a;b++){
				if(arrange_item_list_callback(&inventory[active_inventory].player_items[a],&inventory[active_inventory].player_items[b])<0){
					item_list_object temp_object=inventory[active_inventory].player_items[a];
					inventory[active_inventory].player_items[a]=inventory[active_inventory].player_items[b];
					inventory[active_inventory].player_items[b]=temp_object;
				}
			}
		}


		//find the item slot that now has the player's weapon
		bool weapon_found=false;
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			if(inventory[active_inventory].player_items[a].amount<=0)continue;
			Mod::effect effect;
			if(item_has_effect(inventory[active_inventory].player_items[a].item,9,&effect))
				if(mod.general_creatures[map_main->creature[0].type].weapon==(int)effect.parameter1){
					weapon_selected_from_item_list=a;
					weapon_found=true;
					break;
				}
		}

		//refind all the used slots
		for(a=0;a<inventory[active_inventory].slot_used_by.size();a++){
			inventory[active_inventory].slot_used_by[a]=-1;
		}
		for(a=0;a<inventory[active_inventory].player_items.size();a++){
			if(inventory[active_inventory].player_items[a].wielded){
				//set slots to used
				for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots.size();b++){
					inventory[active_inventory].slot_used_by[mod.general_items[inventory[active_inventory].player_items[a].item].wield_slots[b]]=a;
				}
			}
		}

		//no weapon found
		if(!weapon_found){
			mod.general_creatures[map_main->creature[0].type].weapon=-1;
			weapon_selected_from_item_list=-1;
		}

		calculate_weight();
	}


}


void game_engine::play_animated_sequence(int number, int game_state_after){

	disable_input_override();

	game_state=4;

	animation_start_timer=1000;

	game_state_after_animation=game_state_after;

	animation_playing_number=number;
	animation_frame_current=1;

}



void game_engine::render_animation(void){

	int a;

	disable_input_override();

	animation_start_timer-=elapsed;


	grim->System_ClearScreen(0,0,0,1);

	//computer texture
	grim->System_SetState_Blending(false);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(mod.animations[animation_playing_number].texture);
	grim->Quads_SetColor(1,1,1,1);

	grim->Quads_Begin();
		for(a=0;a<animation_frame_current;a++){
			//draw frame
			grim->Quads_SetSubset(mod.animations[animation_playing_number].frame[a].start_x/1024.0f,mod.animations[animation_playing_number].frame[a].start_y/768.0f,mod.animations[animation_playing_number].frame[a].end_x/1024.0f,mod.animations[animation_playing_number].frame[a].end_y/768.0f);
			float x_0=mod.animations[animation_playing_number].frame[a].start_x/1024.0f*screen_width;
			float y_0=mod.animations[animation_playing_number].frame[a].start_y/768.0f*screen_height;
			float x_1=mod.animations[animation_playing_number].frame[a].end_x/1024.0f*screen_width;
			float y_1=mod.animations[animation_playing_number].frame[a].end_y/768.0f*screen_height;

			grim->Quads_Draw(x_0, y_0, x_1-x_0, y_1-y_0);

		}
	grim->Quads_End();


	text_manager.write(font,mod.animations[animation_playing_number].frame[animation_frame_current-1].text,1.7f/1024.0f*screen_width,100/1024.0f*screen_width,mod.animations[animation_playing_number].frame[animation_frame_current-1].text_y/768.0f*screen_height,800/1024.0f*screen_width,screen_height,false,1,1,1,1);


	//advance frames
	if(animation_start_timer<0){
		//advance by mouse
		if((mouse_right&&!mouse_right2)||(!mouse_left&&mouse_left2)){
			animation_frame_current++;
		}
		//advance by x or i or escape or enter
		if((key_enter&&!key_enter2)||(!key_escape&&key_escape2)){
			animation_frame_current++;
		}

		//end animation
		if(animation_frame_current>mod.animations[animation_playing_number].frames){
			submenu=0;
			create_menu_items();
			game_state=game_state_after_animation;
			//quit_game=true;
		}
	}

}


void game_engine::render_credits(void){


	animation_start_timer-=elapsed;


	grim->System_ClearScreen(1,1,1,1);

	//computer texture
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_Blending(true);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(credits_texture);
	grim->Quads_SetColor(1,1,1,0.6f);
	grim->Quads_Begin();
	grim->Quads_Draw(0, 0, screen_width, screen_height);
	grim->Quads_End();
	/*
	grim->Quads_Begin();
		for(a=0;a<animation_frame_current;a++){
			//draw frame
			grim->Quads_SetSubset(mod.animations[animation_playing_number].frame[a].start_x/1024.0f,mod.animations[animation_playing_number].frame[a].start_y/768.0f,mod.animations[animation_playing_number].frame[a].end_x/1024.0f,mod.animations[animation_playing_number].frame[a].end_y/768.0f);
			float x_0=mod.animations[animation_playing_number].frame[a].start_x/1024.0f*screen_width;
			float y_0=mod.animations[animation_playing_number].frame[a].start_y/768.0f*screen_height;
			float x_1=mod.animations[animation_playing_number].frame[a].end_x/1024.0f*screen_width;
			float y_1=mod.animations[animation_playing_number].frame[a].end_y/768.0f*screen_height;

			grim->Quads_Draw(x_0, y_0, x_1-x_0, y_1-y_0);

		}
	grim->Quads_End();


	text_manager.write(font,mod.animations[animation_playing_number].frame[animation_frame_current-1].text,1.7f/1024.0f*screen_width,100/1024.0f*screen_width,mod.animations[animation_playing_number].frame[animation_frame_current-1].text_y/768.0f*screen_height,800/1024.0f*screen_width,screen_height,false,1,1,1,1);
	*/

	grim->System_SetState_Blending(true);
	grim->Quads_SetColor(0,0,0,1);
	text_manager.write_line(font,144/1024.0f*screen_width,65/768.0f*screen_height,"Notrium "+game_version,3/1024.0f*screen_width);
	grim->Quads_SetColor(1,1,1,1);
	text_manager.write_line(font,147/1024.0f*screen_width,68/768.0f*screen_height,"Notrium "+game_version,3/1024.0f*screen_width);

	string credits("Developed by: Ville M�nkk�nen \\ In association with: Michael Quigley aka Quanrian \\ Music by: Kush Diarra  \\ Based on design by: Mikko Tikkanen \\ Using Grim 2D graphics engine \\ \\ Beta crew: \\ Robbie BT aka ZeXLR8er!! \\ Sergio Enriquez aka Torment aka Casanova \\ Nick Atherley aka Eternal \\ Carl S. aka Click \\ \\ \\ Press enter to continue");



	grim->Quads_SetColor(1,1,1,1);
	text_manager.write(font,credits,1.6f/1024.0f*screen_width,263/1024.0f*screen_width,(193)/768.0f*screen_height,screen_width,screen_height,true,1,1,1,1);
	text_manager.write(font,credits,1.6f/1024.0f*screen_width,266/1024.0f*screen_width,(196)/768.0f*screen_height,screen_width,screen_height,false,1,1,1,1);



	//return to menu
	bool end=false;
	//advance by mouse
	if((mouse_right&&!mouse_right2)||(!mouse_left&&mouse_left2)){
		end=true;
	}
	//advance by x or i or escape or enter
	if((key_enter&&!key_enter2)||(!key_escape&&key_escape2))
		end=true;

	//end
	if(end){
		submenu=0;
		create_menu_items();
		game_state=5;
	}

}


void game_engine::save_game(int slot){


	if(!game_running)return;

	int a,b,area;

	last_saved_game=slot;

// 	SYSTEMTIME current_time;
// 	GetLocalTime(&current_time);


		FILE *fil;
		char tallennusrivi[300];
		char rivi[300];

		//itoa(slot,temprivi,10);
		sprintf(temprivi,"%d",slot);
		strcpy(tallennusrivi,"save/s");
		strcat(tallennusrivi,temprivi);
		strcat(tallennusrivi,".sav");

		fil = fopen(tallennusrivi,"wb");
		if(!fil)return;

// 		//name
// 		//itoa((int)(current_time.wDay),rivi,10);
// 		sprintf(rivi,"%d",(int)(current_time.wDay));
// 		strcat(rivi,".");
// 		//itoa((int)(current_time.wMonth),temprivi,10);strcat(rivi,temprivi);
// 		sprintf(temprivi,"%d",current_time.wMonth);strcat(rivi,temprivi);
// 		strcat(rivi,".");
// 		//itoa((int)(current_time.wYear),temprivi,10);strcat(rivi,temprivi);
// 		sprintf(temprivi,"%d",current_time.wYear);strcat(rivi,temprivi);
// 		strcat(rivi," ");
// 		if(current_time.wHour<10)strcat(rivi,"0");
// 		//itoa((int)(current_time.wHour),temprivi,10);strcat(rivi,temprivi);
// 		sprintf(temprivi,"%d",current_time.wHour);strcat(rivi,temprivi);
// 		strcat(rivi,":");
// 		if(current_time.wMinute<10)strcat(rivi,"0");
// 		//itoa((int)(current_time.wMinute),temprivi,10);strcat(rivi,temprivi);
// 		sprintf(temprivi,"%d",current_time.wMinute);strcat(rivi,temprivi);

		time_t ltime;
		time(&ltime);
		strftime(rivi,sizeof(rivi)-1,"%d.%m.%y %H:%M",localtime(&ltime));
		fprintf(fil, "%s\n",rivi);

		//save game name
		fprintf(fil, "%s\n", save_game_name.c_str());



		//mod name
		fprintf(fil, "%s\n", mod.mod_name.c_str());


		//maps
		/*fprintf(fil, "%d\n", areas_x);
		fprintf(fil, "%d\n", areas_y);

		for(area=0;area<areas_x*areas_y;area++){*/

		fprintf(fil, "%d\n", map_storage.size());
		for(area=0;area<map_storage.size();area++){


			//fwrite (&map_storage[area], 1, sizeof(map_storage[area]), fil);

			//construction parameters
			fprintf(fil, "%d\n", map_storage[area]->sizex);
			fprintf(fil, "%d\n", map_storage[area]->sizey);
			fprintf(fil, "%d\n", map_storage[area]->object.size());
			fprintf(fil, "%d\n", map_storage[area]->creature.size());
			fprintf(fil, "%d\n", map_storage[area]->items.size());
			fprintf(fil, "%d\n", map_storage[area]->climate_number);
			fprintf(fil, "%d\n", map_storage[area]->area_type);
			fprintf(fil, "%d\n", (int)map_storage[area]->been_here_already);

			fprintf(fil, "%f\n", map_storage[area]->wind_speed);
			fprintf(fil, "%f\n", map_storage[area]->wind_direction);
			fprintf(fil, "%f\n", map_storage[area]->wind_direction_target);
			fprintf(fil, "%f\n", map_storage[area]->rain_timer);

			//near areas
			for(a=0;a<4;a++){
				fprintf(fil, "%d\n", map_storage[area]->near_areas[a]);
			}




			//grid
			for(a=0;a<map_storage[area]->sizex;a++){
				for(b=0;b<map_storage[area]->sizey;b++){
					fprintf(fil, "%d\n", map_storage[area]->grid[a].grid[b].terrain_type);
				}
			}

			//objects
			for(a=0;a<map_storage[area]->object.size();a++){
				fwrite (&map_storage[area]->object[a], 1, sizeof(map_storage[area]->object[a]), fil);
			}
			//creatures
			for(a=0;a<map_storage[area]->creature.size();a++){
				fprintf(fil, "%d\n", sizeof(map_storage[area]->creature[a]));
				fwrite (&map_storage[area]->creature[a], 1, sizeof(map_storage[area]->creature[a]), fil);
			}
			//lights
			fprintf(fil, "%d\n", map_storage[area]->lights.size());
			for(a=0;a<map_storage[area]->lights.size();a++){
				fwrite (&map_storage[area]->lights[a], 1, sizeof(map_storage[area]->lights[a]), fil);
			}
			//bullets
			fprintf(fil, "%d\n", map_storage[area]->bullets.size());
			list<bullet>::iterator it;
			for(it=map_storage[area]->bullets.begin(); it!=map_storage[area]->bullets.end(); it++){
				fwrite (&(*it), 1, sizeof((*it)), fil);
			}
			//items
			for(a=0;a<map_storage[area]->items.size();a++){
				fwrite (&map_storage[area]->items[a], 1, sizeof(map_storage[area]->items[a]), fil);
			}

		}


		//player items
		fprintf(fil, "%d\n", inventory.size());
		for(b=0;b<inventory.size();b++){
			//items
			fprintf(fil, "%d\n", inventory[b].player_items.size());
			for(a=0;a<inventory[b].player_items.size();a++){
				fprintf(fil, "%d\n", inventory[b].player_items[a].item);
				fprintf(fil, "%f\n", inventory[b].player_items[a].time_stamp);
				fprintf(fil, "%d\n", inventory[b].player_items[a].amount);
				fprintf(fil, "%d\n", (int)inventory[b].player_items[a].wielded);
			}

			//used slots
			fprintf(fil, "%d\n", inventory[b].slot_used_by.size());
			for(a=0;a<inventory[b].slot_used_by.size();a++){
				fprintf(fil, "%d\n", inventory[b].slot_used_by[a]);
			}
		}


		fprintf(fil, "%f\n", time_from_beginning);
		fprintf(fil, "%d\n", weapon_selected_from_item_list);
		//fprintf(fil, "%d\n", aliens_killed);
		fprintf(fil, "%f\n", day_timer);
		fprintf(fil, "%d\n", day_number);
		fprintf(fil, "%f\n", scanner_active);
		//fprintf(fil, "%f\n", battery_energy);
		//fprintf(fil, "%f\n", maximum_battery_energy);
		fprintf(fil, "%f\n", maximum_carry_weight);
		fprintf(fil, "%d\n", beam_active);
		fprintf(fil, "%d\n", beam_type.effect_number);
		fprintf(fil, "%f\n", beam_type.parameter1);
		fprintf(fil, "%f\n", beam_type.parameter2);
		fprintf(fil, "%f\n", beam_type.parameter3);
		fprintf(fil, "%f\n", beam_type.parameter4);
		//fprintf(fil, "%d\n", flashlight_level);
		for(a=0;a<4;a++){
			for(b=0;b<3;b++){
				fprintf(fil, "%f\n", light_addition[b][a]);
			}
		}
		fprintf(fil, "%f\n", armor);
		fprintf(fil, "%f\n", kill_meter_active);
		fprintf(fil, "%d\n", game_difficulty_level);
		fprintf(fil, "%d\n", alien_attack);
		fprintf(fil, "%f\n", body_temperature);
		fprintf(fil, "%d\n", player_area);
		fprintf(fil, "%d\n", player_race);
		fprintf(fil, "%d\n", mod.general_creatures[map_main->creature[0].type].weapon);
		fprintf(fil, "%d\n", map_active);
		fprintf(fil, "%d\n", current_showing_entry);
		fprintf(fil, "%f\n", stagger_mouse_time);
		fprintf(fil, "%f\n", stagger_mouse_parameter1);
		fprintf(fil, "%f\n", stagger_mouse_parameter2);
		fprintf(fil, "%f\n", game_speed);
		fprintf(fil, "%f\n", screen_shake_power);
		fprintf(fil, "%f\n", screen_shake_time);
		fprintf(fil, "%d\n", old_player_race);
		fprintf(fil, "%d\n", attach_camera_type);
		fprintf(fil, "%f\n", attach_camera_time);
		fprintf(fil, "%f\n", attach_camera_parameter1);
		fprintf(fil, "%f\n", attach_camera_parameter2);
		fprintf(fil, "%f\n", real_camera_x);
		fprintf(fil, "%f\n", real_camera_y);
		fprintf(fil, "%d\n", player_controlled_creature);
		fprintf(fil, "%d\n", active_inventory);
		fprintf(fil, "%d\n", mouse_visible);
		fprintf(fil, "%d\n", override_player_controls);
		fprintf(fil, "%d\n", can_view_inventory);
		fprintf(fil, "%d\n", can_drop_items);
		fprintf(fil, "%d\n", show_radar);











		//seen object texts
		fprintf(fil, "%d\n", seen_item_text.size());
		for(a=0;a<seen_item_text.size();a++){
			fprintf(fil, "%d\n", (int)seen_item_text[a]);
		}
		fprintf(fil, "%d\n", seen_plot_object_text.size());
		for(a=0;a<seen_plot_object_text.size();a++){
			fprintf(fil, "%d\n", (int)seen_plot_object_text[a]);
		}

		//terrain timers
		fprintf(fil, "%d\n", mod.terrain_types.size());
		for(a=0;a<mod.terrain_types.size();a++){
			fprintf(fil, "%d\n", mod.terrain_types[a].effects.size());
			for(b=0;b<mod.terrain_types[a].effects.size();b++){
				fprintf(fil, "%f\n", terrain_timers[a].subtype[b]);
			}
		}
		//rain timers
		fprintf(fil, "%d\n", rain_effect_timers.size());
		for(a=0;a<rain_effect_timers.size();a++){
			fprintf(fil, "%d\n", rain_effect_timers[a].subtype.size());
			for(b=0;b<rain_effect_timers[a].subtype.size();b++){
				fprintf(fil, "%f\n", rain_effect_timers[a].subtype[b]);
			}
		}



		//journal records
		fprintf(fil, "%d\n", journal_records.size());
		for(a=0;a<journal_records.size();a++){
			fprintf(fil, "%d\n", journal_records[a].record_type);
			fprintf(fil, "%d\n", journal_records[a].record_parameter0);
			fprintf(fil, "%d\n", journal_records[a].record_parameter1);
		}

		//save scripts
		fprintf(fil, "%d\n", script_info.size());
		for(a=0;a<script_info.size();a++){
			fprintf(fil, "%d\n", (int)script_info[a].dead);
			fprintf(fil, "%f\n", script_info[a].timer);
			fprintf(fil, "%f\n", script_info[a].script_calculated_on);
		}

		//save bars
		fprintf(fil, "%d\n", mod.general_bars.size());
		for(a=0;a<mod.general_bars.size();a++){
			fprintf(fil, "%d\n", (int)mod.general_bars[a].visible);
		}








		fclose(fil);

		text_manager.message(3000,1000,"Game Saved");
		playsound(UI_menu_click[0],1,0,0,0,0);

	//	luesavet();

}

void game_engine::load_game(int slot){

	int a,b,area;


	text_manager.accept_messages=false;
	game_running=true;


		FILE *fil;
		char tallennusrivi[300];

		//itoa(slot,temprivi,10);
		sprintf(temprivi,"%d",slot);
		strcpy(tallennusrivi,"save/s");
		strcat(tallennusrivi,temprivi);
		strcat(tallennusrivi,".sav");

		fil = fopen(tallennusrivi,"rb");
		if(!fil)return;


		//clear old maps
		for(a=0;a<map_storage.size();a++){
			//first delete old minimaps)
			if(map_storage[a]->map_texture_2>=0){
				grim->Texture_Delete(map_storage[a]->map_texture);
			}
			if(map_storage[a]->map_texture_2>=0){
				grim->Texture_Delete(map_storage[a]->map_texture_2);
			}
			SAFE_DELETE(map_storage[a]);
		}
		map_storage.clear();


		//date
		fgets(temprivi,sizeof(temprivi),fil);

		//save game name
		fgets(temprivi,sizeof(temprivi),fil);


		//mod name
		stripped_fgets(temprivi,sizeof(temprivi),fil);
		mod.mod_name=temprivi;

		//load mod
		load_mod(mod.mod_name);

		//maps
		/*fgets(temprivi,sizeof(temprivi),fil);
		areas_x=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		areas_y=atoi(temprivi);

		//SAFE_DELETE(map_main);

		//load individual maps
		for(area=0;area<areas_x*areas_y;area++){*/

		//number of areas stored
		fgets(temprivi,sizeof(temprivi),fil);
		int areas=atoi(temprivi);


		for(area=0;area<areas;area++){

			//construction parameters
			fgets(temprivi,sizeof(temprivi),fil);
			int map_width=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			int map_height=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			int total_objects=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			int total_creatures=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			int total_items=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			int climate=atoi(temprivi);

			//new map object
			//SAFE_DELETE(map_storage[area]);
			map *temp_map=NULL;
			temp_map=new map(map_width,map_height, total_creatures, total_objects, total_items, climate);
			//temp_map = new map(map_width,map_height, total_creatures, total_objects, total_items, climate);

			//map specialties
			fgets(temprivi,sizeof(temprivi),fil);
			temp_map->area_type=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_map->been_here_already=strtobool(temprivi);

			fgets(temprivi,sizeof(temprivi),fil);
			temp_map->wind_speed=atof(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_map->wind_direction=atof(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_map->wind_direction_target=atof(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_map->rain_timer=atof(temprivi);

			//near areas
			for(a=0;a<4;a++){
				fgets(temprivi,sizeof(temprivi),fil);
				temp_map->near_areas[a]=atoi(temprivi);
			}


			//grid
			for(a=0;a<temp_map->sizex;a++){
				for(b=0;b<temp_map->sizey;b++){
					fgets(temprivi,sizeof(temprivi),fil);
					temp_map->grid[a].grid[b].terrain_type=atoi(temprivi);
				}
			}

			//objects
			temp_map->object.clear();
			for(a=0;a<total_objects;a++){
				map_object temp_object;
				fread (&temp_object, 1, sizeof(temp_object), fil);
				temp_map->object.push_back(temp_object);
			}

			//creatures
			temp_map->creature.clear();
			for(a=0;a<total_creatures;a++){
				int size=atoi(fgets(temprivi,sizeof(temprivi),fil));
				creature_base temp_creature;
				fread (&temp_creature, 1, size, fil);
				//if(!temp_creature.dead){
					//temp_creature.move_to_x=temp_creature.x;
					//temp_creature.move_to_y=temp_creature.y;
					temp_map->creature.push_back(temp_creature);
				//}
			}

			//lights
			fgets(temprivi,sizeof(temprivi),fil);
			int light_amount=atoi(temprivi);
			for(a=0;a<light_amount;a++){
				light temp_light;
				fread (&temp_light, 1, sizeof(temp_light), fil);
				//if(!temp_light.dead)
					temp_map->lights.push_back(temp_light);
			}

			//bullets
			fgets(temprivi,sizeof(temprivi),fil);
			int bullets=atoi(temprivi);
			for(a=0;a<bullets;a++){
				bullet temp_bullet;
				fread (&temp_bullet, 1, sizeof(temp_bullet), fil);
				temp_map->bullets.push_back(temp_bullet);

			}

			//items
			for(a=0;a<total_items;a++){
				item temp_item;
				fread (&temp_item, 1, sizeof(temp_item), fil);
				if(!temp_item.dead)
					temp_map->items.push_back(temp_item);
			}


			temp_map->check_creatures();
			temp_map->initialize_items();
			temp_map->initialize_objects();
			initialize_animation_frames(temp_map);
			create_minimap(temp_map, area);

			map_storage.push_back(temp_map);

		}




		//player items
		inventory.clear();
		fgets(temprivi,sizeof(temprivi),fil);
		int inventories_amount=atoi(temprivi);
		for(b=0;b<inventories_amount;b++){
			inventory_base temp_inventory;

			//items
			fgets(temprivi,sizeof(temprivi),fil);
			int player_items_amount=atoi(temprivi);
			temp_inventory.player_items.clear();
			for(a=0;a<player_items_amount;a++){
				item_list_object temp_item;

				fgets(temprivi,sizeof(temprivi),fil);
				temp_item.item=atoi(temprivi);
				fgets(temprivi,sizeof(temprivi),fil);
				temp_item.time_stamp=atof(temprivi);
				fgets(temprivi,sizeof(temprivi),fil);
				temp_item.amount=atoi(temprivi);
				fgets(temprivi,sizeof(temprivi),fil);
				temp_item.wielded=strtobool(temprivi);

				if(temp_item.amount>0)
					temp_inventory.player_items.push_back(temp_item);
			}

			//used slots
			temp_inventory.slot_used_by.clear();
			fgets(temprivi,sizeof(temprivi),fil);
			int slots_needed=atoi(temprivi);
			for(a=0;a<slots_needed;a++){
				fgets(temprivi,sizeof(temprivi),fil);
				temp_inventory.slot_used_by.push_back(atoi(temprivi));
			}

			inventory.push_back(temp_inventory);
		}



		fgets(temprivi,sizeof(temprivi),fil);
		time_from_beginning=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		weapon_selected_from_item_list=atoi(temprivi);
		//fgets(temprivi,sizeof(temprivi),fil);
		//aliens_killed=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		day_timer=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		day_number=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		scanner_active=atof(temprivi);
		//fgets(temprivi,sizeof(temprivi),fil);
		//battery_energy=atof(temprivi);
		//fgets(temprivi,sizeof(temprivi),fil);
		//maximum_battery_energy=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		maximum_carry_weight=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		beam_active=strtobool(temprivi);
		Mod::effect temp_effect;
		fgets(temprivi,sizeof(temprivi),fil);
		temp_effect.effect_number=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		temp_effect.parameter1=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		temp_effect.parameter2=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		temp_effect.parameter3=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		temp_effect.parameter4=atof(temprivi);
		beam_type=temp_effect;

		for(a=0;a<4;a++){
			for(b=0;b<3;b++){
				fgets(temprivi,sizeof(temprivi),fil);
				light_addition[b][a]=atof(temprivi);
			}
		}

		fgets(temprivi,sizeof(temprivi),fil);
		armor=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		kill_meter_active=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		game_difficulty_level=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		alien_attack=strtobool(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		body_temperature=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		player_area=atoi(temprivi);

		//select new map_main
		map_main=map_storage[player_area];

		fgets(temprivi,sizeof(temprivi),fil);
		player_race=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		mod.general_creatures[map_main->creature[0].type].weapon=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		map_active=strtobool(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		current_showing_entry=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		stagger_mouse_time=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		stagger_mouse_parameter1=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		stagger_mouse_parameter2=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		game_speed=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		screen_shake_power=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		screen_shake_time=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		old_player_race=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		attach_camera_type=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		attach_camera_time=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		attach_camera_parameter1=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		attach_camera_parameter2=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		real_camera_x=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		real_camera_y=atof(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		player_controlled_creature=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		active_inventory=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		mouse_visible=strtobool(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		override_player_controls=atoi(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		can_view_inventory=strtobool(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		can_drop_items=strtobool(temprivi);
		fgets(temprivi,sizeof(temprivi),fil);
		show_radar=strtobool(temprivi);




		//seen object texts
		seen_item_text.clear();
		fgets(temprivi,sizeof(temprivi),fil);
		int seen_items=atoi(temprivi);
		for(a=0;a<seen_items;a++){
			fgets(temprivi,sizeof(temprivi),fil);
			seen_item_text.push_back(strtobool(temprivi));
		}
		seen_plot_object_text.clear();
		fgets(temprivi,sizeof(temprivi),fil);
		int seen_plot_objects=atoi(temprivi);
		for(a=0;a<seen_plot_objects;a++){
			fgets(temprivi,sizeof(temprivi),fil);
			seen_plot_object_text.push_back(strtobool(temprivi));
		}

		//terrain timers
		terrain_timers.clear();
		fgets(temprivi,sizeof(temprivi),fil);
		int timers=atoi(temprivi);
		for(a=0;a<timers;a++){
			timer_base temp_timer;
			fgets(temprivi,sizeof(temprivi),fil);
			int subtypes=atoi(temprivi);
			for(b=0;b<subtypes;b++){
				fgets(temprivi,sizeof(temprivi),fil);
				temp_timer.subtype.push_back(atof(temprivi));
			}
			terrain_timers.push_back(temp_timer);
		}
		//rain timers
		rain_effect_timers.clear();
		fgets(temprivi,sizeof(temprivi),fil);
		timers=atoi(temprivi);
		for(a=0;a<timers;a++){
			timer_base temp_timer;
			fgets(temprivi,sizeof(temprivi),fil);
			int subtypes=atoi(temprivi);
			for(b=0;b<subtypes;b++){
				fgets(temprivi,sizeof(temprivi),fil);
				temp_timer.subtype.push_back(atof(temprivi));
			}
			rain_effect_timers.push_back(temp_timer);
		}


		//journal records
		journal_records.clear();
		fgets(temprivi,sizeof(temprivi),fil);
		int records=atoi(temprivi);
		for(a=0;a<records;a++){
			journal_record temp_record;
			fgets(temprivi,sizeof(temprivi),fil);
			temp_record.record_type=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_record.record_parameter0=atoi(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_record.record_parameter1=atoi(temprivi);
			journal_records.push_back(temp_record);
		}


		//scripts
		fgets(temprivi,sizeof(temprivi),fil);
		script_info.clear();
		int scripts_amount=atoi(temprivi);
		for(a=0;a<scripts_amount;a++){
			script_info_base temp_info;
			//if(a>=scripts.size())continue;

			fgets(temprivi,sizeof(temprivi),fil);
			temp_info.dead=strtobool(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_info.timer=atof(temprivi);
			fgets(temprivi,sizeof(temprivi),fil);
			temp_info.script_calculated_on=atof(temprivi);

			script_info.push_back(temp_info);
		}

		//bars
		fgets(temprivi,sizeof(temprivi),fil);
		int bars_amount=atoi(temprivi);
		for(a=0;a<bars_amount;a++){
			fgets(temprivi,sizeof(temprivi),fil);
			mod.general_bars[a].visible=strtobool(temprivi);

		}


		fclose(fil);


	//calculate places
	/*map_main->check_creatures();
	map_main->initialize_items();
	map_main->initialize_objects();*/
	//flash_light();
//	calculate_lights();







	creatures_checked_on=time_from_beginning;
	creature_visibility_checked_on=time_from_beginning+randDouble(0,1);
	//scripts_calculated_on=time_from_beginning+0.5f;



	//record normal speed
	player_normal_speed=mod.general_creatures[map_main->creature[0].type].movement_speed;

	ask_continue_game=true;
	pop_up_mode=1;
	previous_pop_up_mode=1;
	combine_item=-1;
	combine_item_from_wield=false;
	shortest_distance_to_alien=100000000;
	slider_active=false;
	change_map_to=-1;
	change_map_player_x=0;
	change_map_player_y=0;
	arrange_item_list(false);

	text_manager.accept_messages=true;
	text_manager.message(3000,1000,"Game Loaded");

}

void game_engine::load_mod(const string& mod_name){

	int a,b;

	resources.initialize_resource_handler(grim,&debug,g_pSoundManager,play_sound, sound_initialized);

	//a good time to unload all textures
	resources.unload_unneeded_textures(true);

	debug.debug_output("Mod Loading", 1,0);

	//combine_screen=resources.load_texture("combine.jpg");
	//detector_display=resources.load_texture("detector2.png");
	//computer_texture=resources.load_texture("computer.png");
	item_view=resources.load_texture("itemview.png",mod_name);
	slider_texture=resources.load_texture("slider.png",mod_name);
	mouse_texture[0]=resources.load_texture("mouse0.png",mod_name);
	mouse_texture[1]=resources.load_texture("mouse1.png",mod_name);
	mouse_texture[2]=resources.load_texture("mouse2.png",mod_name);

	//sounds
	pick_up=resources.load_sample("pick_up.wav",2,mod.mod_name);
	UI_game_click[0]=resources.load_sample("click0.wav",2,mod.mod_name);
	UI_game_click[1]=resources.load_sample("click1.wav",2,mod.mod_name);
	UI_game_click[2]=resources.load_sample("click2.wav",2,mod.mod_name);


	//load_object_info("data/"+mod_name+"/object_definitions.dat");
	//load_climate_info("data/"+mod_name+"/climate_types.dat");
	//load_creature_info("data/"+mod_name+"/creatures.dat");
	//load_light_info("data/"+mod_name+"/light.dat");
	//load_weapon_info("data/"+mod_name+"/weapons.dat");
	//load_plot_object_info("data/"+mod_name+"/plot_objects.dat");
	//load_item_info("data/"+mod+"/items.dat");
	//load_animation_info("data/"+mod_name+"/animation.dat");
	//load_area_info("data/"+mod_name+"/areas.dat");
	//load_race_info("data/"+mod_name+"/player_races.dat");

	load_particles("data/"+mod_name+"/particles.dat");
	load_sounds("data/"+mod_name+"/sounds.dat");

	debug.debug_output("Mod Loading", 0,0);


	mod.load_mod(mod_name,&debug,&resources);


	//initialize seen item texts
	seen_item_text.clear();
	quick_keys.clear();
	for(a=0;a<mod.general_items.size();a++){
		seen_item_text.push_back(false);
		for(b=0;b<mod.general_items[a].effects.size();b++){
			if(mod.general_items[a].effects[b].quick_key!=0){
				quick_uses key;
				key.item_type=mod.general_items[a].identifier;
				key.key=mod.general_items[a].effects[b].quick_key;
				quick_keys.push_back(key);
			}
		}
	}
	seen_plot_object_text.clear();
	for(a=0;a<mod.general_plot_objects.size();a++){
		seen_plot_object_text.push_back(false);
	}
	for(b=0;b<inventory.size();b++){
		inventory[b].slot_used_by.clear();
		for(a=0;a<mod.general_races.size();a++){
			while(inventory[b].slot_used_by.size()<mod.general_races[a].slots.size())
				inventory[b].slot_used_by.push_back(-1);
		}
	}
	script_info.clear();
	for(a=0;a<mod.scripts.size();a++){
		script_info_base temp_info;
		temp_info.dead=mod.scripts[a].dead;
		if(!mod.scripts[a].disable_after_first_use)
			temp_info.script_calculated_on=0;
		else
			temp_info.script_calculated_on=randDouble(0,mod.scripts[a].interval*0.001f);
		temp_info.timer=0;
		script_info.push_back(temp_info);
	}
	//terrain timers
	terrain_timers.clear();
	for(a=0;a<mod.terrain_types.size();a++){
		timer_base temp_timer;
		for(b=0;b<mod.terrain_types[a].effects.size();b++){
			temp_timer.subtype.push_back(0);
		}
		terrain_timers.push_back(temp_timer);
	}
	//rain timers
	rain_effect_timers.clear();
	for(a=0;a<mod.general_climates.size();a++){
		timer_base temp_timer;
		for(b=0;b<mod.general_climates[a].rain_effects.size();b++){
			temp_timer.subtype.push_back(0);
		}
		rain_effect_timers.push_back(temp_timer);
	}



}


void game_engine::new_game(void){
	int a,b;

	text_manager.accept_messages=false;

	float sound_volume=volume_slider[0];
	volume_slider[0]=0;

	debug.debug_output("New Game Initialization", 1,0);

	game_running=true;


	//no items in beginning
	inventory.clear();
	inventory_base temp_inventory;
	temp_inventory.player_items.clear();
	temp_inventory.slot_used_by.clear();
	for(a=0;a<21;a++){
		inventory.push_back(temp_inventory);
	}
	active_inventory=0;

	//load object infos + referred textures
	mod.mod_name=mod_names[selected_mod];
	load_mod(mod.mod_name);







	item_list_place=0;
	combine_item=-1;
	combine_item_from_wield=false;




	//
	body_temperature=0;
	item_list_arranged=false;
	journal_records.clear();
	current_showing_entry=0;
	kill_meter_active=0;
	for(a=0;a<4;a++){
		for(b=0;b<3;b++){
			light_addition[b][a]=0;
		}
	}
	armor=0;
	scanner_active=0;
	beam_active=false;
	darken_timer=0;
	day_number=-1;
	//flashlight_level=0;
	day_timer=-1;//start a new day
	//day_timer=0;
	ask_quit=false;
	ask_continue_game=false;
	//aliens_killed=0;
	camera_x=0;
	camera_y=0;
	time_from_beginning=0;
	creatures_checked_on=time_from_beginning;
	creature_visibility_checked_on=time_from_beginning+randDouble(0,1);
	//scripts_calculated_on=time_from_beginning+0.5f;
	paused=false;
	pop_up_mode=1;
	previous_pop_up_mode=1;
	alien_attack=false;
	slider_active=false;
	map_active=false;
	change_map_to=-1;
	change_map_player_x=0;
	change_map_player_y=0;
	player_race=proposed_player_race;
	stagger_mouse_time=0;
	game_speed=1;
	screen_shake_time=-1;
	player_controlled_creature=0;
	old_player_race=player_race;
	override_player_controls=-1;
	mouse_visible=true;
	can_view_inventory=true;
	can_drop_items=true;
	show_radar=true;
	for(a=0;a<100;a++){
		key_clicked[a]=false;
		key_down[a]=false;
		key_down2[a]=false;
	}









	create_maps();





	//set player on the right map
	player_area=mod.general_races[player_race].start_area;
	map_main = map_storage[player_area];

	//player race
	maximum_carry_weight=mod.general_races[player_race].maximum_carry_weight;
	map_main->creature[0].side=mod.general_races[player_race].side;


	//record normal speed
	player_normal_speed=mod.general_creatures[map_main->creature[0].type].movement_speed;

	//find new camera position
	attach_camera_time=-1;
	attach_camera_type=0;
	attach_camera_parameter1=0;
	attach_camera_parameter2=0;
	float suggested_camera_x,suggested_camera_y;
	find_suggested_camera_position(&suggested_camera_x,&suggested_camera_y);
	real_camera_x=suggested_camera_x;
	real_camera_y=suggested_camera_y;





	weapon_selected_from_item_list=-1;
	//map_main->creature[0].weapon_selected=0;
	mod.general_creatures[map_main->creature[0].type].weapon=-1;

	//start specialties
	debug.debug_output("Race Specialties Initialization", 1,0);
	for(a=0;a<mod.general_races[player_race].specialties.size();a++){

		if(!mod.general_races[player_race].specialties[a].difficulty[game_difficulty_level])continue;

		//has bar number parameter0 from bars.dat with minimum of parameter1 and maximum of parameter2 and current value of parameter3 (cumulative to creatures.dat specialties)
		if(mod.general_races[player_race].specialties[a].number==5){
			//map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].value=mod.general_races[player_race].specialties[a].parameter3;
			map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].minimum=mod.general_races[player_race].specialties[a].parameter1;
			map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].maximum=mod.general_races[player_race].specialties[a].parameter2;
			map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].active=true;

			//set the value
			Mod::effect temp_effect;
			temp_effect.effect_number=4;
			temp_effect.parameter1=mod.general_races[player_race].specialties[a].parameter3;
			temp_effect.parameter2=0;
			temp_effect.parameter3=(int)mod.general_races[player_race].specialties[a].parameter0;
			run_effect(temp_effect,&map_main->creature[0],0,0,0,0,false);

		}

		//start with item parameter0 amount parameter1
		if(mod.general_races[player_race].specialties[a].number==3){
			give_item(mod.general_races[player_race].specialties[a].parameter0,mod.general_races[player_race].specialties[a].parameter1,inventory[active_inventory].player_items.size(),false);
		}
	}
	debug.debug_output("Race Specialties Initialization", 0,0);




	arrange_item_list(false);

	//select any weapon
	/*for(a=0;a<inventory[active_inventory].player_items.size();a++){
		if(inventory[active_inventory].player_items[a].amount<=0)continue;
		Mod::effect effect;
		if(item_has_effect(inventory[active_inventory].player_items[a].item,9,&effect)){

			if(mod.general_races[player_race].weapon_classes[mod.general_weapons[effect.parameter1].weapon_class].can_use==0){
				continue;
			}

			//mod.general_creatures[map_main->creature[0].type].weapon=effect.parameter1;
			for(b=0;b<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();b++){
				if(use_item(inventory[active_inventory].player_items[a].item,&a,mod.general_items[inventory[active_inventory].player_items[a].item].effects[b].effect,false,false, false))
					weapon_selected_from_item_list=a;

			}
		}
	}*/





	//start animation
	play_animated_sequence(mod.general_races[player_race].start_animation,0);

	/*for(a=0;a<maximum_general_items;a++){
		give_item(randInt(0,56),0);
	}*/
	play_music_file(-1,&last_played_music);
	volume_slider[0]=sound_volume;

	text_manager.accept_messages=true;


	debug.debug_output("New Game Initialization", 0,0);


}



void game_engine::render_menu(void){

	int a,b;

	float position=0;

	//background
	grim->System_SetState_Blending(false);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(menu);
	grim->Quads_SetColor(1,1,1,1);
	grim->Quads_SetSubset(0,0,1,1);

	grim->Quads_Begin();
		grim->Quads_Draw(0, 0, screen_width,screen_height);
	grim->Quads_End();

	//version
	grim->System_SetState_Blending(true);
	grim->Quads_SetColor(1,1,1,1);
	text_manager.write_line(font,2/1024.0f*screen_width,747/768.0f*screen_height,"Copyright 2005 Ville M�nkk�nen    Version "+game_version,1/1024.0f*screen_width);

	//text_manager.write_line(font,13/1024.0f*screen_width,600/768.0f*screen_height,"TEST VERSION - DO NOT DISTRIBUTE",3/1024.0f*screen_width);
	//text_manager.write_line(font,13/1024.0f*screen_width,650/768.0f*screen_height,"Remember to take screenshots!",3/1024.0f*screen_width);

	//escape
	if(!key_escape&&key_escape2){
		if(submenu!=0){
			playsound(UI_menu_click[0],1,0,0,0,0);
			submenu=0;
		}
		else{
			if(game_running){
				playsound(UI_menu_click[0],1,0,0,0,0);
				game_state=0;
				play_music_file(-1,&last_played_music);
			}
		}
	}

	//main menu
	grim->System_SetState_Blending(true);
	for(a=0;a<menu_system[submenu].items;a++){
		//mouse on this
		bool mouse_on_this=false;
		if((mousex>menu_system[submenu].start_x/1024.0f*screen_width)&&(mousex<menu_system[submenu].end_x/1024.0f*screen_width)
			&&(mousey>(menu_system[submenu].start_y+position)/768.0f*screen_height)&&(mousey<(menu_system[submenu].start_y+position+menu_system[submenu].item[a].height)/768.0f*screen_height))
		{
			mouse_on_this=true;
		}

		//this slot is the current selected save game slot, blink a cursor
		bool cursor=false;
		string replace_text="NULL";
		if(menu_system[submenu].item[a].effect==6){
			if(menu_system[submenu].item[a].effect_parameter==proposed_save_game){
				cursor=true;
				/*if(time_from_beginning-cursor_timer>2){
				}*/
				//change name
				for(char key='a';key<='z';key++){
					if(key_clicked[translate_key_int(key)]){
						if(grim->Key_Down(KEY_LSHIFT)||grim->Key_Down(KEY_RSHIFT))
							save_game_name+=(key-32);
						else
							save_game_name+=key;
					}
				}
				if(grim->Key_Click(KEY_BACK)){
					save_game_name=save_game_name.substr(0,save_game_name.length()-1);
				}
				if(grim->Key_Click(KEY_SPACE)){
					save_game_name+=" ";
				}
				//save the game
				if(grim->Key_Click(KEY_RETURN)||grim->Key_Click(KEY_KP_RETURN)){
					save_game(menu_system[submenu].item[a].effect_parameter);
					read_saves();
					proposed_save_game=-1;
				}
				replace_text=save_game_name;
			}
		}

		//mouse is on this
		grim->Quads_SetColor(0.8f,0.8f,0.8f,1);
		if(mouse_on_this){
			//help text
			if(menu_system[submenu].item[a].help.length()>2){
				grim->System_SetState_Blending(true);
				resources.Texture_Set(black_texture);
				//grim->Quads_SetSubset((17/256.0f),0,1,(256.0f-63)/256.0f);
				grim->Quads_SetSubset(0,0,1,1);
				grim->Quads_SetColor(1,1,1,0.8f);

				//picture for player races, need bigger black box too
				if(menu_system[submenu].item[a].effect==18){
					grim->Quads_Begin();
						grim->Quads_Draw(490/1024.0f*screen_width, 254/768.0f*screen_height, 520/1024.0f*screen_width, 270/768.0f*screen_height);
					grim->Quads_End();

					//and the race picture
					grim->Quads_SetColor(1,1,1,1.0f);
					resources.Texture_Set(race_names[selected_mod].picture[proposed_player_race]);
					grim->Quads_Begin();
						grim->Quads_Draw((750)/1024.0f*screen_width, 260/768.0f*screen_height, 256/1024.0f*screen_width, 256/768.0f*screen_height);
					grim->Quads_End();
				}
				//normal black box
				else{
					grim->Quads_Begin();
						grim->Quads_Draw(500/1024.0f*screen_width, 254/768.0f*screen_height, 370/1024.0f*screen_width, 270/768.0f*screen_height);
					grim->Quads_End();
				}

				text_manager.write(font,menu_system[submenu].item[a].help,1.5f/1024.0f*screen_width,520/1024.0f*screen_width,270/768.0f*screen_height,730/1024.0f*screen_width,768/768.0f*screen_height,false,1,1,1,1);
			}



			//click
			if(!mouse_left&&mouse_left2){
				mouse_left2=false;

				if(menu_system[submenu].item[a].effect>=0)
					playsound(UI_menu_click[0],1,0,0,0,0);

				switch(menu_system[submenu].item[a].effect){
					//new game
					case 0:
						//new_game();
						draw_loading_screen();
						text_manager.write(font,"Generating new game...",1.7f,20, 20,0,0,false,1,1,1,1);
						game_state=7;
						return;
						break;
					//show load game menu
					case 1:
						read_saves();
						submenu=1;
						break;
					//show save game menu
					case 2:
						read_saves();
						submenu=2;
						break;
					//show options menu
					case 3:
						submenu=3;
						break;
					//exit
					case 4:
						quit_game=true;
						return;
						break;
					//load game
					case 5:
						//load_game(menu_system[submenu].item[a].effect_parameter);
						draw_loading_screen();
						text_manager.write(font,"Loading game...",1.7f,20, 20,0,0,false,1,1,1,1);
						game_state=6;
						load_slot=menu_system[submenu].item[a].effect_parameter;
						return;
						break;
					//save game
					case 6:
						//choose slot
						if(menu_system[submenu].item[a].effect_parameter!=proposed_save_game){
							save_game_name=menu_system[submenu].item[a].text.substr(1,menu_system[submenu].item[a].text.length());
							if(save_game_name=="-Unused-")
								save_game_name="";
							proposed_save_game=menu_system[submenu].item[a].effect_parameter;
						}
						//actually save the game
						else{
							save_game(menu_system[submenu].item[a].effect_parameter);
							read_saves();
							proposed_save_game=-1;
						}
						//game_state=0;
						break;
					//back to main menu
					case 7:
						submenu=0;
						break;
					//return to game
					case 8:
						game_state=0;
						play_music_file(-1,&last_played_music);
						break;
					//change control type
					case 9:
						player_control_type++;
						if(player_control_type>1)
							player_control_type=0;
						create_menu_items();
						break;
					//change mouse speed
					case 10:
						break;
					//show new game menu
					case 11:
						//select the last played mod if it still is there
						selected_mod=-1;
						for(b=0;b<mods;b++){
							if(mod_names[b]==last_played_mod){
								selected_mod=b;
								break;
							}
						}

						//last played mod not found, select default
						if(selected_mod==-1){
							for(b=0;b<mods;b++)
								if(mod_names[b]=="Default"){
									selected_mod=b;
									break;
								}
						}

						//still not found, select the first
						if(selected_mod==-1){
							selected_mod=0;
						}

						//map_size_setting=1;
						game_difficulty_level=1;
						proposed_player_race=0;
						create_menu_items();
						submenu=4;
						break;
					//change map size
					case 12:
						/*map_size_setting++;
						if(map_size_setting>2)
							map_size_setting=0;
						create_menu_items();*/
						break;
					//change game difficulty
					case 13:
						game_difficulty_level++;
						if(game_difficulty_level>2)
							game_difficulty_level=0;
						create_menu_items();
						break;
					//change item list sorting
					case 14:
						arrange_alphabetically=!arrange_alphabetically;
						if(game_running)arrange_item_list(false);
						create_menu_items();
						break;
					//change sound volume
					case 15:
						break;
					//change music volume
					case 16:
						break;
					//change puzzle play
					case 17:
					/*	play_puzzle+=1;
						if(play_puzzle>=3)
							play_puzzle=0;
						create_menu_items();*/
						break;
					//change player race
					case 18:
						proposed_player_race++;
						if(proposed_player_race>=race_names[selected_mod].names.size())
							proposed_player_race=0;
						while(!race_names[selected_mod].visible[proposed_player_race]){
							proposed_player_race++;
							if(proposed_player_race>=race_names[selected_mod].names.size())
								proposed_player_race=0;
						}
						create_menu_items();
						break;
					//change selected mod
					case 19:
						selected_mod++;
						if(selected_mod>=mods)
							selected_mod=0;
						//tempstring=mod_names[selected_mod];
						if(debug.debug_state[0]==0){
							if(mod_names[selected_mod]=="Tutorial"){
								selected_mod++;
								if(selected_mod>=mods)
									selected_mod=0;
							}
						}
						last_played_mod=mod_names[selected_mod];
						proposed_player_race=0;
						create_menu_items();
						break;
					//play tutorial
					case 20:
						for(b=0;b<mods;b++)
							if(mod_names[b]=="Tutorial"){
								selected_mod=b;
								break;
							}
						//map_size_setting=1;
						game_difficulty_level=1;
						proposed_player_race=0;
						draw_loading_screen();
						text_manager.write(font,"Loading tutorial...",1.7f,20, 20,0,0,false,1,1,1,1);
						game_state=7;
						return;
						break;
					//credits
					case 21:
						credits_texture=resources.load_texture("moss_back.jpg","");
						game_state=8;
						return;
						break;
					//map editor
					case 22:
						start_map_editor();
						return;
						break;
					//change journal showing
					case 23:
						show_journals=!show_journals;
						create_menu_items();
						break;
					//load last saved game
					case 24:
						draw_loading_screen();
						text_manager.write(font,"Loading game...",1.7f,20, 20,0,0,false,1,1,1,1);
						game_state=6;
						load_slot=last_saved_game;
						return;
						break;





				}

			}

			if(menu_system[submenu].item[a].effect!=-1){
				if(mouse_left){
					grim->Quads_SetColor(0.6f,0.6f,0.6f,1);
					//drag slider
					if(menu_system[submenu].item[a].text=="slider"){
						menu_system[submenu].item[a].effect_parameter=(mousex*1024.0f/screen_width-menu_system[submenu].start_x)/(menu_system[submenu].end_x-menu_system[submenu].start_x);

						switch(menu_system[submenu].item[a].effect){
							case 10:
								mouse_speed=menu_system[submenu].item[a].effect_parameter+0.5f;
								break;
							case 15:
								volume_slider[0]=menu_system[submenu].item[a].effect_parameter;
								break;
							case 16:
								volume_slider[1]=menu_system[submenu].item[a].effect_parameter;
								if(play_music)
									set_volume(volume_slider[1]);
								break;
						}

					}
				}

				else grim->Quads_SetColor(1,1,1,1);
			}
		}


		//draw slider
		if(menu_system[submenu].item[a].text=="slider"){
			resources.Texture_Set(bar_texture);
			grim->Quads_Begin();
				//left side
				grim->Quads_SetSubset((0/16.0f),(0/16.0f),(7/16.0f),(16/16.0f));
				grim->Quads_Draw((menu_system[submenu].start_x-7)/1024.0f*screen_width, (menu_system[submenu].start_y+position)/768.0f*screen_height, 7/1024.0f*screen_width, 16/768.0f*screen_height);
				//middle part
				grim->Quads_SetSubset((7/16.0f),(0/16.0f),(9/16.0f),(16/16.0f));
				grim->Quads_Draw(menu_system[submenu].start_x/1024.0f*screen_width, (menu_system[submenu].start_y+position)/768.0f*screen_height, (menu_system[submenu].end_x-menu_system[submenu].start_x)/1024.0f*screen_width, 16/768.0f*screen_height);
				//right side
				grim->Quads_SetSubset((9/16.0f),(0/16.0f),(16/16.0f),(16/16.0f));
				grim->Quads_Draw(menu_system[submenu].end_x/1024.0f*screen_width, (menu_system[submenu].start_y+position)/768.0f*screen_height, 7/1024.0f*screen_width, 16/768.0f*screen_height);

				//slider point
				grim->Quads_SetColor(1,1,1,1);
				grim->Quads_SetSubset((0/16.0f),(0/16.0f),(16/16.0f),(16/16.0f));
				grim->Quads_Draw((menu_system[submenu].start_x-7+menu_system[submenu].item[a].effect_parameter*(menu_system[submenu].end_x-menu_system[submenu].start_x))/1024.0f*screen_width, (menu_system[submenu].start_y+position)/768.0f*screen_height, 16/1024.0f*screen_width, 16/768.0f*screen_height);

			grim->Quads_End();

		}
		//draw text
		else{
			string text=menu_system[submenu].item[a].text;
			if(replace_text!="NULL")text=replace_text;
			if(cursor)text+=":";
			text_manager.write_line(font,menu_system[submenu].start_x/1024.0f*screen_width,(menu_system[submenu].start_y+position)/768.0f*screen_height,text,menu_system[submenu].item[a].text_size/1024.0f*screen_width);
		}

		position+=menu_system[submenu].item[a].height;

	}



	draw_mouse(0,0,0,1,1,1);
}

void game_engine::read_saves(void){

	proposed_save_game=-1;

	int		i;
	char	name[256];
	char	time[256];
	FILE	*f;


	int d=0;
	menu_system[1].item[d].text="LOAD GAME";
	menu_system[1].item[d].help=" ";
	menu_system[1].item[d].effect=-1;
	menu_system[1].item[d].text_size=1.5f;
	menu_system[1].item[d].height=25;
	menu_system[2].item[d].text="SAVE GAME";
	menu_system[2].item[d].help=" ";
	menu_system[2].item[d].effect=-1;
	menu_system[2].item[d].text_size=1.5f;
	menu_system[2].item[d].height=25;
	/*d++;
	menu_system[1].item[d].text=" ";
	menu_system[1].item[d].help=" ";
	menu_system[1].item[d].effect=-1;
	menu_system[1].item[d].text_size=1.5f;
	menu_system[1].item[d].height=25;
	menu_system[2].item[d].text=" ";
	menu_system[2].item[d].help=" ";
	menu_system[2].item[d].effect=-1;
	menu_system[2].item[d].text_size=1.5f;
	menu_system[2].item[d].height=25;*/

	d++;
	for (i=0 ; i<10 ; i++)
	{

		menu_system[1].item[i+d].text_size=1.4f;
		menu_system[1].item[i+d].height=25;
		menu_system[1].item[i+d].help=" ";
		menu_system[1].item[i+d].effect=5;
		menu_system[1].item[i+d].effect_parameter=i;

		menu_system[2].item[i+d].text_size=1.4f;
		menu_system[2].item[i+d].height=25;
		menu_system[2].item[i+d].help=" ";
		menu_system[2].item[i+d].effect=6;
		menu_system[2].item[i+d].effect_parameter=i;

		sprintf (name, "save/s%i.sav", i);
		f = fopen (name, "r");
		//file not found
		if (!f){
			menu_system[1].item[i+d].text= " -Unused-";
			menu_system[1].item[i+d].effect=-1;
			menu_system[2].item[i+d].text= " -Unused-";
			continue;
		}
		//load game time
		stripped_fgets(time,sizeof(time),f);
		stripped_fgets(name,sizeof(name),f);
		menu_system[1].item[i+d].text=" ";
		menu_system[2].item[i+d].text=" ";
		menu_system[1].item[i+d].text+=name;
		menu_system[2].item[i+d].text+=name;
		menu_system[1].item[i+d].help=time;
		menu_system[2].item[i+d].help=time;
		fclose (f);


	}
	for (i=1 ; i<3 ; i++){

		menu_system[i].item[d].text=" Quicksave";

		/*menu_system[i].item[20+d].text=" ";
		menu_system[i].item[20+d].help=" ";
		menu_system[i].item[20+d].effect=-1;
		menu_system[i].item[20+d].text_size=1.5f;
		menu_system[i].item[20+d].height=25;*/
		menu_system[i].item[10+d].text="Back";
		menu_system[i].item[10+d].help=" ";
		menu_system[i].item[10+d].effect=7;
		menu_system[i].item[10+d].text_size=1.5f;
		menu_system[i].item[10+d].height=25;

		menu_system[i].items=11+d;
		//menu_system[i].text_size=1.5f;
		//menu_system[i].item_height=25;
		menu_system[i].start_x=120;
		menu_system[i].end_x=370;
		menu_system[i].start_y=225;
	}
}


void game_engine::create_menu_items(void){


	//main menu
	{
		int d=0;
		if(game_running){
			menu_system[0].item[d].text="Return";
			menu_system[0].item[d].effect=8;
			menu_system[0].item[d].text_size=2.5f;
			menu_system[0].item[d].height=50.0f;
			d++;
		}
		//continue last saved game
		else{
			if(last_saved_game>=0){
				//see if the slot exists
				char	name[256];
				FILE	*f;
				sprintf (name, "save/s%i.sav", last_saved_game);
				f = fopen (name, "r");
				//file found
				if (f){
					menu_system[0].item[d].text="Continue";
					menu_system[0].item[d].help="Load last saved game.";
					menu_system[0].item[d].effect=24;
					menu_system[0].item[d].text_size=2.5f;
					menu_system[0].item[d].height=50.0f;
					d++;
					fclose(f);
				}
			}
		}
		menu_system[0].item[d].text="New Game";
		menu_system[0].item[d].help=" ";
		menu_system[0].item[d].effect=11;
		menu_system[0].item[d].text_size=2.5f;
		menu_system[0].item[d].height=50.0f;
		d++;
		menu_system[0].item[d].text="Tutorial";
		menu_system[0].item[d].help=" ";
		menu_system[0].item[d].effect=20;
		menu_system[0].item[d].text_size=2.5f;
		menu_system[0].item[d].height=50.0f;
		d++;
		menu_system[0].item[d].text="Load Game";
		menu_system[0].item[d].help=" ";
		menu_system[0].item[d].effect=1;
		menu_system[0].item[d].text_size=2.5f;
		menu_system[0].item[d].height=50.0f;
		d++;
		if(game_running){
			menu_system[0].item[d].text="Save Game";
			menu_system[0].item[d].help=" ";
			menu_system[0].item[d].effect=2;
			menu_system[0].item[d].text_size=2.5f;
			menu_system[0].item[d].height=50.0f;
			d++;
		}
		menu_system[0].item[d].text="Options";
		menu_system[0].item[d].help=" ";
		menu_system[0].item[d].effect=3;
		menu_system[0].item[d].text_size=2.5f;
		menu_system[0].item[d].height=50.0f;
		if(!game_running){
			d++;
			menu_system[0].item[d].text="Credits";
			menu_system[0].item[d].help=" ";
			menu_system[0].item[d].effect=21;
			menu_system[0].item[d].text_size=2.5f;
			menu_system[0].item[d].height=50.0f;
		}
		d++;
		menu_system[0].item[d].text="Exit";
		menu_system[0].item[d].help=" ";
		menu_system[0].item[d].effect=4;
		menu_system[0].item[d].text_size=2.5f;
		menu_system[0].item[d].height=50.0f;

		menu_system[0].items=d+1;
		menu_system[0].start_x=120;
		menu_system[0].end_x=370;
		menu_system[0].start_y=213;


		/*//new game button is return to game button when a game is going on
		if(game_running){
			menu_system[0].item[0].text,"Return";
			menu_system[0].item[0].effect=8;

			menu_system[0].item[2].text,"Save Game";
			menu_system[0].item[2].effect=2;
		}
		else{
			menu_system[0].item[0].text,"New Game";
			menu_system[0].item[0].effect=11;

			menu_system[0].item[2].text," ";
			menu_system[0].item[2].effect=-1;
		}*/
	}

	//options menu
	{
		int d=0;
		//control type
		menu_system[3].item[d].text="Control Type:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		if(player_control_type==0){
			menu_system[3].item[d].text="  Absolute";
			menu_system[3].item[d].help="W,A,S and D keys move player to absolute map directions. \\ \\ Aim and fire with mouse.";
		}
		if(player_control_type==1){
			menu_system[3].item[d].text="  Relative";
			menu_system[3].item[d].help="W,A,S and D keys move player to directions relative to player facing. \\ \\ Aim and fire with mouse.";
		}
		menu_system[3].item[d].effect=9;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;

	/*	//play puzzles
		d++;
		menu_system[3].item[d].text="Combine Puzzles:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		d++;
		if(play_puzzle==2){
			menu_system[3].item[d].text="  Full Puzzles";
			menu_system[3].item[d].help="Play a (possibly difficult) puzzle when combining items.";
		}
		if(play_puzzle==1){
			menu_system[3].item[d].text="  Easy Puzzles";
			menu_system[3].item[d].help="Play an easy puzzle when combining items.";
		}
		if(play_puzzle==0){
			menu_system[3].item[d].text="  Inactive";
			menu_system[3].item[d].help="No puzzles when combining items.";
		}
		menu_system[3].item[d].effect=17;
		*/

		//item list sort
		d++;
		menu_system[3].item[d].text="Sort Inventory:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		if(!arrange_alphabetically){
			menu_system[3].item[d].text="  By time";
			menu_system[3].item[d].help="Sorts the list of items by the time you got the item.";
		}
		if(arrange_alphabetically){
			menu_system[3].item[d].text="  Alphabetically";
			menu_system[3].item[d].help="Sorts the list of items by alphabet.";
		}
		menu_system[3].item[d].effect=14;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;

		//show journals
		d++;
		menu_system[3].item[d].text="Show journal:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		if(!show_journals){
			menu_system[3].item[d].text="  No";
			menu_system[3].item[d].help="The journal doesn't pop up when something is recorded in it.";
		}
		if(show_journals){
			menu_system[3].item[d].text="  Yes";
			menu_system[3].item[d].help="The journal is shown to you when something is recorded in it.";
		}
		menu_system[3].item[d].effect=23;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;

		//mouse speed
		d++;
		menu_system[3].item[d].text="Mouse Speed:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		menu_system[3].item[d].text="slider";
		menu_system[3].item[d].help="Drag slider to adjust.";
		menu_system[3].item[d].effect=10;
		menu_system[3].item[d].effect_parameter=mouse_speed-0.5f;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;


		//sound volume
		d++;
		menu_system[3].item[d].text="Sound Volume:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		menu_system[3].item[d].text="slider";
		menu_system[3].item[d].help="Drag slider to adjust. The absolute minimum will turn the sound off.";
		menu_system[3].item[d].effect=15;
		menu_system[3].item[d].effect_parameter=volume_slider[0];
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;

		//music volume
		d++;
		menu_system[3].item[d].text="Music Volume:";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		menu_system[3].item[d].text="slider";
		menu_system[3].item[d].help="Drag slider to adjust. The absolute minimum will turn the music off.";
		menu_system[3].item[d].effect=16;
		menu_system[3].item[d].effect_parameter=volume_slider[1];
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;

		//back
		d++;
		menu_system[3].item[d].text=" ";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=-1;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;
		d++;
		menu_system[3].item[d].text="Back";
		menu_system[3].item[d].help=" ";
		menu_system[3].item[d].effect=7;
		menu_system[3].item[d].text_size=1.5f;
		menu_system[3].item[d].height=25;

		menu_system[3].items=d+1;
		menu_system[3].start_x=120;
		menu_system[3].end_x=370;
		menu_system[3].start_y=213;
	}


	//new game
	{
		int d=0;
		menu_system[4].item[d].text="Start Game";
		menu_system[4].item[d].help="Start a new game with the shown settings.";
		menu_system[4].item[d].effect=0;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;

		if(debug.debug_state[0]==1){
			d++;
			menu_system[4].item[d].text="Start Editor";
			menu_system[4].item[d].help="Starts the map editor";
			menu_system[4].item[d].effect=22;
			menu_system[4].item[d].text_size=1.5f;
			menu_system[4].item[d].height=25;
		}

		d++;
		menu_system[4].item[d].text=" ";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=-1;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;

		d++;
		menu_system[4].item[d].text="Mod:";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=-1;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;
		d++;
		tempstring="  ";
		tempstring+=mod_names[selected_mod];
		menu_system[4].item[d].text=tempstring;
		menu_system[4].item[d].help="Select the mod to play. You can find mods on the Notrium homepage, or you can make your own.";
		menu_system[4].item[d].effect=19;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;


		/*d++;
		menu_system[4].item[d].text="Map Size:";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=-1;
		d++;
		if(map_size_setting==0){
			menu_system[4].item[d].text="  Small";
			menu_system[4].item[d].help="Items and creatures are closer together on a small map.";
		}
		if(map_size_setting==1){
			menu_system[4].item[d].text="  Average";
			menu_system[4].item[d].help="An average sized map is a good choise for a beginner.";
		}
		if(map_size_setting==2){
			menu_system[4].item[d].text="  Large";
			menu_system[4].item[d].help="Large maps have greater distances between items and creatures.";
		}
		menu_system[4].item[d].effect=12;*/

		d++;
		menu_system[4].item[d].text="Game difficulty:";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=-1;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;
		d++;
		if(game_difficulty_level==0){
			menu_system[4].item[d].text="  Easy";
			//menu_system[4].item[d].help="No body temperature. Lesser food and battery usage. Best for beginners.";
		}
		if(game_difficulty_level==1){
			menu_system[4].item[d].text="  Medium";
			//menu_system[4].item[d].help="Medium difficulty is good for most players, but expect to restart a few times if you've never played before.";
		}
		if(game_difficulty_level==2){
			menu_system[4].item[d].text="  Hard";
			//menu_system[4].item[d].help="Higher food and battery usage. For those who love the challenge.";
		}
		menu_system[4].item[d].help=race_names[selected_mod].difficulty_level_descriptions[game_difficulty_level];
		menu_system[4].item[d].effect=13;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;

		d++;
		menu_system[4].item[d].text="Player Race:";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=-1;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;

		d++;
		menu_system[4].item[d].text="  "+race_names[selected_mod].names[proposed_player_race];
		menu_system[4].item[d].help=race_names[selected_mod].description[proposed_player_race];
		menu_system[4].item[d].effect=18;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;



		//back
		d++;
		menu_system[4].item[d].text=" ";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=-1;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;
		d++;
		menu_system[4].item[d].text="Back";
		menu_system[4].item[d].help=" ";
		menu_system[4].item[d].effect=7;
		menu_system[4].item[d].text_size=1.5f;
		menu_system[4].item[d].height=25;

		menu_system[4].items=d+1;
		menu_system[4].start_x=120;
		menu_system[4].end_x=370;
		menu_system[4].start_y=213;
	}

}



void game_engine::calculate_body_temperature(void)
//change body temperature
{
	//planet temperature
	float heat_multiplier=0.9f*mod.general_races[player_race].temperature_multiplier;
	//if(game_difficulty_level==0)
	//	body_temperature=0;
	/*if(game_difficulty_level==2)
		heat_multiplier=0.7f;*/
	float base_temperature=(sincos.table_sin(day_timer/mod.general_races[player_race].day_speed*pi*2))*1.2f;


	//climate specific night and day temperatures
	if(base_temperature<0)
		base_temperature=-base_temperature*mod.general_climates[map_main->climate_number].temperature[0];
	if(base_temperature>0)
		base_temperature=base_temperature*mod.general_climates[map_main->climate_number].temperature[1];



	//player is in shade, reduces temperature for day
	if(in_shade&&(base_temperature>0))
		base_temperature=0;

	//player is near a fire, increase temperature for all times of day
	/*if(near_fire)
		if(base_temperature<0)
			base_temperature=0;
		else
			base_temperature+=1;*/

	if(map_main->creature[0].up){
		base_temperature+=0.2f;
	}


	//change body temperature
	body_temperature+=(base_temperature-body_temperature)*elapsed*game_speed*0.00005f*heat_multiplier;

	//terrain
	/*{
		//walking in hot/cold terrain
		float parameter0,parameter1;
		if(has_terrain_effect(map_main,map_main->grid[(int)(player_middle_x/grid_size)].grid[(int)(player_middle_y/grid_size)].terrain_type,0,&parameter0,&parameter1)){
			body_temperature+=elapsed*game_speed*0.0005f*parameter0*heat_multiplier;
		}
		//walking in damaging terrain
		if(has_terrain_effect(map_main,map_main->grid[(int)(player_middle_x/grid_size)].grid[(int)(player_middle_y/grid_size)].terrain_type,1,&parameter0,&parameter1)){
			map_main->creature[0].bars[(int)parameter1].value+=elapsed*game_speed*0.002f*parameter0;
		}
		//message
		if(mod.general_climates[map_main->climate_number].terrain_types[map_main->grid[(int)(player_middle_x/grid_size)].grid[(int)(player_middle_y/grid_size)].terrain_type].walk_on_text!="none"){
			text_manager.message(1000,1000,mod.general_climates[map_main->climate_number].terrain_types[map_main->grid[(int)(player_middle_x/grid_size)].grid[(int)(player_middle_y/grid_size)].terrain_type].walk_on_text);
		}
	}*/


	//by default player is not in shade or near fire
	in_shade=false;

}


void game_engine::sound_init(void){
		sound_initialized=true;

		//if( FAILED( hr = g_pSoundManager->Initialize( hWnd, DSSCL_PRIORITY, 2, 44100, 16 ) ) )
		if( ! g_pSoundManager->Initialize( 44100, 2 ) )
		{
			sound_initialized=false;
		}

		//initialize sound loader
		resources.initialize_resource_handler(grim,&debug,g_pSoundManager,play_sound, sound_initialized);

		//load extra samples
		UI_menu_click[0]=resources.load_sample("click0.wav",2,mod.mod_name);
		UI_menu_click[1]=resources.load_sample("click1.wav",2,mod.mod_name);
		UI_menu_click[2]=resources.load_sample("click2.wav",2,mod.mod_name);
		intro_sound=resources.load_sample("intro.wav",2,mod.mod_name);
}



void game_engine::playsound(int sample_number,float volume,float sound_x,float sound_y,float listener_x,float listener_y){//plays a sound

	if(!play_sound)return;
	if(!sound_initialized)return;
	if(sample_number>=resources.samples_loaded)return;
	if(sample_number<0)return;

	if(volume<=0)
		return;


	float fLog = log(1+volume*9)/log(10.0f);

	float distance=sqrtf(sqrtf(sqr(sound_x-listener_x)+sqr(sound_y-listener_y)))*13;
	float end_volume=((600.0f-distance)/600.0f)*1.6f*randDouble(0.95f,1.05f)*(float)volume_slider[0]*fLog;
	if(end_volume<0)end_volume=0;
	if(end_volume>1)end_volume=1;

	float pan=((sound_x-listener_x))/600.0f;
	if(pan<-1)pan=-1;
	if(pan>1)pan=1;

	if(end_volume<0.03f)return;

	resources.sample[sample_number]->Play( end_volume,pan );

}

/*
int game_engine::load_sample(string name, int samples){//loads the sample if it's unique

	if(name=="none")return -1;

	if(!play_sound)return -1;
	if(!sound_initialized)return -1;
	if(samples_loaded>=maximum_samples)return -1;

	int i;
	char temprivi[300];
	//find if the sample was already loaded
	for(i=0;i<samples_loaded;i++){
		//if(strcmpi(name,sample_name[i])==0){return i;}
		if(sample_name[i]==name){return i;}
	}

	//not loaded, load it
	{

		tempstring="Load Sample ";
		tempstring+=name;
		debug.debug_output(tempstring,1);

		if(samples==-1)samples=2;

		HRESULT hr;
		//try mod directory
		strcpy(temprivi,"sound/");
		strcat(temprivi,mod_name.c_str());
		strcat(temprivi,"/");
		strcat(temprivi,name.c_str());
		hr=g_pSoundManager->Create( &sample[samples_loaded], temprivi, DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN , GUID_NULL,samples );



		//try default directory
		if(hr!=S_OK){
			SAFE_DELETE(sample[samples_loaded]);
			strcpy(temprivi,"sound/");
			strcat(temprivi,name.c_str());
			hr=g_pSoundManager->Create( &sample[samples_loaded], temprivi, DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN , GUID_NULL,samples );
		}

		if(hr==S_OK){
			//store name
			sample_name[samples_loaded]=name;

			debug.debug_output(tempstring,0);

			samples_loaded++;
			return samples_loaded-1;
		}

		debug.debug_output("Loading Sample",2);
		return -1;
	}

}*/

void game_engine::draw_loading_screen(void){
	//loading screen
			grim->System_SetState_Blending(false);
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
			grim->Quads_SetRotation(0);
			resources.Texture_Set(presents_texture);
			grim->Quads_SetColor(1,1,1,1);
			grim->Quads_SetSubset(0,0,1,1);

			grim->Quads_Begin();
				grim->Quads_Draw(0, 0, screen_width,screen_height);
			grim->Quads_End();

}


void game_engine::play_music_file(int song_number, int *do_not_play)
{
	if(!play_music)
		return;
    // retrieve the filename of the EXE file
    /*string ModuleFileName;
    ModuleFileName.reserve(MAX_PATH);
    GetModuleFileName(
        NULL, const_cast<char*>(ModuleFileName.data()), MAX_PATH);
    // extract the path info from the filename
    string FileName = ModuleFileName.substr(0, ModuleFileName.find_last_of(":\\"));
    // append the sub-folder path
    FileName += folder;
	FileName += "/\*.mp3";


    WIN32_FIND_DATA ffd;
    HANDLE h;


	int music_files=0;
	//vector<char*> filenames;
	//filenames.clear();

	string filenames[30];

	//find music files
	h = FindFirstFile(FileName.c_str(), &ffd);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			filenames[music_files]=string(ffd.cFileName);
			//filenames.push_back(ffd.cFileName);
			music_files++;
			if(music_files>=30)break;

		} while (FindNextFile(h, &ffd));

		FindClose(h);
	}

	//music found
	if(music_files>0){*/

	//user wants random music
	if((song_number<0)||(song_number>mod.music.size())){

		//list all available songs
		vector <int> available_songs;
		for(int a=0;a<mod.music.size();a++){
			if(mod.music[a].dead)continue;
			if(!mod.music[a].can_be_random)continue;
			available_songs.push_back(a);
		}

		if(available_songs.size()==0)return;

		song_number=available_songs[randInt(0,available_songs.size())];

		//make sure we don't play the same song again
		if(available_songs.size()>1){
			while(song_number==*do_not_play){
				song_number=available_songs[randInt(0,available_songs.size())];
			}
		}
	}
	*do_not_play=song_number;

	string play_file =mod.music[song_number].name;

	//first see if such a song exists in the mod directory
	strcpy(temprivi,"music/");
	strcat(temprivi,mod.mod_name.c_str());
	strcat(temprivi,"/");
	strcat(temprivi,play_file.c_str());
	if(grim->File_Exists(temprivi)){
		SwapSourceFilter(temprivi);
	}

	//no song there, try the default directory
	strcpy(temprivi,"music/");
	strcat(temprivi,play_file.c_str());
	if(grim->File_Exists(temprivi)){
		SwapSourceFilter(temprivi);
	}

}


bool game_engine::SwapSourceFilter(const char* file)
{
    g_pSoundManager->playMusic(file);
	return true;
}

//This seems to change the current music track
//HRESULT game_engine::SwapSourceFilter(const char* file)
//{
//	//volume
//	float volume=volume_slider[1];
//
//    HRESULT hr = S_OK;
//    IPin *pPin = NULL;
//    WCHAR wFileName[MAX_PATH];
//
//
//	WIN32_FIND_DATA ffd;
//    HANDLE h;
//	//find music files
//	h = FindFirstFile(file, &ffd);
//	if (h == INVALID_HANDLE_VALUE) {
//		{
//			return ERROR_FILE_NOT_FOUND;
//		}
//	}
//	FindClose(h);
//
//
//
//    #ifndef UNICODE
//        MultiByteToWideChar(CP_ACP, 0, file, -1, wFileName, MAX_PATH);
//    #else
//		lstrcpy(wFileName, file);
//    #endif
//
//
//
//
//    // OPTIMIZATION OPPORTUNITY
//	// This will open the file, which is expensive. To optimize, this
//    // should be done earlier, ideally as soon as we knew this was the
//    // next file to ensure that the file load doesn't add to the
//    // filter swapping time & cause a hiccup.
//    //
//    // Add the new source filter to the graph. (Graph can still be running)
//    hr = g_pGraphBuilder->AddSourceFilter(wFileName, wFileName, &g_pSourceNext);
//
//    // Get the first output pin of the new source filter. Audio sources
//    // typically have only one output pin, so for most audio cases finding
//    // any output pin is sufficient.
//    if (SUCCEEDED(hr)) {
//        hr = g_pSourceNext->FindPin(L"Output", &pPin);
//    }
//
//    // Stop the graph
//    if (SUCCEEDED(hr)) {
//        hr = g_pMediaControl->Stop();
//    }
//
//    // Break all connections on the filters. You can do this by adding
//    // and removing each filter in the graph
//    if (SUCCEEDED(hr)) {
//        IEnumFilters *pFilterEnum = NULL;
//        //IBaseFilter  *pFilterTemp = NULL;
//
//        if (SUCCEEDED(hr = g_pGraphBuilder->EnumFilters(&pFilterEnum))) {
//            int iFiltCount = 0;
//            int iPos = 0;
//
//            // Need to know how many filters. If we add/remove filters during the
//            // enumeration we'll invalidate the enumerator
//            while (S_OK == pFilterEnum->Skip(1)) {
//                iFiltCount++;
//            }
//
//            // Allocate space, then pull out all of the
//            IBaseFilter **ppFilters = reinterpret_cast<IBaseFilter **>
//                                      (_alloca(sizeof(IBaseFilter *) * iFiltCount));
//            pFilterEnum->Reset();
//
//            while (S_OK == pFilterEnum->Next(1, &(ppFilters[iPos++]), NULL));
//            RELEASE(pFilterEnum);
//
//            for (iPos = 0; iPos < iFiltCount; iPos++) {
//                g_pGraphBuilder->RemoveFilter(ppFilters[iPos]);
//				// Put the filter back, unless it is the old source
//				if (ppFilters[iPos] != g_pSourceCurrent) {
//					g_pGraphBuilder->AddFilter(ppFilters[iPos], NULL);
//                }
//				RELEASE(ppFilters[iPos]);
//            }
//        }
//    }
//
//    // We have the new ouput pin. Render it
//    if (SUCCEEDED(hr)) {
//        hr = g_pGraphBuilder->Render(pPin);
//        g_pSourceCurrent = g_pSourceNext;
//        g_pSourceNext = NULL;
//    }
//
//    RELEASE(pPin);
//    RELEASE(g_pSourceNext); // In case of errors
//
//    // Re-seek the graph to the beginning
//    if (SUCCEEDED(hr)) {
//        LONGLONG llPos = 0;
//        hr = g_pMediaSeeking->SetPositions(&llPos, AM_SEEKING_AbsolutePositioning,
//                                           &llPos, AM_SEEKING_NoPositioning);
//    }
//
//	//set volume
//	if(SUCCEEDED(hr)){
//		hr=set_volume(volume);
//	}
//
//    // Start the graph
//    if (SUCCEEDED(hr)) {
//        hr = g_pMediaControl->Run();
//    }
//
//    // Release the old source filter.
//    RELEASE(g_pSourceCurrent);
//    return S_OK;
//}

bool game_engine::set_volume(float volume){
    g_pSoundManager->setMusicVolume(volume);
	return true;
}

//HRESULT game_engine::set_volume(float volume)
//{
//
//
//    HRESULT hr=S_OK;
//
//
//    IBasicAudio *pBA=NULL;
//
//    if (!g_pGraphBuilder)
//        return S_OK;
//
//    float fLog = (float)log(1+volume*9)/log(10);
//    float lVol = long(fLog*10000.f)-10000;
//
//    hr =  g_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void **)&pBA);
//    if (FAILED(hr))
//        return S_OK;
//
//    // Set new volume
//    hr = pBA->put_Volume(lVol);
//    if (FAILED(hr))
//    {
//        //RetailOutput(TEXT("Failed in pBA->put_Volume!  hr=0x%x\r\n"), hr);
//    }
//
//    pBA->Release();
//    return hr;
//}

bool game_engine::GraphInit()
{
    return true;
}

//HRESULT game_engine::GraphInit(void)
//{
//    HRESULT hr;
//    // Initialize COM
//    if (FAILED (hr = CoInitialize(NULL)) )
//        return hr;
//
//    // Create DirectShow Graph
//    if (FAILED (hr = CoCreateInstance(CLSID_FilterGraph, NULL,
//                                      CLSCTX_INPROC, IID_IGraphBuilder,
//                                      reinterpret_cast<void **>(&g_pGraphBuilder))) )
//        return hr;
//
//    // Get the IMediaControl Interface
//    if (FAILED (hr = g_pGraphBuilder->QueryInterface(IID_IMediaControl,
//                                 reinterpret_cast<void **>(&g_pMediaControl))))
//        return hr;
//
//    // Get the IMediaControl Interface
//    if (FAILED (hr = g_pGraphBuilder->QueryInterface(IID_IMediaSeeking,
//                                 reinterpret_cast<void **>(&g_pMediaSeeking))))
//        return hr;
//
//    // Create the intial graph
//    /*if (FAILED (hr = SwapSourceFilter("music/menu.mp3")))
//        return hr;*/
//
//	// Set the owner window to receive event notices.
//    if (FAILED (hr = g_pGraphBuilder->QueryInterface(IID_IMediaEventEx, reinterpret_cast<void **>(&pEvent))))
//		return hr;
//    /*if (FAILED (hr = pEvent->SetNotifyWindow(reinterpret_cast<OAHWND>(hWnd), WM_GRAPHNOTIFY, 0)))
//		return hr;*/
//
//
//
//
//
//    return S_OK;
//}


bool game_engine::HandleGraphEvent()
{
    //TODO: start next file when finished?
    return true;
}

//HRESULT game_engine::HandleGraphEvent(void)
//{
//    LONG evCode, evParam1, evParam2;
//    HRESULT hr=S_OK;
//
//    while(SUCCEEDED(pEvent->GetEvent(&evCode, &evParam1, &evParam2, 0)))
//    {
//        // Spin through the events
//        hr = pEvent->FreeEventParams(evCode, evParam1, evParam2);
//
//        if(EC_COMPLETE == evCode)
//        {
//			play_music_file(-1,&last_played_music);
//
//        }
//    }
//    return hr;
//}


void game_engine::handle_map_changed(void){

	//map change requested
	if(change_map_to>=0){
		change_map(change_map_to,change_map_player_x,change_map_player_y,transfer_enemies_when_changing_map);
		change_map_to=-1;
		change_map_player_x=0;
		change_map_player_y=0;
	}
	//else check if player is near edges
	else{

		float size=mod.general_creatures[map_main->creature[player_controlled_creature].type].size*map_main->creature[player_controlled_creature].size*general_creature_size;
		player_middle_x=map_main->creature[player_controlled_creature].x+size*0.5f;
		player_middle_y=map_main->creature[player_controlled_creature].y+size*0.5f;

		if(player_middle_x<grid_size+10){
			int new_area=map_main->near_areas[0];
			change_map(new_area,0,0,true);
		}

		else if(player_middle_y<grid_size+10){
			int new_area=map_main->near_areas[2];
			change_map(new_area,-2,0,true);
		}

		else if(player_middle_x>=map_main->sizex*grid_size-grid_size*2-10){
			int new_area=map_main->near_areas[1];
			change_map(new_area,-1,0,true);
		}

		else if(player_middle_y>=map_main->sizey*grid_size-grid_size*2-10){
			int new_area=map_main->near_areas[3];
			change_map(new_area,-3,0,true);
		}
	}

}

bool game_engine::creature_will_collide(map *new_map, creature_base *creature){
	int a,b,c;
	float bound_circle=14;

	if(creature==NULL)return false;

	if(creature->x<0)return true;
	if(creature->y<0)return true;
	if(creature->x>(new_map->sizex-1)*grid_size)return true;
	if(creature->y>(new_map->sizey-1)*grid_size)return true;


	int alku_x=(int)(creature->x/grid_size-2);
	int alku_y=(int)(creature->y/grid_size-2);
	int loppu_x=alku_x+4;
	int loppu_y=alku_y+4;

	if(alku_x<0)alku_x=0;
	if(loppu_x>new_map->sizex-1)loppu_x=new_map->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>new_map->sizey-1)loppu_y=new_map->sizey-1;

	float creature_size=(bound_circle)*(creature->size*mod.general_creatures[creature->type].size);
	//float creature_x=creature->x+creature_size*0.5f/(bound_circle)*general_creature_size;
	//float creature_y=creature->y+creature_size*0.5f/(bound_circle)*general_creature_size;



	//collisions prevented in creature specialties
	bool prop_prevented=false;
	bool plot_object_prevented=false;
	for(b=0;b<mod.general_creatures[creature->type].specialties.size();b++){
		if(mod.general_creatures[creature->type].specialties[b].number==4){
			if(mod.general_creatures[creature->type].specialties[b].parameter3==1){
				prop_prevented=true;
			}
			if(mod.general_creatures[creature->type].specialties[b].parameter0==1){
				plot_object_prevented=true;
			}
		}
	}



	for(a=alku_x;a<loppu_x;a++){
		for(b=alku_y;b<loppu_y;b++){
			//props
			if(!prop_prevented)
			for(c=0;c<new_map->grid[a].grid[b].objects.size();c++){
				if(new_map->object[new_map->grid[a].grid[b].objects[c]].dead)continue;
				if(creature_collision_detection(creature,&new_map->object[new_map->grid[a].grid[b].objects[c]],false))
					return true;
			}
			//plot objects
			if(!plot_object_prevented)
			for(c=0;c<new_map->grid[a].grid[b].items.size();c++){
				if(new_map->items[new_map->grid[a].grid[b].items[c]].dead)continue;
				if(new_map->items[new_map->grid[a].grid[b].items[c]].base_type!=0)continue;
				if(creature_collision_detection(creature,&new_map->items[new_map->grid[a].grid[b].items[c]],false))
					return true;
			}
		}
	}

	return false;
}


bool game_engine::point_will_collide(map *new_map, float x, float y, bool only_ones_that_stop_bullets){
	int a,b,c;
	float bound_circle=14;


	if(x<0)return true;
	if(y<0)return true;
	if(x>(new_map->sizex-1)*grid_size)return true;
	if(y>(new_map->sizey-1)*grid_size)return true;


	int alku_x=(int)(x/grid_size-2);
	int alku_y=(int)(y/grid_size-2);
	int loppu_x=alku_x+4;
	int loppu_y=alku_y+4;

	if(alku_x<0)alku_x=0;
	if(loppu_x>new_map->sizex-1)loppu_x=new_map->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>new_map->sizey-1)loppu_y=new_map->sizey-1;

	//float creature_x=creature->x+creature_size*0.5f/(bound_circle)*general_creature_size;
	//float creature_y=creature->y+creature_size*0.5f/(bound_circle)*general_creature_size;

	for(a=alku_x;a<loppu_x;a++){
		for(b=alku_y;b<loppu_y;b++){
			//props
			for(c=0;c<new_map->grid[a].grid[b].objects.size();c++){
				if(new_map->object[new_map->grid[a].grid[b].objects[c]].dead)continue;
				if(only_ones_that_stop_bullets)
					if(!mod.general_objects[new_map->object[new_map->grid[a].grid[b].objects[c]].type].stops_bullets)continue;
				if(point_in_object(x,y,&new_map->object[new_map->grid[a].grid[b].objects[c]]))
				//if(creature_collision_detection(creature,&new_map->object[new_map->grid[a].grid[b].objects[c]],false))
					return true;
			}
			//plot objects
			for(c=0;c<new_map->grid[a].grid[b].items.size();c++){
				if(new_map->items[new_map->grid[a].grid[b].items[c]].dead)continue;
				if(new_map->items[new_map->grid[a].grid[b].items[c]].base_type!=0)continue;
				if(only_ones_that_stop_bullets)
					if(!mod.general_objects[new_map->items[new_map->grid[a].grid[b].items[c]].type].stops_bullets)continue;
				//if(creature_collision_detection(creature,&new_map->items[new_map->grid[a].grid[b].items[c]],false))
				if(point_in_object(x,y,&new_map->items[new_map->grid[a].grid[b].items[c]]))
					return true;
			}
		}
	}

	return false;
}


void game_engine::change_map(int move, float new_x, float new_y, bool move_enemies){

	int a;

	//area doesn't exist
	if((move<=-1)||(move>=map_storage.size())){
		text_manager.message(1000,1000,"Cannot enter, region impassable.");

		map_main->creature[player_controlled_creature].x=map_main->creature[player_controlled_creature].x2;
		map_main->creature[player_controlled_creature].y=map_main->creature[player_controlled_creature].y2;
		return;
	}

	//a good time to unload unneeded textures
	resources.unload_unneeded_textures(false);

	//store old area
	int old_map=player_area;

	float player_x=(map_main->creature[0].x-grid_size)/((map_main->sizex-3)*grid_size);
	float player_y=(map_main->creature[0].y-grid_size)/((map_main->sizey-3)*grid_size);
	if(player_x<0)player_x=0;
	if(player_x>1)player_x=1;
	if(player_y<0)player_y=0;
	if(player_y>1)player_y=1;


	//make all vanished creatures reappear
	for(a=0;a<map_storage[old_map]->creature.size();a++){
		if(map_storage[old_map]->creature[a].vanish_timer>0){
			map_storage[old_map]->creature[a].vanish_timer=0;
			map_storage[old_map]->creature[a].dead=false;
		}
	}

	//find new area
	player_area=move;

	/*if(player_area_x<0)
		player_area_x=areas_x-1;
	if(player_area_y<0)
		player_area_y=areas_y-1;
	if(player_area_x>=areas_x)
		player_area_x=0;
	if(player_area_y>=areas_y)
		player_area_y=0;*/

	//not moving to same old map
	if(old_map!=player_area){

		//change map
		map_main=map_storage[player_area];

		//delete old light from new map if there is one
		if(map_main->creature[0].carry_light>=0){
			//map_main->lights.erase(map_main->lights.begin() + map_main->creature[0].carry_light);
			//map_main->lights[map_main->creature[0].carry_light].dead=true;
			delete_light(map_main,map_main->creature[0].carry_light);
		}

		//carry light to new area
		int set_new_light=-1;
		if(map_storage[old_map]->creature[0].carry_light>=0){
			map_main->lights.push_back(map_storage[old_map]->lights[map_storage[old_map]->creature[0].carry_light]);
			delete_light(map_storage[old_map],map_storage[old_map]->creature[0].carry_light);
			set_new_light=map_main->lights.size()-1;
		}

		//carry creature
		int carry_creature=-1;
		if(map_storage[old_map]->creature[0].carried_creature>=0){
			map_main->creature.push_back(map_storage[old_map]->creature[map_storage[old_map]->creature[0].carried_creature]);
			carry_creature=map_main->creature.size()-1;
			delete_creature(map_storage[old_map],map_storage[old_map]->creature[0].carried_creature);
		}

		//copy player from old to new
		memcpy(&map_main->creature[0],
			&map_storage[old_map]->creature[0],
			sizeof(map_main->creature[0]));
		map_main->creature[0].carry_light=set_new_light;
		map_main->creature[0].carried_creature=carry_creature;
	}


	float size=mod.general_creatures[map_main->creature[0].type].size*map_main->creature[0].size*general_creature_size;

	//move player to right position
	if(new_x<=0){
		map_main->creature[0].x=player_x*(map_main->sizex-3)*grid_size+grid_size;
		map_main->creature[0].y=player_y*(map_main->sizey-3)*grid_size+grid_size;
		map_main->creature[0].x2=map_main->creature[0].x;
		map_main->creature[0].y2=map_main->creature[0].y;
		if(new_x==0){
			map_main->creature[0].x=map_main->sizex*grid_size-grid_size*2-20-size*0.5f;
			map_main->creature[0].x2=map_main->creature[0].x;
		}
		if(new_x==-2){
			map_main->creature[0].y=map_main->sizey*grid_size-grid_size*2-20-size*0.5f;
			map_main->creature[0].y2=map_main->creature[0].y;
		}
		if(new_x==-1){
			map_main->creature[0].x=grid_size+20-size*0.5f;
			map_main->creature[0].x2=map_main->creature[0].x;
		}
		if(new_x==-3){
			map_main->creature[0].y=grid_size+20-size*0.5f;
			map_main->creature[0].y2=map_main->creature[0].y;
		}
	}
	//preset position
	if(new_x>0){
		map_main->creature[0].x=new_x-size*0.5f;
		map_main->creature[0].y=new_y-size*0.5f;
		map_main->creature[0].x2=map_main->creature[0].x;
		map_main->creature[0].y2=map_main->creature[0].y;
	}

	//make sure player is on valid map
	int creature=0;

	if(map_main->creature[creature].x<grid_size+10-size*0.5f)
        map_main->creature[creature].x=grid_size+10-size*0.5f;
	if(map_main->creature[creature].y<grid_size+10-size*0.5f)
        map_main->creature[creature].y=grid_size+10-size*0.5f;
	if(map_main->creature[creature].x>map_main->sizex*grid_size-grid_size*2-10-size*0.5f)
        map_main->creature[creature].x=map_main->sizex*grid_size-grid_size*2-10-size*0.5f;
	if(map_main->creature[creature].y>map_main->sizey*grid_size-grid_size*2-10-size*0.5f)
        map_main->creature[creature].y=map_main->sizey*grid_size-grid_size*2-10-size*0.5f;

	//check collisions for player, we don't want to end up in a rock
	while(creature_will_collide(map_main,&map_main->creature[0])){
		creature=0;
		map_main->creature[creature].x+=randDouble(-50,50);
		map_main->creature[creature].y+=randDouble(-50,50);

		//make sure creature is on map again
		if(map_main->creature[creature].x<grid_size+10+size*0.5f)
            map_main->creature[creature].x=grid_size+10+size*0.5f;
		if(map_main->creature[creature].y<grid_size+10+size*0.5f)
            map_main->creature[creature].y=grid_size+10+size*0.5f;
		if(map_main->creature[creature].x>map_main->sizex*grid_size-grid_size*2-10+size*0.5f)
            map_main->creature[creature].x=map_main->sizex*grid_size-grid_size*2-10+size*0.5f;
		if(map_main->creature[creature].y>map_main->sizey*grid_size-grid_size*2-10+size*0.5f)
            map_main->creature[creature].y=map_main->sizey*grid_size-grid_size*2-10+size*0.5f;
	}

	//find new camera position
	attach_camera_time=-1;
	attach_camera_type=0;
	attach_camera_parameter1=0;
	attach_camera_parameter2=0;
	float suggested_camera_x,suggested_camera_y;
	find_suggested_camera_position(&suggested_camera_x,&suggested_camera_y);
	real_camera_x=suggested_camera_x;
	real_camera_y=suggested_camera_y;

	mousex=screen_width/2;
	mousey=screen_height/2;

	//if(move_enemies){
		int player_controlled_creature_was=player_controlled_creature;
		player_controlled_creature=0;
		//move all moving nearby creatures to other map
		for(a=1;a<map_storage[old_map]->creature.size();a++){
			if(!map_storage[old_map]->creature[a].dead){
					if(!map_storage[old_map]->creature[a].killed)
					//if(map_storage[old_map]->creature[a].AI_order!=1)//not commanded to stay
					if(mod.general_creatures[map_storage[old_map]->creature[a].type].can_change_area)
					//if(map_storage[old_map]->creature[a].may_change_area)
						if((mod.general_creatures[map_storage[old_map]->creature[a].type].movement_speed>0)
                                ||(map_storage[old_map]->creature[a].side==map_storage[old_map]->creature[0].side)){

							//if we're not supposed to move any other creatures, the only creature we move is the one carrying the player
							if(!move_enemies){
								bool OK=false;
								if(map_storage[old_map]->creature[a].carried_creature==0)
									OK=true;
								if(!OK)continue;
							}

							//not stunned
							bool OK=true;
							for(int b=0;b<map_storage[old_map]->creature[a].weapon_effects_amount;b++){
								if(map_storage[old_map]->creature[a].weapon_effects[b].effect_type==0)
								if(map_storage[old_map]->creature[a].weapon_effects[b].parameter0==0){
									OK=false;
									break;
								}
							}
							if(!OK)continue;

							//if not close enough
							//own side
							if(map_storage[old_map]->creature[a].side==map_storage[old_map]->creature[0].side)
								if(sqr(map_storage[old_map]->creature[a].x-map_storage[old_map]->creature[0].x)+sqr(map_storage[old_map]->creature[a].y-map_storage[old_map]->creature[0].y)>sqr(500))
									continue;
							//enemies
							if(map_storage[old_map]->creature[a].side!=map_storage[old_map]->creature[0].side)
								if(sqr(map_storage[old_map]->creature[a].x-map_storage[old_map]->creature[0].x)+sqr(map_storage[old_map]->creature[a].y-map_storage[old_map]->creature[0].y)>sqr(400))
									continue;

							//carry light
							int set_new_light=-1;
							if(map_storage[old_map]->creature[a].carry_light>=0){
								map_main->lights.push_back(map_storage[old_map]->lights[map_storage[old_map]->creature[a].carry_light]);
								set_new_light=map_main->lights.size()-1;
								delete_light(map_storage[old_map],map_storage[old_map]->creature[a].carry_light);
							}

							//carry creature
							int carry_creature=-1;
							if(map_storage[old_map]->creature[a].carried_creature==0){
								carry_creature=0;
							}
							if(map_storage[old_map]->creature[a].carried_creature>0){
								map_main->creature.push_back(map_storage[old_map]->creature[map_storage[old_map]->creature[a].carried_creature]);
								carry_creature=map_main->creature.size()-1;
								int temp=player_controlled_creature;
								delete_creature(map_storage[old_map],map_storage[old_map]->creature[a].carried_creature);
								player_controlled_creature=temp;

								if(a<player_controlled_creature_was)
									player_controlled_creature_was--;
							}

							//if not carrying player or on player's side, make the creature vanish for a time
							float distance=sqrtf(sqr(map_storage[old_map]->creature[a].x-map_storage[old_map]->creature[0].x)+sqr(map_storage[old_map]->creature[a].y-map_storage[old_map]->creature[0].y));
							if((map_storage[old_map]->creature[a].side!=map_storage[old_map]->creature[0].side)&&(map_storage[old_map]->creature[a].carried_creature!=0)){
								map_storage[old_map]->creature[a].vanish_timer=distance/mod.general_creatures[map_storage[old_map]->creature[a].type].movement_speed;
								map_storage[old_map]->creature[a].dead=true;
							}

							//put to new area
							int creature=a;
							if(old_map!=player_area){
								map_main->creature.push_back(map_storage[old_map]->creature[a]);
								creature=map_main->creature.size()-1;
								if(a==player_controlled_creature_was)
									player_controlled_creature=creature;
							}
							float player_size=mod.general_creatures[map_main->creature[0].type].size*map_main->creature[0].size*general_creature_size;
							float creature_size=mod.general_creatures[map_main->creature[creature].type].size*map_main->creature[creature].size*general_creature_size;

							map_main->creature[creature].x=map_main->creature[0].x+player_size*0.5f-creature_size*0.5f+randDouble(-20,20);
							map_main->creature[creature].y=map_main->creature[0].y+player_size*0.5f-creature_size*0.5f+randDouble(-20,20);
							map_main->creature[creature].carry_light=set_new_light;
							map_main->creature[creature].carried_creature=carry_creature;


							//check collisions for creature, we don't want to end up in a rock
							while(creature_will_collide(map_main,&map_main->creature[0])){
								map_main->creature[creature].x+=randDouble(-50,50);
								map_main->creature[creature].y+=randDouble(-50,50);
								//make sure creature is on map again
								if(map_main->creature[creature].x<1)
                                    map_main->creature[creature].x=1;
								if(map_main->creature[creature].y<1)
                                    map_main->creature[creature].y=1;
								if(map_main->creature[creature].x>(map_main->sizex-1)*(grid_size)-1)
                                    map_main->creature[creature].x=(map_main->sizex-1)*(grid_size)-1;
								if(map_main->creature[creature].y>(map_main->sizey-1)*(grid_size)-1)
								map_main->creature[creature].y=(map_main->sizey-1)*(grid_size)-1;
							}

							//remove from old area
							if(old_map!=player_area){
								int temp=player_controlled_creature;
								delete_creature(map_storage[old_map],a);
								//map_storage[old_map]->creature.erase(map_storage[old_map]->creature.begin() + a);
								a--;
								player_controlled_creature=temp;
								if(a<player_controlled_creature_was)
									player_controlled_creature_was--;
							}
						}
				//}
			}
		}
	//}

	//recalculate creature positions
	map_main->check_creatures();
	creatures_checked_on=time_from_beginning;
	creature_visibility_checked_on=time_from_beginning+randDouble(0,1);
	for(a=0;a<map_main->creature.size();a++){
		map_main->creature[a].wall_between_creature_and_player=-1;
	}

	//on_enter_text
	if(mod.general_areas[map_main->area_type].on_enter_text!="none")
	if(!map_main->been_here_already){
		map_main->been_here_already=true;

		record_message(2,map_main->area_type);
		if(show_journals)show_text_window(journal_records.size()-1);
	}

	//delete all particles
	for(a=0;a<particles.size();a++){
		if(particles[a].dead)continue;
		for(int b=0;b<3;b++){
			particles[a].particles_list[b].clear();
		}
	}

}

void game_engine::combine_items(int a,int combine_item, const vector<Mod::combines::combine_results_base>& combine_results, bool discard_this, bool discard_that){

	//unuse the combination items
	if(discard_this){
		inventory[active_inventory].player_items[a].amount--;
		if((inventory[active_inventory].player_items[a].amount==0)&&(inventory[active_inventory].player_items[a].wielded)){
			for(int c=0;c<mod.general_items[inventory[active_inventory].player_items[a].item].effects.size();c++){
				use_item(inventory[active_inventory].player_items[a].item,&a, mod.general_items[inventory[active_inventory].player_items[a].item].effects[c].effect,true,false, false,false);
			}
			delete_item(a);

			//one slot gets deleted, the other must be down by one
			if(a<combine_item)
				combine_item--;
		}
	}
	if(discard_that){
		inventory[active_inventory].player_items[combine_item].amount--;
		if((inventory[active_inventory].player_items[combine_item].amount==0)&&(inventory[active_inventory].player_items[combine_item].wielded)){
			for(int c=0;c<mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects.size();c++){
				use_item(inventory[active_inventory].player_items[combine_item].item,&combine_item, mod.general_items[inventory[active_inventory].player_items[combine_item].item].effects[c].effect,true,false, false,false);
			}
			delete_item(combine_item);
		}
	}



	//give result
	for(int b=0;b<combine_results.size();b++){
		give_item(combine_results[b].combines_to,combine_results[b].combines_amount,time_from_beginning,true);
	}

	//inform of success
	text_manager.message(3000,1000,"Combined!");


	arrange_item_list(false);
}



void game_engine::create_maps(void){

	debug.debug_output("Map Creation",1,0);

	int a,b,c,d;

	float map_size_multiplier=1.0f;
	/*//small map
	if(map_size_setting==0){
		map_size_multiplier=0.6f;
	}
	//large map
	if(map_size_setting==2){
		map_size_multiplier=1.5f;
	}*/

	//clear any old maps
	for(a=0;a<map_storage.size();a++){
		//first delete old minimaps)
		if(map_storage[a]->map_texture_2>=0){
			grim->Texture_Delete(map_storage[a]->map_texture);
		}
		if(map_storage[a]->map_texture_2>=0){
			grim->Texture_Delete(map_storage[a]->map_texture_2);
		}
		SAFE_DELETE(map_storage[a]);
	}
	map_storage.clear();

	//int maps=areas_x*areas_x;//power of 2

	/*//declare special areas
	int *special_areas=new int[maps];
	//default area = 0
	for(d=0;d<maps;d++){
		special_areas[d]=0;
	}
	for(a=0;a<available_mod.general_areas;a++){
		for(d=0;d<mod.general_areas[a].amount;d++){
			int place=randInt(0,maps);
			while(special_areas[place]!=0){
				place=randInt(0,maps);
			}
			special_areas[place]=a;
		}
	}*/

	//find total area of all areas
	float total_area=0;
	for(d=0;d<mod.general_areas.size();d++){
		if(mod.general_areas[d].dead)continue;
		int sizex=mod.terrain_maps[mod.general_areas[d].terrain_map_number].terrain_grid[0].terrain_blocks.size();
		int sizey=mod.terrain_maps[mod.general_areas[d].terrain_map_number].terrain_grid.size();
		total_area+=sizex*sizey;
	}

	//let's count how many items we might have for one map
	int items_max=0;
	for(a=0;a<mod.general_plot_objects.size();a++){
		items_max+=mod.general_plot_objects[a].amount;
		//there may be many of the same type
		//for(int b=0;b<mod.general_plot_objects[a].amount;b++){
			//if(mod.general_plot_objects[a].map_type_to_place>=0)
			//if(mod.general_plot_objects[a].map_type_to_place!=map_special)continue;
		//	items_max++;
		//}
	}

	//distribute items
	debug.debug_output("Distributing Items",1,0);
	int *items_pre=new int[mod.general_areas.size()*items_max];
	int *items_total= new int[mod.general_areas.size()];
	for(d=0;d<mod.general_areas.size();d++){items_total[d]=0;};
	//int item_counter=0;
	//plot objects
	for(a=0;a<mod.general_plot_objects.size();a++){
		if(mod.general_plot_objects[a].dead)continue;

		/*//not on this map type
		if(mod.general_plot_objects[a].map_type_to_place>=0)
		if(mod.general_plot_objects[a].map_type_to_place!=map_to_edit->special_types[0])continue;*/

		//there may be many of the same type
		for(int b=0;b<mod.general_plot_objects[a].amount;b++){
			bool area_unfit=true;
			int tries=0;
			int area_number=0;
			while(area_unfit&&(tries<1000)){
				tries++;
				area_number=randInt(0,mod.general_areas.size());
				if(mod.general_areas[area_number].dead)continue;

				//area type excludes this plot_object class
				bool cannot_continue=false;
				for(int i=0;i<mod.general_areas[area_number].exclude_plot_objects.size();i++){
					if(mod.general_areas[area_number].exclude_plot_objects[i]==mod.general_plot_objects[a].plot_object_class)
						cannot_continue=true;
				}
				if(cannot_continue)continue;

				int sizex=mod.terrain_maps[mod.general_areas[area_number].terrain_map_number].terrain_grid[0].terrain_blocks.size();
				int sizey=mod.terrain_maps[mod.general_areas[area_number].terrain_map_number].terrain_grid.size();

				//right on this map type
				if(mod.general_plot_objects[a].map_type_to_place>=0){
					if(area_number==mod.general_plot_objects[a].map_type_to_place)
						area_unfit=false;
					//if(mod.general_plot_objects[a].map_type_to_place!=special_areas[area_number])continue;
				}
				//randomly here
				else if(randDouble(0,1)<((sizex*sizey*mod.general_areas[area_number].random_item_density)/total_area)){
					area_unfit=false;
				}
			}
			//area found, add item to list
			if(!area_unfit){
				items_pre[area_number*items_max+items_total[area_number]]=a;
				items_total[area_number]++;
			}
		}
	}
	debug.debug_output("Distributing Items",0,0);

	//create maps
	for(d=0;d<mod.general_areas.size();d++){

		if(mod.general_areas[d].dead){
			continue;
		}



		int map_type=d;

		debug.debug_output("Initializing Area "+mod.general_areas[map_type].name,1,0);

		//find multiplier for prop numbers
		debug.debug_output("Find prop number multipliers",1,0);
		int sizex=mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].terrain_grid[0].terrain_blocks.size();
		int sizey=mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].terrain_grid.size();
		float amount_multiplier=(map_size_multiplier*map_size_multiplier*sizex*sizey)/(100*100);
		debug.debug_output("Find prop number multipliers",0,0);

		//select climate
		debug.debug_output("Select Climate",1,0);
		int climate=randInt(0,mod.general_climates.size());
		int tries=0;
		while(!mod.general_climates[climate].can_be_random||mod.general_climates[climate].dead){
			climate=randInt(0,mod.general_climates.size());
			tries++;
			if(tries>100){climate=0;break;}
		}
		if(mod.general_areas[map_type].climate_override!=-1)
			climate=mod.general_areas[map_type].climate_override;

		//override for map development
		if(climate_override!=-1)
			climate=climate_override;
		debug.debug_output("Select Climate",0,0);

		//create list of hazardous terrain maps
		debug.debug_output("List no_random_objects_terrain",1,0);
		vector <bool> terrain_hazardous;
		for(a=0;a<sizex;a++){
			for(b=0;b<sizey;b++){
				bool hazardous=false;
				if(mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].terrain_grid[b].terrain_blocks[a].no_random_items==1)
					hazardous=true;
				if(mod.terrain_types[mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].terrain_grid[b].terrain_blocks[a].terrain_type].do_not_place_random_objects)
					hazardous=true;
				terrain_hazardous.push_back(hazardous);

			}
		}
		//create list of terrains that don't want to be on the edges
		vector <bool> do_not_place_on_map_edges;
		for(a=0;a<mod.terrain_types.size();a++){
			do_not_place_on_map_edges.push_back(mod.terrain_types[a].do_not_place_on_map_edges);
		}
		//create list of terrain types that don't want random objects on them
		vector <bool> no_random_terrain_types;
		for(a=0;a<mod.terrain_types.size();a++){
			no_random_terrain_types.push_back(mod.terrain_types[a].do_not_place_random_objects);
		}
		debug.debug_output("List no_random_objects_terrain",0,0);

		debug.debug_output("Allocate memory for map",1,0);
		map *temp_map;
		//temp_map = new map((int)(mod.general_areas[map_type].sizex*map_size_multiplier),(int)(mod.general_areas[map_type].sizey*map_size_multiplier),amount_multiplier,climate,mod.general_climates[climate].terrain_texture,mod.general_climates[climate].terrain_type,mod.general_climates[climate].prop_amount,mod.general_climates[climate].prop_object_definition_number,mod.general_areas[map_type].alien_type,mod.general_areas[map_type].alien_amount,items_total[d]);
		map_storage.push_back(new map((int)(sizex*map_size_multiplier),(int)(sizey*map_size_multiplier),amount_multiplier,climate,mod.general_climates[climate].terrain_types,no_random_terrain_types,do_not_place_on_map_edges,terrain_hazardous,mod.general_climates[climate].prop_amount,mod.general_climates[climate].prop_object_definition_number,mod.general_areas[map_type].alien_type,mod.general_areas[map_type].alien_amount,mod.general_areas[map_type].alien_sides,items_total[d]));
		temp_map=map_storage[d];
		temp_map->area_type=map_type;
		debug.debug_output("Allocate memory for map",0,0);


		//player race
		temp_map->creature[0].type=mod.general_races[player_race].creature_number;

		//weather
		temp_map->wind_direction=randDouble(0,2*pi);
		temp_map->wind_direction_target=randDouble(0,2*pi);
		temp_map->wind_speed=randDouble(0,mod.general_climates[climate].maximum_wind_speed);
		temp_map->rain_timer=-1;


		//vary object sizes
		vary_object_sizes(temp_map);

		//drop map editor terrains
		debug.debug_output("Distribute Map Editor terrains",1,0);
		for(a=0;a<sizex;a++){
			for(b=0;b<sizey;b++){
				if(mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].terrain_grid[b].terrain_blocks[a].terrain_type>1){
					temp_map->grid[a].grid[b].terrain_type=mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].terrain_grid[b].terrain_blocks[a].terrain_type;
				}
			}
		}
		debug.debug_output("Distribute Map Editor terrains",0,0);





		//initialize creature parameters
		debug.debug_output("Initializing Creature Specialties",1,0);
		for(a=0;a<temp_map->creature.size();a++){

			//creature specialties
			initialize_creature_specialties(&temp_map->creature[a],temp_map,true);

			//timed creature scripts
			for(c=0;c<mod.general_creatures[temp_map->creature[a].type].timed_block.size();c++){
				Mod::general_creatures_base::timed_effect_block effect=mod.general_creatures[temp_map->creature[a].type].timed_block[c];

				temp_map->creature[a].script_timer[c]=time_from_beginning+effect.interval*0.001f*randDouble(0,1);
			}

			//initialize AI parameters
			temp_map->creature[a].tactic[0]=mod.general_creatures[temp_map->creature[a].type].AI_tactics[0];
			temp_map->creature[a].tactic[1]=mod.general_creatures[temp_map->creature[a].type].AI_tactics[1];
			AI_initiate_behavior_parameters(&temp_map->creature[a]);

		}
		debug.debug_output("Initializing Creature Specialties",0,0);





		//drop map editor objects
		debug.debug_output("Distribute Map Editor objects",1,0);
		for(a=0;a<mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].map_objects.size();a++){
			Mod::terrain_map_base::editor_object_base temp_object=mod.terrain_maps[mod.general_areas[map_type].terrain_map_number].map_objects[a];
			switch(temp_object.type){
				//items
				case 2:
					create_item(temp_map,temp_object.number,temp_object.amount,temp_object.x,temp_object.y,temp_object.rotation);
				break;
				//plot_objects
				case 3:
					create_plot_object(temp_map,temp_object.number,temp_object.x,temp_object.y,temp_object.rotation,temp_object.size*mod.general_objects[mod.general_plot_objects[temp_object.number].object_definition_number].base_size);
				break;
				//creatures
				case 4:
					{
						//see if this is a player_race creature
						bool is_player_race=false;
						for(b=0;b<mod.general_races.size();b++){
							if(mod.general_races[b].creature_number==temp_object.number){
								is_player_race=true;
								temp_map->creature[0].x=temp_object.x-0.5f*64*mod.general_creatures[temp_object.number].size;
								temp_map->creature[0].y=temp_object.y-0.5f*64*mod.general_creatures[temp_object.number].size;
								//temp_map->creature[0].side=temp_object.side;
								temp_map->creature[0].rotation=temp_object.rotation;
								temp_map->creature[0].rotation_legs=temp_object.rotation;
								temp_map->creature[0].rotation_head=temp_object.rotation;

							}
						}
						//it's not, create the creature
						if(!is_player_race)
							spawn_creature(temp_object.side,-1,-1,temp_object.x,temp_object.y,temp_object.rotation,temp_object.number,temp_map);
					}

				break;
				//lights
				case 5:
					{
					int light=temp_map->create_light(temp_object.x,temp_object.y,temp_object.number,temp_object.size,mod.general_lights[temp_object.number].r,mod.general_lights[temp_object.number].g,mod.general_lights[temp_object.number].b,mod.general_lights[temp_object.number].a,-1);
					temp_map->lights[light].rotation=temp_object.rotation;
					}

				break;
				//props
				case 6:
					map_object temp_prop;
					float size=temp_object.size*mod.general_objects[temp_object.number].base_size*general_object_size;
					temp_prop.x=temp_object.x-size*0.5f;
					temp_prop.y=temp_object.y-size*0.5f;
					temp_prop.dead=false;
					temp_prop.rotation=temp_object.rotation;
					temp_prop.type=temp_object.number;
					temp_prop.size=temp_object.size*mod.general_objects[temp_object.number].base_size;
					temp_prop.sway_phase=0;
					temp_prop.sway_power=0;
					temp_map->object.push_back(temp_prop);
				break;
			}
		}
		debug.debug_output("Distribute Map Editor objects",0,0);

		//initialize terrain frames
		debug.debug_output("Initialize animation frames",1,0);
		initialize_animation_frames(temp_map);
		debug.debug_output("Initialize animation frames",0,0);

		//initialize the map
		temp_map->initialize_objects();

		//check where the creatures are
		temp_map->check_creatures();



		//now that object infos are known, we can initialize items and plot objects
		debug.debug_output("Initializing Plot Objects and Items",1,0);
		temp_map->items.reserve(items_total[d]+10);
		for(a=0;a<items_total[d];a++){
			create_plot_object(temp_map,items_pre[d*items_max+a],-1,-1,-1,-1);
		}
		debug.debug_output("Initializing Plot Objects and Items",0,0);



		//run effects
		map_main=temp_map;
		for(a=0;a<temp_map->items.size();a++){

			if(temp_map->items[a].base_type!=0)continue;
			if(temp_map->items[a].event_used)continue;
			if(temp_map->items[a].dead)continue;


			if(mod.general_plot_objects[temp_map->items[a].item_type].trigger_event_by==3)
			for(c=0;c<mod.general_plot_objects[temp_map->items[a].item_type].effects.size();c++){


				for(b=0;b<mod.general_plot_objects[temp_map->items[a].item_type].effects[c].effects.size();b++){
					run_effect(mod.general_plot_objects[temp_map->items[a].item_type].effects[c].effects[b],&temp_map->creature[0],0,temp_map->items[a].x+temp_map->items[a].size*0.5f*general_object_size,temp_map->items[a].y+temp_map->items[a].size*0.5f*general_object_size,temp_map->items[a].rotation,false);
				}
				//delete item if needed
				if((mod.general_plot_objects[temp_map->items[a].item_type].effects[c].vanish_after_used==0)||(mod.general_plot_objects[temp_map->items[a].item_type].effects[c].vanish_after_used==1)){//cannot be reused
					temp_map->items[a].event_used=true;
				}
				if(mod.general_plot_objects[temp_map->items[a].item_type].effects[c].vanish_after_used==1){

					temp_map->items[a].dead=true;
					//temp_map->items.erase(temp_map->items.begin()+a);
					//break;
				}
			}
		}







		//initialize items
		temp_map->initialize_items();

		//map texture
		debug.debug_output("Creating minimap",1,0);
		temp_map->map_texture=-1;
		temp_map->map_texture_2=-1;
		create_minimap(temp_map,d);
		debug.debug_output("Creating minimap",0,0);


		debug.debug_output("Initializing Area "+mod.general_areas[map_type].name,0,0);

	}

	//set all edges to lead to right places
	set_edges();



	SAFE_DELETE_ARRAY(items_pre);
	SAFE_DELETE_ARRAY(items_total);


	debug.debug_output("Map Creation",0,0);
}

void game_engine::create_minimap(map *map_to_edit, int d){

	map_size_x=256;
	map_size_y=256;

	tempstring="map_texture "+d;
	//itoa(d,temprivi,10);
	//sprintf(temprivi,"%d",d);
	//tempstring+=temprivi;
	//texture one
	//strcpy(temprivi,tempstring.c_str());
	grim->Texture_Create(tempstring, map_size_x, map_size_y);
	map_to_edit->map_texture=grim->Texture_Get(tempstring);
	//texture two
	//strcat(temprivi,"_2");
	tempstring += "_2";
	grim->Texture_Create(tempstring, map_size_x, map_size_y);
	map_to_edit->map_texture_2=grim->Texture_Get(tempstring);
	can_draw_map=false;
	if(grim->System_SetRenderTarget(map_to_edit->map_texture)){
		can_draw_map=true;
		draw_map_grid_small(map_to_edit,map_to_edit->map_texture,map_to_edit->map_texture_2);
	}
	grim->System_SetRenderTarget(-1);
	if (!can_draw_map){
		grim->Texture_Delete(map_to_edit->map_texture);
		grim->Texture_Delete(map_to_edit->map_texture_2);
		map_to_edit->map_texture = -1;
		map_to_edit->map_texture_2 = -1;
	}
}

void game_engine::calculate_weather(void){
	int a,b;

	//change speed
	map_main->wind_speed=mod.general_climates[map_main->climate_number].maximum_wind_speed*(sincos.table_sin(time_from_beginning*0.15f)+0.25f*sincos.table_sin(time_from_beginning*0.05f+0.5f)+0.25f*sincos.table_sin(time_from_beginning*0.025f+2.0f));

	//change direction
	if(map_main->wind_direction_target<map_main->wind_direction){
		map_main->wind_direction-=0.0001f*elapsed*game_speed;
		//target reached
		if(map_main->wind_direction_target>=map_main->wind_direction){
			map_main->wind_direction_target=randDouble(0,2*pi);
		}
	}
	if(map_main->wind_direction_target>=map_main->wind_direction){
		map_main->wind_direction+=0.0001f*elapsed*game_speed;
		//target reached
		if(map_main->wind_direction_target<map_main->wind_direction){
			map_main->wind_direction_target=randDouble(0,2*pi);
		}
	}

	//rain timer
	if(map_main->rain_timer<=0){
		if(randInt(0,mod.general_climates[map_main->climate_number].rain_probability/elapsed*game_speed)==0){
			map_main->rain_timer=randDouble(mod.general_climates[map_main->climate_number].rain_length_min,mod.general_climates[map_main->climate_number].rain_length_max);
		}
	}



	//it's raining
	if(mod.general_climates[map_main->climate_number].rain_particle>=0)
	if(map_main->rain_timer>0){
		map_main->rain_timer-=elapsed*game_speed;

		if(!in_shade){
			//go throught all effect blocks
			for(a=0;a<mod.general_climates[map_main->climate_number].rain_effects.size();a++){

				if(time_from_beginning-rain_effect_timers[map_main->climate_number].subtype[a]<mod.general_climates[map_main->climate_number].rain_effects[a].interval*0.001f)continue;

				Mod::effect_base effect=mod.general_climates[map_main->climate_number].rain_effects[a].effect;

				//check conditions
				bool OK=true;
				for(b=0;b<effect.conditions.size();b++){
					if(!check_condition(effect.conditions[b],&map_main->creature[0],0,player_middle_x,player_middle_y,false)){
						OK=false;
						break;
					}
				}
				if(!OK)continue;

				//run effects
				for(b=0;b<effect.effects.size();b++){
					run_effect(effect.effects[b],&map_main->creature[0],0,player_middle_x,player_middle_y,map_main->creature[0].rotation,false);
				}

				if(effect.event_text!="none"){
					//show normal message
					text_manager.message(mod.general_climates[map_main->climate_number].rain_effects[a].interval+200,200,effect.event_text);
				}
			}
		}


		//raindrops
		if(mod.general_climates[map_main->climate_number].rain_particle>=0)
		for(int b=0;b<(int)10;b++){
			if(randInt(0,100/elapsed)==0){
				float random_speed=map_main->wind_speed+randDouble(-0.1f,0.1f);
				float angle=map_main->wind_direction+randDouble(-0.1f,0.1f);
				make_particle(mod.general_climates[map_main->climate_number].rain_particle,2,randDouble(mod.general_climates[map_main->climate_number].rain_particle_life_min,mod.general_climates[map_main->climate_number].rain_particle_life_max),randDouble(camera_x-300,camera_x+screen_width+300),randDouble(camera_y-300,camera_y+screen_height+300),random_speed*sincos.table_cos(angle),random_speed*sincos.table_sin(angle));
			}
		}
	}

}

void game_engine::calculate_mouse_on_creatures(void){

	int i,j,k;

	//check area
	int alku_x=(int)(camera_x/grid_size)-2;
	int alku_y=(int)(camera_y/grid_size)-2;
	int loppu_x=alku_x+(int)(screen_width/grid_size)+2;
	int loppu_y=alku_y+(int)(screen_height/grid_size)+2;

	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

	//all the creatures here
	for(i=alku_x;i<loppu_x;i++){
		for(j=alku_y;j<loppu_y;j++){
			if(map_main->grid[i].grid[j].total_creatures>0)
			for(k=0;k<map_main->grid[i].grid[j].total_creatures;k++){
				if(map_main->creature[map_main->grid[i].grid[j].creatures[k]].dead)continue;

				int creature=map_main->grid[i].grid[j].creatures[k];
				creature_base& thiscreature = map_main->creature[creature];

				if(creature==0)continue;//not player

				float size=mod.general_creatures[thiscreature.type].size*thiscreature.size*general_creature_size;
				float distance = sqr(thiscreature.x+size*0.5f-player_middle_x)+sqr(thiscreature.y+size*0.5f-player_middle_y);

				//energy bar
				thiscreature.show_energy=false;

				//mouse is on it
				if(sqr(thiscreature.x+size*0.5f-(camera_x+mousex))+sqr(thiscreature.y+size*0.5f-(camera_y+mousey))<sqr(size*0.35f)){

					//pick up corpses
					if(thiscreature.killed)
					if(mod.general_creatures[thiscreature.type].corpse_item>=0)
					if(mod.general_creatures[thiscreature.type].corpse_item_amount>0)
					{

						float touch_size=size;
						if(touch_size<80)touch_size=80;

						tempstring=mod.general_items[mod.general_creatures[thiscreature.type].corpse_item].name;

						//if the player is close enough we can show the use text
						//if(mod.general_plot_objects[map_main->items[map_main->grid[i].grid[j].items[k]].item_type].event!=8)//item is not a resting place
						if(distance<sqr(touch_size)){
							tempstring+=" (Right click to pick up)";
							if(!mouse_right&&mouse_right2){
								mouse_right2=false;
								give_item(mod.general_creatures[thiscreature.type].corpse_item,mod.general_creatures[thiscreature.type].corpse_item_amount,time_from_beginning,true);
								//thiscreature.dead=true;
								delete_creature(map_main,creature);
								k--;
								playsound(pick_up,1,0,0,0,0);
								return;
							}
						}
						else
							tempstring+=" (Too far to pick up)";

						text_manager.message(1000,1000,tempstring);
					}

					//show energy bar
					if(!map_main->creature[map_main->grid[i].grid[j].creatures[k]].killed){
						thiscreature.show_energy=true;
						creature_nearest_to_the_mouse=creature;
					}

					//if it's a moving fried, command it to stop
					if(thiscreature.side==map_main->creature[0].side)
					//if(mod.general_creatures[thiscreature.type].movement_speed!=0)
					if(!thiscreature.killed)
					{
						tempstring=mod.AI_tactics[thiscreature.tactic[thiscreature.AI_order]].name;
						tempstring+=" Mode (Right click to change)";

						text_manager.message(1000,1000,tempstring);

						//clicked
						if(!mouse_right&&mouse_right2){
							if(thiscreature.AI_order==0)
								thiscreature.AI_order=1;
							else if(thiscreature.AI_order==1)
								thiscreature.AI_order=0;

							AI_initiate_behavior_parameters(&thiscreature);
						}
					}

				}
			}
		}
	}
}


void game_engine::load_mod_names(const string& StartingPath)
{
	//load mod names
	mods=0;
	selected_mod=0;

	vector<string> v = grim->File_ListDirectory(StartingPath);

	for (int i=0; i<v.size(); i++) {
		if (grim->File_IsDirectory(StartingPath + '/' + v[i])) {
			mod_names[mods]=v[i];
			//select default mod as default
			if(mod_names[mods]=="Default")
				selected_mod=mods;
			mods++;
	   }
	}

	//load race names
	player_race=0;
	proposed_player_race=0;
	race_names.clear();
	int i,j;
	for(i=0;i<mods;i++){
		mod.mod_name=mod_names[i];
		mod.load_race_info("data/"+mod_names[i]+"/player_races.dat");
		name_vector temp_names;
		temp_names.names.clear();
		temp_names.description.clear();
		temp_names.visible.clear();
		temp_names.picture.clear();
		for(j=0;j<mod.general_races.size();j++){
			temp_names.names.push_back(mod.general_races[j].name);
			temp_names.description.push_back(mod.general_races[j].description);
			temp_names.visible.push_back(mod.general_races[j].visible_in_start_menu);
			temp_names.picture.push_back(mod.general_races[j].rag_doll);

		}
		temp_names.difficulty_level_descriptions.clear();
		for(j=0;j<mod.difficulty_level_descriptions.size();j++){
			temp_names.difficulty_level_descriptions.push_back(mod.difficulty_level_descriptions[j]);
		}

		race_names.push_back(temp_names);

	}
}

void game_engine::spawn_creature(int side, int tactic, int tactic2, float x, float y, float angle, int type, map *map){

	creature_base temp_creature;
	memset(&temp_creature, 0, sizeof(temp_creature));

	temp_creature.dead=false;
	//temp_creature.weapon_selected=0;
	temp_creature.rotation=angle;
	temp_creature.rotation_head=temp_creature.rotation;
	temp_creature.rotation_legs=temp_creature.rotation;
	temp_creature.size=1;
	temp_creature.type=type;
	//temp_creature.health=mod.general_creatures[temp_creature.type].health;
	temp_creature.killed=false;
	temp_creature.side=side;
	//temp_creature.sneak=false;
	temp_creature.animation[0]=0;
	temp_creature.animation[1]=2;
	temp_creature.animation[2]=5;
	temp_creature.carry_light=-1;
	//temp_creature.may_change_area=true;
	temp_creature.weapon_effects_amount=0;
	temp_creature.tactic[0]=tactic;
	if(tactic==-1)temp_creature.tactic[0]=mod.general_creatures[temp_creature.type].AI_tactics[0];
	temp_creature.tactic[1]=tactic2;
	if(tactic2==-1)temp_creature.tactic[1]=mod.general_creatures[temp_creature.type].AI_tactics[1];
	temp_creature.dialog=-1;
	temp_creature.fire_timer=0;
	temp_creature.last_bullet_hit_from_creature_number=-1;
	temp_creature.carried_creature=-1;
	temp_creature.touched_enemy=-1;
	temp_creature.fire=false;
	temp_creature.wall_between_creature_and_player=-1;
	temp_creature.force_AI=false;
	temp_creature.anger_level=0;


	temp_creature.x=x+randDouble(-1,1)-0.5f*64*mod.general_creatures[temp_creature.type].size;//it may be best to adjust the coordinate by just a little for collision checking
	temp_creature.y=y+randDouble(-1,1)-0.5f*64*mod.general_creatures[temp_creature.type].size;
	temp_creature.x2=temp_creature.x+randDouble(-128,128);
	temp_creature.y2=temp_creature.y+randDouble(-128,128);
	temp_creature.move_to_x=temp_creature.x;
	temp_creature.move_to_y=temp_creature.y;

	//timed creature scripts
	for(int c=0;c<mod.general_creatures[temp_creature.type].timed_block.size();c++){
		Mod::general_creatures_base::timed_effect_block effect=mod.general_creatures[temp_creature.type].timed_block[c];

		temp_creature.script_timer[c]=time_from_beginning+effect.interval*0.001f*randDouble(0,1);
	}

	//creature specialties
	initialize_creature_specialties(&temp_creature,map,true);
	/*for(int specialty=0;specialty<mod.general_creatures[temp_creature.type].specialties.size();specialty++){

		//add bars for creatures
		if(mod.general_creatures[temp_creature.type].specialties[specialty].number==0){
			temp_creature.bars[(int)mod.general_creatures[temp_creature.type].specialties[specialty].parameter0].value=mod.general_creatures[temp_creature.type].specialties[specialty].parameter3;
			temp_creature.bars[(int)mod.general_creatures[temp_creature.type].specialties[specialty].parameter0].minimum=mod.general_creatures[temp_creature.type].specialties[specialty].parameter1;
			temp_creature.bars[(int)mod.general_creatures[temp_creature.type].specialties[specialty].parameter0].maximum=mod.general_creatures[temp_creature.type].specialties[specialty].parameter2;
			temp_creature.bars[(int)mod.general_creatures[temp_creature.type].specialties[specialty].parameter0].active=true;
		}
	}*/



	AI_initiate_behavior_parameters(&temp_creature);
	map->creature.push_back(temp_creature);


}

void game_engine::set_edges(void){

	int a,b;
	int width, height;

	//load map file
	FILE *fil;
	char rivi[800];
	string filename="data/"+mod.mod_name+"/map.dat";

	int *map_list=NULL;

	fil = fopen(filename.c_str(),"rt");
	if(fil){
		width=atoi(stripped_fgets(rivi,sizeof(rivi),fil));
		height=atoi(stripped_fgets(rivi,sizeof(rivi),fil));

		map_list=new int[width*height];
		memset(map_list, 0, width*height*sizeof(int));
		//for(a=0;a<width*height;a++){
		//	map_list[a]=0;
		//}

		for(a=0;a<height;a++){

			fgets(rivi,sizeof(rivi),fil);

			vector<string>  ls;
			stringtok (ls, rivi, ";");
			//int d=ls.size();
			if(ls.size()<width){
				debug.debug_output("Misconfigured map.dat!",2,0);
			}

			for(b=0;b<width;b++){
				map_list[a*width+b]=atoi(ls[b].c_str());
			}
		}
	}
	fclose(fil);



	//queue all areas
	vector <int> areas_list;
	areas_list.clear();
	for(a=0;a<mod.general_areas.size();a++){
		if(mod.general_areas[a].dead)
			continue;
		areas_list.push_back(a);
	}



	//place areas that are placed by area classes
	for(a=0;a<width*height;a++){
		if(map_list[a]>=0){

			//find all areas that fit the class
			vector <int> areas_class;
			for(b=0;b<areas_list.size();b++){
				if(mod.general_areas[areas_list[b]].area_class==map_list[a])
					areas_class.push_back(areas_list[b]);
			}

			//place area
			if(areas_class.size()>0){
				map_list[a]=areas_class[randInt(0,areas_class.size())];
			}
			else{
				map_list[a]=-1;
			}

			//remove it from queue
			for(b=0;b<areas_list.size();b++){
				if(areas_list[b]==map_list[a]){
					areas_list.erase(areas_list.begin() + b);
					b--;
				}
			}
		}
	}

	//place all random areas that must exist
	for(a=0;a<width*height;a++){
		if(map_list[a]==-2){
			//choose any area that is not yet taken
			if(areas_list.size()>0){
				int take_area=randInt(0,areas_list.size());
				map_list[a]=areas_list[take_area];
				areas_list.erase(areas_list.begin() + take_area);//remove it
			}
		}
	}

	//place all random areas that may exist
	/*for(a=0;a<1000;a++){
		if(areas_list.size()==0)break;
		//pick any area
		int try_area=randInt(0,width*height);
		if(map_list[try_area]==-3){
			//find if there is a square adjacent to this one
			bool OK=false;

			//left
			if(try_area>0)
				if(map_list[try_area-1]>=0)OK=true;
			//right
			if(try_area<width*height-1)
				if(map_list[try_area+1]>=0)OK=true;
			//up
			if(try_area>=width)
				if(map_list[try_area-width]>=0)OK=true;
			//down
			if(try_area<(height-1)*width)
				if(map_list[try_area+width]>=0)OK=true;

			//area is nearby, set random area here
			if(OK){
				//choose any area that is not yet taken
				if(areas_list.size()>0){
					int take_area=randInt(0,areas_list.size());
					map_list[a]=areas_list[take_area];
					areas_list.erase(areas_list.begin() + take_area);//remove it
				}
			}
		}
	}*/


	//add area
	for(b=0;b<areas_list.size();b++){

		//create list of possibilities
		vector <int> possible_areas;
		for(a=0;a<width*height;a++){
			if(map_list[a]==-3){
				//find if there is a square adjacent to this one
				bool OK=false;

				//left
				if(a>0)
					if(map_list[a-1]>=0)OK=true;
				//right
				if(a<width*height-1)
					if(map_list[a+1]>=0)OK=true;
				//up
				if(a>=width)
					if(map_list[a-width]>=0)OK=true;
				//down
				if(a<(height-1)*width)
					if(map_list[a+width]>=0)OK=true;

				//area is nearby, list it
				if(OK){
					possible_areas.push_back(a);
				}
			}
		}

		//no possible areas
		if(possible_areas.size()==0)
			break;
		//find a possible spot, and put the area there
		int try_area=randInt(0,possible_areas.size());
		map_list[possible_areas[try_area]]=areas_list[b];
		//possible_areas.erase(possible_areas.begin() + try_area);
		areas_list.erase(areas_list.begin() + b);//remove it
		b--;


	}

	//map edges
	//clear all
	for(a=0;a<map_storage.size();a++){
		for(b=0;b<4;b++){
			map_storage[a]->near_areas[b]=-1;
		}
		/*if(map_list[a]>=0){
			for(b=0;b<4;b++){
				map_storage[map_list[a]]->near_areas[b]=-1;
			}
		}*/
	}
	//go through all map places
	for(a=0;a<width*height;a++){
		//if it has an area
		if(map_list[a]>=0){
			//find if it has a neighbor
			//left
			if(a>0)
				if(map_list[a-1]>=0)map_storage[map_list[a]]->near_areas[0]=map_list[a-1];
			//right
			if(a<width*height-1)
				if(map_list[a+1]>=0)map_storage[map_list[a]]->near_areas[1]=map_list[a+1];
			//up
			if(a>=width)
				if(map_list[a-width]>=0)map_storage[map_list[a]]->near_areas[2]=map_list[a-width];
			//down
			if(a<(height-1)*width)
				if(map_list[a+width]>=0)map_storage[map_list[a]]->near_areas[3]=map_list[a+width];
		}
	}

	//set wrapping edges
	for(a=0;a<map_storage.size();a++){
		//horizontal wrapping
		if((mod.general_areas[map_storage[a]->area_type].wrap_type==0)||(mod.general_areas[map_storage[a]->area_type].wrap_type==2)){
			map_storage[a]->near_areas[0]=a;
			map_storage[a]->near_areas[1]=a;
		}
		//vertical wrapping
		if((mod.general_areas[map_storage[a]->area_type].wrap_type==1)||(mod.general_areas[map_storage[a]->area_type].wrap_type==2)){
			map_storage[a]->near_areas[2]=a;
			map_storage[a]->near_areas[3]=a;
		}

	}



	SAFE_DELETE_ARRAY(map_list);
}

void game_engine::calculate_weight(void){
	int a;

	carry_weight=0;
	for(a=0;a<inventory[active_inventory].player_items.size();a++){
		if(inventory[active_inventory].player_items[a].amount<=0)continue;
		carry_weight+=inventory[active_inventory].player_items[a].amount*mod.general_items[inventory[active_inventory].player_items[a].item].weight;
	}

	mod.general_creatures[map_main->creature[0].type].movement_speed=player_normal_speed;

	//slow player down if too much weight
	if(carry_weight>maximum_carry_weight){
		text_manager.message(1000,1000,"You are overburdened, drop something.");


		mod.general_creatures[map_main->creature[0].type].movement_speed=
			mod.general_creatures[map_main->creature[0].type].movement_speed*
			(1-(carry_weight-maximum_carry_weight)/maximum_carry_weight*1.5f);

		if(mod.general_creatures[map_main->creature[0].type].movement_speed<0.001f)
			mod.general_creatures[map_main->creature[0].type].movement_speed=0.001f;
	}
}

void game_engine::show_slider(int type, int item, int item2, int x, int y, int point){
	slider_active=true;
	slider_x=x;
	slider_y=y;
	slider_item=inventory[active_inventory].player_items[item].item;
	slider_item2=inventory[active_inventory].player_items[item2].item;
	slider_type=type;
	slider_point=point;

	//middle
	if(x==-1){
		slider_x=screen_width/2-70.0f;
		slider_y=screen_height/2+70.0f;
	}

	if(slider_x<100)
		slider_x=100;
	if(slider_y<25)
		slider_y=25;
	if(slider_x>screen_width-233)
		slider_x=screen_width-233;
	if(slider_x>screen_height-90)
		slider_x=screen_height-90;


	mouse_left=false;
	mouse_left2=false;
	override_mouse_left=false;
	override_mouse_left2=false;
	override_mousex=mousex;
	override_mousey=mousey;
	key_d=false;
	key_d2=false;
	input_override=true;

	slider_minimum=0;

	//0=give items to player
	if(slider_type==0){
		slider_maximum=map_main->items[item].amount;
	}

	//1=drop items to ground
	if(slider_type==1){
		slider_maximum=inventory[active_inventory].player_items[item].amount;
	}

	//2=countdown to combine items
	if(slider_type==2){
		slider_maximum=slider_point;
		pop_up_mode=1;
	}
}

void game_engine::draw_slider(void){

	if(!slider_active)return;
	if(ask_continue_game)return;

	input_override=true;

	//draw background
	grim->Quads_SetSubset(0,0,1,1);
	grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
	grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	grim->Quads_SetRotation(0);
	resources.Texture_Set(slider_texture);
	grim->Quads_SetColor(1,1,1,1);
	grim->Quads_Begin();
	//draw small computer
	grim->Quads_Draw(slider_x-100, slider_y-24, 256*1.3f, 128*0.65f);
	grim->Quads_End();

	bool show_cancel=true;
	bool show_ok=false;
	bool show_all=false;
	bool move_by_mouse=false;

	//0=give items to player
	if(slider_type==0){
		show_ok=true;
		show_all=true;
		move_by_mouse=true;
	}

	//1=drop items to ground
	if(slider_type==1){
		show_ok=true;
		show_all=true;
		move_by_mouse=true;
	}

	//2=countdown to combine items
	if(slider_type==2){
		slider_point-=elapsed*game_speed*0.001f;

		if(slider_point<0){

			/*int combines_to=0;
			int amount=0;
			for(int b=0;b<mod.general_items[inventory[active_inventory].player_items[slider_item2].item].combines_with_max;b++){
				if(mod.general_items[inventory[active_inventory].player_items[slider_item2].item].combines_with[b]==inventory[active_inventory].player_items[slider_item].item){
					combines_to=mod.general_items[inventory[active_inventory].player_items[slider_item2].item].combines_to[b];
					amount=mod.general_items[inventory[active_inventory].player_items[slider_item2].item].combines_amount[b];
				}
			}
			for(b=0;b<mod.general_items[inventory[active_inventory].player_items[slider_item].item].combines_with_max;b++){
				if(mod.general_items[inventory[active_inventory].player_items[slider_item].item].combines_with[b]==inventory[active_inventory].player_items[slider_item2].item){
					combines_to=mod.general_items[inventory[active_inventory].player_items[slider_item].item].combines_to[b];
					amount=mod.general_items[inventory[active_inventory].player_items[slider_item].item].combines_amount[b];
				}
			}*/

			//find the combined items
			int item_1=-1;
			int item_2=-1;
			int a;
			for(a=0;a<inventory[active_inventory].player_items.size();a++){
				if(inventory[active_inventory].player_items[a].item<=-1)continue;
				if(inventory[active_inventory].player_items[a].amount<=0)continue;
				if(mod.general_items[inventory[active_inventory].player_items[a].item].visible_in_inventory==0)continue;
				if(inventory[active_inventory].player_items[a].item==slider_item)
					item_1=a;
				if(inventory[active_inventory].player_items[a].item==slider_item2)
					item_2=a;
			}
			if(item_1==-1)return;
			if(item_2==-1)return;


			Mod::combines combination;
			bool discard_this=false;
			bool discard_that=false;
			bool combination_found=false;


			if(item_has_combination(inventory[active_inventory].player_items[item_1].item,inventory[active_inventory].player_items[item_2].item,&combination)){
				discard_this=combination.discard_this;
				discard_that=combination.discard_that;
				combination_found=true;
			}
			if(item_has_combination(inventory[active_inventory].player_items[item_2].item,inventory[active_inventory].player_items[item_1].item,&combination)){
				discard_this=combination.discard_that;
				discard_that=combination.discard_this;
				combination_found=true;
			}

			if(combination_found){
				combine_items(item_1,item_2,combination.combine_results,discard_this,discard_that);
			}

			slider_active=false;
			disable_input_override();
			pop_up_mode=3;
			return;
		}
	}



	//slider bar
	resources.Texture_Set(bar_texture);
	grim->Quads_Begin();
		float slider_bar_x=0;
		float slider_bar_y=0;
		float slider_length=128;
		float slider_height=32;
		int slider_point_x=((float)slider_point/(float)(slider_maximum-slider_minimum))*slider_length;
		grim->Quads_SetColor(1,1,1,1);

		//left side
		grim->Quads_SetSubset((0/16.0f),(0/16.0f),(7/16.0f),(16/16.0f));
		grim->Quads_Draw((slider_x+slider_bar_x-7), (slider_y+slider_bar_y), 7, slider_height);
		//middle part
		grim->Quads_SetSubset((7/16.0f),(0/16.0f),(9/16.0f),(16/16.0f));
		grim->Quads_Draw((slider_x+slider_bar_x), (slider_y+slider_bar_y), (slider_length), slider_height);
		//right side
		grim->Quads_SetSubset((9/16.0f),(0/16.0f),(16/16.0f),(16/16.0f));
		grim->Quads_Draw((slider_x+slider_bar_x+slider_length), (slider_y+slider_bar_y), 7, slider_height);

		//change with mouse
		grim->Quads_SetColor(1,1,0,1);
		if(move_by_mouse){
			if((override_mousey>slider_y+slider_bar_y)&&(override_mousey<slider_y+slider_bar_y+slider_height)){
				grim->Quads_SetColor(0.8f,0.8f,0,1);

				if(override_mouse_left){
					slider_point=(int)(((override_mousex-slider_x-slider_bar_x)/slider_length)*(slider_maximum-slider_minimum)+0.5f);

					if(slider_point<slider_minimum)
						slider_point=slider_minimum;
					if(slider_point>slider_maximum)
						slider_point=slider_maximum;
				}

			}
		}

		//slider point
		grim->Quads_SetSubset((0/16.0f),(0/16.0f),(16/16.0f),(16/16.0f));
		grim->Quads_Draw((slider_x+slider_bar_x-8+slider_point_x), (slider_y+slider_bar_y), 16, 32);

	grim->Quads_End();



	//numbers
	grim->Quads_SetColor(1,1,1,1);
	//itoa(slider_point,temprivi,10);
	sprintf(temprivi,"%d",slider_point);
	text_manager.write_line(font,slider_x+slider_bar_x-66,slider_y+slider_bar_y+1,temprivi,2);
	//itoa(slider_maximum,temprivi,10);
	sprintf(temprivi,"%d",slider_maximum);
	text_manager.write_line(font,slider_x+slider_bar_x+slider_length+2,slider_y+slider_bar_y+1,temprivi,2);


	//cancel
	if(show_cancel){
		//draw button
		grim->Quads_SetSubset(0.0f/128.0f,0.0f/128.0f,128.0f/128.0f,128.0f/128.0f);
		grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
		grim->Quads_SetRotation(0);
		resources.Texture_Set(slider_texture);
		grim->Quads_SetColor(1,1,1,1);
		int button_left=-37;
		int button_top=38;
		int button_width=100;
		int button_height=48;

		//escape
		if(!override_key_escape&&override_key_escape2){
			slider_active=false;
			disable_input_override();
		}
		//mouse
		if((override_mousex>slider_x+button_left)&&(override_mousex<slider_x+button_left+button_width)&&(override_mousey>slider_y+button_top)&&(override_mousey<slider_y+button_top+button_height)){
			//text_manager.write_line(font,override_mousex+5, override_mousey-6,"Cancel",1);
			grim->Quads_SetColor(0.8f,0.8f,0.8f,1);
			if(!override_mouse_left&&override_mouse_left2){
				slider_active=false;
				disable_input_override();

				//2=countdown to combine items
				if(slider_type==2){
					pop_up_mode=3;
				}
			}
		}
		grim->Quads_Begin();
		grim->Quads_Draw(slider_x+button_left, slider_y+button_top, button_width, button_height);
		grim->Quads_End();
		text_manager.write_line(font,slider_x+button_left+20,slider_y+button_top+16,"Cancel",1);
	}

	//ok
	if(show_ok){
		//draw button
		grim->Quads_SetSubset(0.0f/128.0f,0.0f/128.0f,128.0f/128.0f,128.0f/128.0f);
		grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
		grim->Quads_SetRotation(0);
		resources.Texture_Set(slider_texture);
		grim->Quads_SetColor(1,1,1,1);
		int button_left=68;
		int button_top=38;
		int button_width=100;
		int button_height=48;


		//mouse
		if((override_mousex>slider_x+button_left)&&(override_mousex<slider_x+button_left+button_width)&&(override_mousey>slider_y+button_top)&&(override_mousey<slider_y+button_top+button_height)){
			//text_manager.write_line(font,override_mousex+5, override_mousey-6,"Accept",1);
			grim->Quads_SetColor(0.8f,0.8f,0.8f,1);
			if(!override_mouse_left&&override_mouse_left2){

				//find the combined items
				int item_1=-1;
				int a;
				for(a=0;a<inventory[active_inventory].player_items.size();a++){
					if(inventory[active_inventory].player_items[a].item<=-1)continue;
					if(inventory[active_inventory].player_items[a].amount<=0)continue;
					if(mod.general_items[inventory[active_inventory].player_items[a].item].visible_in_inventory==0)continue;
					if(inventory[active_inventory].player_items[a].item==slider_item)
						item_1=a;
				}
				if(item_1==-1)return;


				//0=give items to player
				if(slider_type==0){
					give_item(map_main->items[item_1].type,slider_point,time_from_beginning, true);

					map_main->items[item_1].amount-=slider_point;
					if(map_main->items[item_1].amount==0){
						map_main->items[item_1].dead=true;
					}

				}

				//1=drop items to ground
				if(slider_type==1){

					create_item(map_main,inventory[active_inventory].player_items[item_1].item,slider_point,player_middle_x,player_middle_y,-1);

					inventory[active_inventory].player_items[item_1].amount-=slider_point;
					if(inventory[active_inventory].player_items[item_1].amount==0){
						//unuse item
						if(inventory[active_inventory].player_items[item_1].wielded)
						for(int c=0;c<mod.general_items[inventory[active_inventory].player_items[item_1].item].effects.size();c++){
							use_item(inventory[active_inventory].player_items[item_1].item,&item_1, mod.general_items[inventory[active_inventory].player_items[item_1].item].effects[c].effect,true, false, false,false);//unuse
						}
						delete_item(item_1);
					}
					arrange_item_list(false);
				}

				slider_active=false;
				disable_input_override();
			}
		}
		grim->Quads_Begin();
		grim->Quads_Draw(slider_x+button_left, slider_y+button_top, button_width, button_height);
		grim->Quads_End();
		text_manager.write_line(font,slider_x+button_left+36,slider_y+button_top+16,"OK",1);
	}
}

void game_engine::disable_input_override(void){
	if(input_override){

		input_override=false;
		mouse_left=false;
		mouse_left2=false;
		mousex=override_mousex;
		mousey=override_mousey;
		key_escape=false;
		key_escape2=false;
		mouse_wheel=0;
	}

}


bool game_engine::race_specialty(int find_specialty, Mod::specialty *specialty){

	for(int i=0;i<mod.general_races[player_race].specialties.size();i++){
		if(!mod.general_races[player_race].specialties[i].difficulty[game_difficulty_level])continue;
		//specialty found
		if(mod.general_races[player_race].specialties[i].number==find_specialty){
			*specialty=mod.general_races[player_race].specialties[i];

			/**specialty_parameter0=mod.general_races[player_race].specialties[i].parameter0;
			*specialty_parameter1=mod.general_races[player_race].specialties[i].parameter1;
			*specialty_parameter2=mod.general_races[player_race].specialties[i].parameter2;
			*specialty_parameter3=mod.general_races[player_race].specialties[i].parameter3;*/
			return true;
		}
	}

	return false;

}



vector <game_engine::collision_base> game_engine::list_collisions(float x1,float y1, float x2, float y2, bool only_visible_area){

	//find the map squares between last frame and this one
	int xx1,xx2,yy1,yy2;
	int a,b,c;
	vector <collision_base> collisions;
	collisions.clear();

	//not moving at all, set a little difference
	if((x1==x2)&&(y1==y2)){
		x2+=randDouble(-1,1);
		y2+=randDouble(-1,1);
	}



	//viivan alku ja loppu
	if(x1>x2){
		float tempx=x1;
		x1=x2;
		x2=tempx;
		float tempy=y1;
		y1=y2;
		y2=tempy;
	}
	xx1=(int)(x1/(general_object_size))-1;
	xx2=(int)(x2/(general_object_size))+1;


	//check that it is in view and in range
	if(only_visible_area){
		if(xx1<screen_start_x-3)xx1=screen_start_x-3;
		if(xx1>=screen_end_x+3)xx1=screen_end_x+3;
		if(xx2<screen_start_x-3)xx2=screen_start_x-3;
		if(xx2>=screen_end_x+3)xx2=screen_end_x+3;
	}

	if(xx1<0)xx1=0;
	if(xx2>map_main->sizex-1)xx2=map_main->sizex-1;


	float yalku,yloppu;
	yalku=y1;
	yloppu=y2;
	if(yalku<yloppu){
		yy1=(int)(yalku/(general_object_size))-1;
		yy2=(int)(yloppu/(general_object_size))+1;
	}
	else{
		yy2=(int)(yalku/(general_object_size))+1;
		yy1=(int)(yloppu/(general_object_size))-1;
	}

	//check that it is in view and in range
	if(only_visible_area){
		if(yy1<screen_start_y-3)yy1=screen_start_y-3;
		if(yy1>=screen_end_y+3)yy1=screen_end_y+3;
		if(yy2<screen_start_y-3)yy2=screen_start_y-3;
		if(yy2>=screen_end_y+3)yy2=screen_end_y+3;
	}
	if(yy1<0)yy1=0;
	if(yy2>map_main->sizey-1)yy2=map_main->sizey-1;


	//go through all the x blocks
	for (a=xx1; a<=xx2; a++){


		//go through all the y blocks
		for (b=yy1; b<=yy2; b++){
			//go through all the creatures on this map square
			for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){

				//rule out the shooter's side
				//if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].side==map_main->bullets[i].side)continue;
				//killed
				if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].killed)continue;


				//find out if the bullet hits the creature
				float size=mod.general_creatures[map_main->creature[map_main->grid[a].grid[b].creatures[c]].type].size*map_main->creature[map_main->grid[a].grid[b].creatures[c]].size*general_creature_size;
				float radius=size*0.5f*0.5f;
				float creature_x=map_main->creature[map_main->grid[a].grid[b].creatures[c]].x+size*0.5f;
				float creature_y=map_main->creature[map_main->grid[a].grid[b].creatures[c]].y+size*0.5f;

				vector <point2d> p;

				p=sphere_line_intersection (
				x1 , y1 ,
				x2 , y2 ,
				creature_x , creature_y ,
				radius );

				//no hit found
				if(p.size()==0)continue;

				point2d temp_point=p[0];
				if(p.size()>1)
				if(sqr(x1-p[0].x)+sqr(y1-p[0].y)>sqr(x1-p[1].x)+sqr(y1-p[1].y))
					temp_point=p[1];


				//check if this creature is already listed
				bool listed=false;
				for(int d=0;d<collisions.size();d++){
					if(collisions[d].type==0)
					if(collisions[d].subtype==map_main->grid[a].grid[b].creatures[c])
						listed=true;
				}

				//it's not, list it
				if(!listed){
					collision_base temp_collision;
					temp_collision.type=0;
					temp_collision.subtype=map_main->grid[a].grid[b].creatures[c];
					temp_collision.x=temp_point.x;
					temp_collision.y=temp_point.y;
					collisions.push_back(temp_collision);
				}



			}
		}
	}

	return collisions;
}

void game_engine::draw_line_map(int x1, int y1, int width, int height, const map* map_draw){

	float r,g,b;
	//left
	r=1;g=0;b=0;
	if(map_draw->near_areas[0]>=0){r=0;g=1;b=0;}
	text_manager.draw_line(x1,y1,x1,y1+height,1.5f,0.5f,0.5f,r,g,b);

	//right
	r=1;g=0;b=0;
	if(map_draw->near_areas[1]>=0){r=0;g=1;b=0;}
	text_manager.draw_line(x1+width,y1,x1+width,y1+height,1.5f,0.5f,0.5f,r,g,b);

	//top
	r=1;g=0;b=0;
	if(map_draw->near_areas[2]>=0){r=0;g=1;b=0;}
	text_manager.draw_line(x1,y1,x1+width,y1,1.5f,0.5f,0.5f,r,g,b);

	//bottom
	r=1;g=0;b=0;
	if(map_draw->near_areas[3]>=0){r=0;g=1;b=0;}
	text_manager.draw_line(x1,y1+height,x1+width,y1+height,1.5f,0.5f,0.5f,r,g,b);

}

bool game_engine::has_terrain_effect(map *map_to_edit, int terrain_type, int search_effect, Mod::effect *effect){
	/*int a;


	for(a=0;a<mod.general_climates[map_to_edit->climate_number].terrain_types[terrain_type].terrain_effects.size();a++){
		if(mod.general_climates[map_to_edit->climate_number].terrain_types[terrain_type].terrain_effects[a].effect_type==search_effect){

			*parameter0=mod.general_climates[map_to_edit->climate_number].terrain_types[terrain_type].terrain_effects[a].parameter0;
			*parameter1=mod.general_climates[map_to_edit->climate_number].terrain_types[terrain_type].terrain_effects[a].parameter1;
			return true;
		}
	}
	return false;*/

	for(unsigned int a=0;a<mod.terrain_types[terrain_type].effects.size();a++){
		for(unsigned int b=0;b<mod.terrain_types[terrain_type].effects[a].effect.effects.size();b++){
			if(mod.terrain_types[terrain_type].effects[a].effect.effects[b].effect_number==search_effect){
				*effect=mod.terrain_types[terrain_type].effects[a].effect.effects[b];
				return true;
			}
		}
	}
	return false;
}


bool game_engine::item_has_effect(int item_type, int search_effect, Mod::effect *effect){
	unsigned int a,b;
	for(a=0;a<mod.general_items[item_type].effects.size();a++){
		for(b=0;b<mod.general_items[item_type].effects[a].effect.effects.size();b++){
			if(mod.general_items[item_type].effects[a].effect.effects[b].effect_number==search_effect){
				*effect=mod.general_items[item_type].effects[a].effect.effects[b];
				return true;
			}
		}
	}
	return false;
}

bool game_engine::plot_object_has_effect(int object_type, int search_effect, Mod::effect *effect){
	unsigned int a,b;
	for(a=0;a<mod.general_plot_objects[object_type].effects.size();a++){
		for(b=0;b<mod.general_plot_objects[object_type].effects[a].effects.size();b++){
			if(mod.general_plot_objects[object_type].effects[a].effects[b].effect_number==search_effect){
				*effect=mod.general_plot_objects[object_type].effects[a].effects[b];
				return true;
			}
		}
	}
	return false;
}

bool game_engine::item_has_combination(int item_type, int search_combination, Mod::combines *combination){
	unsigned int a;
	for(a=0;a<mod.general_items[item_type].combinations.size();a++){
		if(mod.general_items[item_type].combinations[a].combines_with==search_combination){
			*combination=mod.general_items[item_type].combinations[a];
			return true;
		}
	}
	return false;
}

Key game_engine::translate_key_KEY(char key){

	key=tolower(key);

	switch (key){
		case 'a': return KEY_A;
		case 'b': return KEY_B;
		case 'c': return KEY_C;
		case 'd': return KEY_D;
		case 'e': return KEY_E;
		case 'f': return KEY_F;
		case 'g': return KEY_G;
		case 'h': return KEY_H;
		case 'i': return KEY_I;
		case 'j': return KEY_J;
		case 'k': return KEY_K;
		case 'l': return KEY_L;
		case 'm': return KEY_M;
		case 'n': return KEY_N;
		case 'o': return KEY_O;
		case 'p': return KEY_P;
		case 'q': return KEY_Q;
		case 'r': return KEY_R;
		case 's': return KEY_S;
		case 't': return KEY_T;
		case 'u': return KEY_U;
		case 'v': return KEY_V;
		case 'w': return KEY_W;
		case 'x': return KEY_X;
		case 'y': return KEY_Y;
		case 'z': return KEY_Z;
		case '0': return KEY_0;
		case '1': return KEY_1;
		case '2': return KEY_2;
		case '3': return KEY_3;
		case '4': return KEY_4;
		case '5': return KEY_5;
		case '6': return KEY_6;
		case '7': return KEY_7;
		case '8': return KEY_8;
		case '9': return KEY_9;
	}

    //This might not quite be what we need, but whatev.
	return KEY_SPACE;
}

int game_engine::translate_key_int(char key){

	key=tolower(key);

	switch (key){
		case 'a': return 1;
		case 'b': return 2;
		case 'c': return 3;
		case 'd': return 4;
		case 'e': return 5;
		case 'f': return 6;
		case 'g': return 7;
		case 'h': return 8;
		case 'i': return 9;
		case 'j': return 10;
		case 'k': return 11;
		case 'l': return 12;
		case 'm': return 13;
		case 'n': return 14;
		case 'o': return 15;
		case 'p': return 16;
		case 'q': return 17;
		case 'r': return 18;
		case 's': return 19;
		case 't': return 20;
		case 'u': return 21;
		case 'v': return 22;
		case 'w': return 23;
		case 'x': return 24;
		case 'y': return 25;
		case 'z': return 26;
		case '0': return 27;
		case '1': return 28;
		case '2': return 29;
		case '3': return 30;
		case '4': return 31;
		case '5': return 32;
		case '6': return 33;
		case '7': return 34;
		case '8': return 35;
		case '9': return 36;
	}

	return 0;
}

bool game_engine::check_condition(const Mod::condition& condition, const creature_base *creature, int creature_number, float x, float y, bool show_message){

	unsigned int a,b,c;

	switch(condition.condition_number){
	//0=must have item parameter0 amount parameter1 (use 0 for checking if the player doesn't have such an item)
	case 0:
		{
			//find if we have such an item
			bool found=false;
			if(condition.condition_parameter1>0){
				for(a=0;a<inventory[active_inventory].player_items.size();a++){
					if((inventory[active_inventory].player_items[a].item==condition.condition_parameter0)&&(inventory[active_inventory].player_items[a].amount>=condition.condition_parameter1)){
						found=true;
					}
				}
				//not found, return
				if(!found){
					if(show_message){
						if(isvowel(mod.general_items[condition.condition_parameter0].name.at(0)))tempstring="An ";
						else tempstring="A ";
						tempstring+=mod.general_items[condition.condition_parameter0].name;
						tempstring+=" required for this to work.";
						text_manager.message(3000,3000,tempstring);
					}
					return false;
				}
			}
			//checking if there are no such items
			else if(condition.condition_parameter1==0){
				for(a=0;a<inventory[active_inventory].player_items.size();a++)
					if((inventory[active_inventory].player_items[a].item==condition.condition_parameter0)&&(inventory[active_inventory].player_items[a].amount>0))
						found=true;
				if(found)return false;
			}
		}

	break;
	//1=must be distance parameter2 + object size from plot_object parameter0
	case 1:
		{
			int start_x=(int)(x/grid_size)-(2+condition.condition_parameter1/128);
			int start_y=(int)(y/grid_size)-(2+condition.condition_parameter1/128);
			int end_x=(int)(x/grid_size)+(2+condition.condition_parameter1/128);
			int end_y=(int)(y/grid_size)+(2+condition.condition_parameter1/128);

			if(start_x<0)start_x=0;
			if(end_x>map_main->sizex-1)end_x=map_main->sizex-1;
			if(start_y<0)start_y=0;
			if(end_y>map_main->sizey-1)end_y=map_main->sizey-1;

			bool found=false;

			for (a=start_x; a<end_x; a++){
				for (b=start_y; b<end_y; b++){
					for (c=0; c<map_main->grid[a].grid[b].items.size(); c++){
						if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
						if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=0)continue;
						if(map_main->items[map_main->grid[a].grid[b].items[c]].item_type==condition.condition_parameter0){

							float size=map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size;
							float distance=sqr(x-(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f))+sqr(y-(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f));

							if(distance<sqr(size*0.5f+condition.condition_parameter1))
								found=true;
						}
					}
				}
			}

			if(!found){
				if(show_message){
					tempstring="Must be near ";
					if(isvowel(mod.general_plot_objects[condition.condition_parameter0].name.at(0)))tempstring+="an ";
					else tempstring+="a ";
					tempstring+=mod.general_plot_objects[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;


	//2=must be distance parameter2 + object size from plot_object class parameter0
	case 2:
		{
			int start_x=(int)(x/grid_size)-(2+condition.condition_parameter1/128);
			int start_y=(int)(y/grid_size)-(2+condition.condition_parameter1/128);
			int end_x=(int)(x/grid_size)+(2+condition.condition_parameter1/128);
			int end_y=(int)(y/grid_size)+(2+condition.condition_parameter1/128);

			if(start_x<0)start_x=0;
			if(end_x>map_main->sizex-1)end_x=map_main->sizex-1;
			if(start_y<0)start_y=0;
			if(end_y>map_main->sizey-1)end_y=map_main->sizey-1;

			bool found=false;

			for (a=start_x; a<end_x; a++){
				for (b=start_y; b<end_y; b++){
					for (c=0; c<map_main->grid[a].grid[b].items.size(); c++){
						if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
						if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=0)continue;
						if(mod.general_plot_objects[map_main->items[map_main->grid[a].grid[b].items[c]].item_type].plot_object_class==condition.condition_parameter0){

							float size=map_main->items[map_main->grid[a].grid[b].items[c]].size*general_object_size;
							float distance=sqr(x-(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f))+sqr(y-(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f));

							if(distance<sqr(size*0.5f+condition.condition_parameter1))
								found=true;
						}
					}
				}
			}

			if(!found){
				if(show_message){
					tempstring="Must be near ";
					if(isvowel(mod.plot_object_classes[condition.condition_parameter0].at(0)))tempstring+="an ";
					else tempstring+="a ";
					tempstring+=mod.plot_object_classes[condition.condition_parameter0];
					tempstring+=".";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//3=creature's bar parameter0 is greater or equal to parameter1
	case 3:
		{
			//no such bar
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//not enough
			if(creature->bars[(int)condition.condition_parameter0].value<condition.condition_parameter1){
				if(show_message){
					tempstring="Must have more ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;

	//4=killed all creatures parameter0 from area parameter1 (-1=current area)
	case 4:
		{
			//find right area
			int area=condition.condition_parameter1;
			if(area==-1)area=player_area;
			for(a=0;a<map_storage.size();a++){
				if(map_storage[a]->area_type==area){
					//go trough all aliens, and see if there are any of parameter0 alive
					for(c=0;c<map_storage[a]->creature.size();c++){
						if(map_storage[a]->creature[c].dead)continue;
						if(map_storage[a]->creature[c].type!=condition.condition_parameter0)continue;
						if(map_storage[a]->creature[c].killed)continue;
						if(show_message){
							tempstring="Must destroy all ";
							tempstring+=mod.general_creatures[map_storage[a]->creature[c].type].name;
							tempstring+="s from ";
							tempstring+=mod.general_areas[map_storage[a]->area_type].name;
							tempstring+=" first.";

							text_manager.message(3000,3000,tempstring);
						}
						return false;
					}
				}
			}
		}
	break;
	//5=bar parameter0 is smaller than parameter1
	case 5:
		{
			//no such bar
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//not enough
			if(creature->bars[(int)condition.condition_parameter0].value>=condition.condition_parameter1){
				if(show_message){
					tempstring="Must have less ";
					//tempstring+=" (add code here!) ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//6=player is race parameter0
	case 6:
		{
			if(player_race!=condition.condition_parameter0)return false;
		}
	break;
	//7=player is not race parameter0
	case 7:
		{
			if(player_race==condition.condition_parameter0)return false;
		}
	break;
	//8=random integer between 0 and parameter1 is 0
	case 8:
		{
			if(randInt(0,condition.condition_parameter0)!=0)return false;
		}
	break;
	//9=player (parameter0, 0=is, 1=is not) in shade
	case 9:
		{
			if(condition.condition_parameter0==0)
				if(!in_shade)return false;
			if(condition.condition_parameter0==1)
				if(in_shade)return false;
		}
	break;
	//10=creature has eaten parameter0 eat items
	case 10:
		{
			if(creature->eat_counter<condition.condition_parameter0)return false;
		}
	break;
	//11=item parameter0 is (parameter1=0=wielded, parameter1=1=not wielded)
	case 11:
		{
			int item=-1;
			for(a=0;a<inventory[active_inventory].player_items.size();a++){
				if(inventory[active_inventory].player_items[a].item==condition.condition_parameter0){
					item=a;
				}
			}

			if(condition.condition_parameter1==0){
				if(item==-1)return false;
				if(inventory[active_inventory].player_items[item].wielded)return true;

				else if(show_message){
					tempstring=mod.general_items[condition.condition_parameter0].name;
					tempstring+=" must be wielded.";
					text_manager.message(3000,3000,tempstring);
					return false;
				}
				return false;
			}

			else if(condition.condition_parameter1==1){
				if(item==-1)return true;
				if(inventory[active_inventory].player_items[item].wielded){
					if(show_message){
						tempstring=mod.general_items[condition.condition_parameter0].name;
						tempstring+=" must not be wielded.";
						text_manager.message(3000,3000,tempstring);
					}
					return false;
				}
				return true;
			}
			return false;
		}
	break;
	//12=game difficulty is (parameter0, 0=higher than, 1=lower than, 2=equal to) parameter1 (0=easy, 1=normal, 2=hard)
	case 12:
		{
			if(condition.condition_parameter0==0){
				if(game_difficulty_level<=condition.condition_parameter1)return false;
			}
			else if(condition.condition_parameter0==1){
				if(game_difficulty_level>=condition.condition_parameter1)return false;
			}
			else if(condition.condition_parameter0==2){
				if(game_difficulty_level!=condition.condition_parameter1)return false;
			}
		}
	break;
	//13=area (parameter1, 0=is, 1=is not) parameter0
	case 13:
		{
			if(condition.condition_parameter1==0)
				if(player_area!=condition.condition_parameter0){
					if(show_message){
						tempstring="Must be at area ";
						tempstring=mod.general_areas[condition.condition_parameter0].name;
						tempstring+=" for this to work.";

						text_manager.message(3000,3000,tempstring);
					}
					return false;
				}
			if(condition.condition_parameter1==1)
				if(player_area==condition.condition_parameter0){
					if(show_message){
						tempstring="Must not be at area ";
						tempstring=mod.general_areas[condition.condition_parameter0].name;
						tempstring+=" for this to work.";

						text_manager.message(3000,3000,tempstring);
					}
					return false;
				}
		}
	break;
	//14=creature nearer than distance parameter1 + creature size pixels from creature parameter0 (negative value = creature type)
	case 14:
		{
			int start_x=(int)(x/grid_size)-(5+condition.condition_parameter1/128);
			int start_y=(int)(y/grid_size)-(5+condition.condition_parameter1/128);
			int end_x=(int)(x/grid_size)+(5+condition.condition_parameter1/128);
			int end_y=(int)(y/grid_size)+(5+condition.condition_parameter1/128);

			if(start_x<0)start_x=0;
			if(end_x>map_main->sizex-1)end_x=map_main->sizex-1;
			if(start_y<0)start_y=0;
			if(end_y>map_main->sizey-1)end_y=map_main->sizey-1;

			bool found=false;

			for (a=start_x; a<end_x; a++){
				for (b=start_y; b<end_y; b++){
					for (c=0; c<map_main->grid[a].grid[b].total_creatures; c++){
						if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].dead)continue;
						if(condition.condition_parameter0>=0){
							if(map_main->creature[map_main->grid[a].grid[b].creatures[c]].type!=condition.condition_parameter0)continue;
						}
						else{
							if(mod.general_creatures[map_main->creature[map_main->grid[a].grid[b].creatures[c]].type].creature_class!=-condition.condition_parameter0)continue;
						}

						//if(mod.general_plot_objects[map_main->creature[map_main->grid[a].grid[b].creatures[c]].item_type].plot_object_class==condition.condition_parameter0){

						float size=mod.general_creatures[map_main->creature[map_main->grid[a].grid[b].creatures[c]].type].size*map_main->creature[map_main->grid[a].grid[b].creatures[c]].size*general_creature_size;
						//float size=map_main->creature[map_main->grid[a].grid[b].creatures[c]].size*general_object_size;
						float distance=sqr(x-(map_main->creature[map_main->grid[a].grid[b].creatures[c]].x+size*0.5f))+sqr(y-(map_main->creature[map_main->grid[a].grid[b].creatures[c]].y+size*0.5f));

						if(distance<sqr(size*0.5f+condition.condition_parameter1))
							found=true;

					}
				}
			}
			if(!found){
				if(show_message){
					tempstring="Must be near ";
					if(isvowel(mod.general_creatures[condition.condition_parameter0].name.at(0)))tempstring+="an ";
					else tempstring+="a ";
					tempstring+=mod.general_creatures[condition.condition_parameter0].name;
					tempstring+=".";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//15=player is (parameter1, 0=nearer, 1=farther) than parameter0 pixels
	case 15:
		{
			float distance=sqr(x-player_middle_x)+sqr(y-player_middle_y);
			if(condition.condition_parameter1==0)
			if(distance>sqr(condition.condition_parameter0)){
				return false;
			}
			if(condition.condition_parameter1==1)
			if(distance<sqr(condition.condition_parameter0)){
				return false;
			}
		}
	break;
	//16=player has parameter0 free weight
	case 16:
		{
			if(maximum_carry_weight-carry_weight<condition.condition_parameter0){
				if(show_message){
					tempstring="Must have at least ";
					tempstring+=(int)condition.condition_parameter0;
					tempstring+=" free weight.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//17=(parameter1, 0=is, 1=is not) on terrain parameter0
	case 17:
		{
			int grix_x=x/grid_size;
			int grix_y=y/grid_size;
			if(condition.condition_parameter1==0)
				if(map_main->grid[grix_x].grid[grix_y].terrain_type!=(int)condition.condition_parameter0){
					if(show_message){
						tempstring="Must be on ";
						tempstring+=mod.terrain_types[(int)condition.condition_parameter0].name;
						tempstring+=".";
						text_manager.message(3000,3000,tempstring);
					}
					return false;
				}
			if(condition.condition_parameter1==1)
				if(map_main->grid[grix_x].grid[grix_y].terrain_type==(int)condition.condition_parameter0){
					if(show_message){
						tempstring="Must not be on ";
						tempstring+=mod.terrain_types[(int)condition.condition_parameter0].name;
						tempstring+=".";
						text_manager.message(3000,3000,tempstring);
					}
					return false;
				}
		}
	break;
	//18=creature's bar parameter0's maximum amount is bigger than or equal to parameter1
	case 18:
		{
			//no such bar
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//not enough
			if(creature->bars[(int)condition.condition_parameter0].maximum<condition.condition_parameter1){
				if(show_message){
					tempstring="Must have bigger ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" capacity.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//19=creature's AI tactic parameter0 (0 or 1) is parameter1
	case 19:
		{
			if(creature->tactic[(int)condition.condition_parameter0]!=(int)condition.condition_parameter1){
				/*if(show_message){
					tempstring="Must have bigger ";
					//tempstring+=" (add code here!) ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" capacity for this to work.";
					text_manager.message(3000,3000,tempstring);
				}*/
				return false;
			}
		}
	break;
	//20=creature's anger level is (parameter0: 0=bigger than or equal to, 1=smaller than) parameter1
	case 20:
		{
			if(condition.condition_parameter0==0){
				if(creature->anger_level<condition.condition_parameter1){
					return false;
				}
			}
			if(condition.condition_parameter0==1){
				if(creature->anger_level>=condition.condition_parameter1){
					return false;
				}
			}
		}
	break;
	//21=creature (parameter1, 0=is, 1=is not) of side parameter0
	case 21:
		{
			if(condition.condition_parameter1==0){
				if(condition.condition_parameter0!=creature->side){
					return false;
				}
			}
			else if(condition.condition_parameter1==1){
				if(condition.condition_parameter0==creature->side){
					return false;
				}
			}
		}
	break;
	//22=creature is of (parameter1: 0=type, 1=class) parameter0
	case 22:
		{
			if(condition.condition_parameter1==0){
				if(condition.condition_parameter0!=creature->type){
					return false;
				}
			}
			else if(condition.condition_parameter1==1){
				if(condition.condition_parameter0!=mod.general_creatures[creature->type].creature_class){
					return false;
				}
			}
		}
	break;
	//23=creature (parameter0: 0=is, 1=is not) player
	case 23:
		{
			if(condition.condition_parameter0==0){
				if(creature_number!=0)return false;
			}
			else if(condition.condition_parameter0==1){
				if(creature_number==0)return false;
			}
		}
	break;
	//24=player's bar parameter0 is greater or equal to parameter1
	case 24:
		{
			//no such bar
			if(!map_main->creature[0].bars[(int)condition.condition_parameter0].active)return false;

			//not enough
			if(map_main->creature[0].bars[(int)condition.condition_parameter0].value<condition.condition_parameter1){
				if(show_message){
					tempstring="Must have more ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//25=player's bar parameter0 is smaller than parameter1
	case 25:
		{
			//no such bar
			if(!map_main->creature[0].bars[(int)condition.condition_parameter0].active)return false;

			//not enough
			if(map_main->creature[0].bars[(int)condition.condition_parameter0].value>=condition.condition_parameter1){
				if(show_message){
					tempstring="Must have less ";
					//tempstring+=" (add code here!) ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//26=creature is not of (parameter1: 0=type, 1=class) parameter0
	case 26:
		{
			if(condition.condition_parameter1==0){
				if(condition.condition_parameter0==creature->type){
					return false;
				}
			}
			else if(condition.condition_parameter1==1){
				if(condition.condition_parameter0==mod.general_creatures[creature->type].creature_class){
					return false;
				}
			}
		}
	break;
	//27=creature's last dialog (parameter1: 0=was, 1=was not) parameter0
	case 27:
		{
			if(condition.condition_parameter1==0){
				if(condition.condition_parameter0!=creature->dialog){
					return false;
				}
			}
			else if(condition.condition_parameter1==1){
				if(condition.condition_parameter0==creature->dialog){
					return false;
				}
			}
		}
	break;
	//28=it (parameter0: 0=is, 1=is not) raining
	case 28:
		{
			if(condition.condition_parameter1==0){
				if(map_main->rain_timer<=0){
					return false;
				}
			}
			else if(condition.condition_parameter1==1){
				if(map_main->rain_timer>0){
					return false;
				}
			}
		}
	break;
	//29=amount of parameter0 (-1=all) creatures in area is higher than or equal to parameter1
	case 29:
		{
			//find the amount of such creatures
			int amount=0;
			for(unsigned int a=0;a<map_main->creature.size();a++){
				if(map_main->creature[a].dead)continue;
				if(map_main->creature[a].killed)continue;

				if((condition.condition_parameter0<0)||(map_main->creature[a].type==condition.condition_parameter0))
					amount++;
			}


			if(amount<condition.condition_parameter1)return false;

		}
	break;
	//30=amount of parameter0 (-1=all) creatures in area is lower than parameter1
	case 30:
		{
			//find the amount of such creatures
			int amount=0;
			for(unsigned int a=0;a<map_main->creature.size();a++){
				if(map_main->creature[a].dead)continue;
				if(map_main->creature[a].killed)continue;

				if((condition.condition_parameter0<0)||(map_main->creature[a].type==condition.condition_parameter0))
					amount++;
			}


			if(amount>=condition.condition_parameter1)return false;

		}
	break;
	//31=amount of parameter0 class creatures in area is higher than or equal to parameter1
	case 31:
		{
			//find the amount of such creatures
			int amount=0;
			for(unsigned int a=0;a<map_main->creature.size();a++){
				if(map_main->creature[a].dead)continue;
				if(map_main->creature[a].killed)continue;

				if((condition.condition_parameter0<0)||(mod.general_creatures[map_main->creature[a].type].creature_class==condition.condition_parameter0))
					amount++;
			}

			if(amount<condition.condition_parameter1)
                return false;
		}
	break;
	//32=amount of parameter0 class creatures in area is lower than parameter1
	case 32:
		{
			//find the amount of such creatures
			int amount=0;
			for(unsigned int a=0;a<map_main->creature.size();a++){
				if(map_main->creature[a].dead)continue;
				if(map_main->creature[a].killed)continue;

				if((condition.condition_parameter0<0)||(mod.general_creatures[map_main->creature[a].type].creature_class==condition.condition_parameter0))
					amount++;
			}

			if(amount>=condition.condition_parameter1)
                return false;
		}
	break;
	//33=creature (parameter0, 0=is, 1=is not) being carried by another creature
	case 33:
		{
			//look through all creatures
			for(int a=0;a<map_main->creature.size();a++){
				if(map_main->creature[a].dead)continue;
				if(map_main->creature[a].killed)continue;

				if(map_main->creature[a].carried_creature==creature_number){
					if(condition.condition_parameter0==0)
						return true;
					if(condition.condition_parameter0==1)
						return false;
				}
			}

			if(condition.condition_parameter0==0)
				return false;
			if(condition.condition_parameter0==1)
				return true;

		}
	break;
	//34=creature (parameter0, 0=is, 1=is not) carrying another creature
	case 34:
		{
			//look through all creatures
			if(condition.condition_parameter0==0)
				if(creature->carried_creature==-1)
					return false;
			if(condition.condition_parameter0==1)
				if(creature->carried_creature>=0)
					return false;
		}
	break;
	//35=must be distance parameter1 from item parameter0
	case 35:
		{
			int start_x=static_cast<int>(x/grid_size)-(2+condition.condition_parameter1/128);
			int start_y=static_cast<int>(y/grid_size)-(2+condition.condition_parameter1/128);
			int end_x=static_cast<int>(x/grid_size)+(2+condition.condition_parameter1/128);
			int end_y=static_cast<int>(y/grid_size)+(2+condition.condition_parameter1/128);

			if(start_x<0)start_x=0;
			if(end_x>map_main->sizex-1)end_x=map_main->sizex-1;
			if(start_y<0)start_y=0;
			if(end_y>map_main->sizey-1)end_y=map_main->sizey-1;

			bool found=false;

			for (a=start_x; a<end_x; a++){
				for (b=start_y; b<end_y; b++){
					for (c=0; c<map_main->grid[a].grid[b].items.size(); c++){
						if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
						if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=1)continue;
						if(map_main->items[map_main->grid[a].grid[b].items[c]].item_type==condition.condition_parameter0){

							float size=mod.general_items[condition.condition_parameter0].size*general_object_size;
							float distance=sqr(x-(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f))+sqr(y-(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f));
							//make_particle(1,1,1000,(map_main->items[map_main->grid[a].grid[b].items[c]].x+size*0.5f),(map_main->items[map_main->grid[a].grid[b].items[c]].y+size*0.5f),0,0);
							//make_particle(1,1,1000,x,y,0,0);
							if(distance<sqr(condition.condition_parameter1))
								found=true;
						}
					}
				}
			}

			if(!found){
				if(show_message){
					tempstring="Must be near ";
					if(isvowel(mod.general_items[condition.condition_parameter0].name.at(0)))tempstring+="an ";
					else tempstring+="a ";
					tempstring+=mod.general_items[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);
				}
				return false;
			}
		}
	break;
	//36=creature's bar (parameter0) is (parameter1, 0=visible, 1=not visible)
	case 36:
		{
			if(condition.condition_parameter1==0){
				if(!mod.general_bars[(int)condition.condition_parameter0].visible)
					return false;
			}
			if(condition.condition_parameter1==1){
				if(mod.general_bars[(int)condition.condition_parameter0].visible)
					return false;
			}
		}
	break;
	//37=mouse button (parameter0, 0=left, 1=right, 2=middle) is (parameter1, 0=pressed, 1=not pressed)
	case 37:
		{
			//left
			if(condition.condition_parameter0==0){
				if((int)condition.condition_parameter1==0)
					if(!mouse_left)
						return false;
				if((int)condition.condition_parameter1==1)
					if(mouse_left)
						return false;
			}
			//right
			if(condition.condition_parameter0==1){
				if((int)condition.condition_parameter1==0)
					if(!mouse_right)
						return false;
				if((int)condition.condition_parameter1==1)
					if(mouse_right)
						return false;
			}
			//middle
			if(condition.condition_parameter0==2){
				if((int)condition.condition_parameter1==0)
					if(!mouse_middle)
						return false;
				if((int)condition.condition_parameter1==1)
					if(mouse_middle)
						return false;
			}

		}
	break;
	//38=creature's bar parameter0 is equal to parameter1 (rounded down)
	case 38:
		{
			//no such bar
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//not equal
			if((int)creature->bars[(int)condition.condition_parameter0].value!=(int)condition.condition_parameter1){
				if(show_message){
					/*tempstring="Must have ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);*/
				}
				return false;
			}
		}
	break;
	//39=creature's bar parameter0 is not equal to parameter1 (rounded down)
	case 39:
		{
			//no such bar
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//not equal
			if((int)creature->bars[(int)condition.condition_parameter0].value==(int)condition.condition_parameter1){
				if(show_message){
					/*tempstring="Must have more ";
					tempstring+=mod.general_bars[condition.condition_parameter0].name;
					tempstring+=" for this to work.";
					text_manager.message(3000,3000,tempstring);*/
				}
				return false;
			}
		}
	break;
	//40=creature's bar parameter0 is less than creature's bar parameter1
	case 40:
		{
			//no such bars
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;
			if(!creature->bars[(int)condition.condition_parameter1].active)return false;

			//compare
			if(creature->bars[(int)condition.condition_parameter0].value>=creature->bars[(int)condition.condition_parameter1].value){
				if(show_message){
				}
				return false;
			}
		}
	break;
	//41=creature's bar parameter0 is more than creature's bar parameter1
	case 41:
		{
			//no such bars
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;
			if(!creature->bars[(int)condition.condition_parameter1].active)return false;

			//compare
			if(creature->bars[(int)condition.condition_parameter0].value<=creature->bars[(int)condition.condition_parameter1].value){
				if(show_message){
				}
				return false;
			}
		}
	break;
	//42=creature's bar parameter0 is equal to creature's bar parameter1
	case 42:
		{
			//no such bars
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;
			if(!creature->bars[(int)condition.condition_parameter1].active)return false;

			//compare
			if(creature->bars[(int)condition.condition_parameter0].value!=creature->bars[(int)condition.condition_parameter1].value){
				if(show_message){
				}
				return false;
			}
		}
	break;
	//43=creature's bar parameter0 is smaller than parameter1 percent of maximum (value of 100=100%)
	case 43:
		{
			//no such bars
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//compare
			if((creature->bars[(int)condition.condition_parameter0].value/creature->bars[(int)condition.condition_parameter0].maximum)*100>=condition.condition_parameter1){
				if(show_message){
				}
				return false;
			}
		}
	break;
	//44=creature's bar parameter0 is more than parameter1 percent of maximum (value of 100=100%)
	case 44:
		{
			//no such bars
			if(!creature->bars[(int)condition.condition_parameter0].active)return false;

			//compare
			if((creature->bars[(int)condition.condition_parameter0].value/creature->bars[(int)condition.condition_parameter0].maximum)*100<=condition.condition_parameter1){
				if(show_message){
				}
				return false;
			}
		}
	break;
	case 45:
	//45=last weapon to hit creature was weapon type parameter0 (negative=weapon class), and the weapon hit in the previous parameter1 milliseconds
		{
		//weapon class
		if((int)condition.condition_parameter0<0){
			if(mod.general_weapons[creature->last_weapon_to_hit].weapon_class!=-(int)condition.condition_parameter0)
				return false;
		}
		//weapon type
		else{
			if(creature->last_weapon_to_hit!=(int)condition.condition_parameter0)
				return false;
		}
		//time
		if((time_from_beginning-creature->last_weapon_to_hit_time)*1000>condition.condition_parameter1)
			return false;

		}
	break;
	//46=must have item class parameter0 amount parameter1 (use 0 for checking if the player doesn't have such an item)
	case 46:
		{
			//find if we have such an item
			bool found=false;
			if(condition.condition_parameter1>0){
				for(a=0;a<inventory[active_inventory].player_items.size();a++){
					if((mod.general_items[inventory[active_inventory].player_items[a].item].item_class==condition.condition_parameter0)&&(inventory[active_inventory].player_items[a].amount>=condition.condition_parameter1)){
						found=true;
					}
				}
				//not found, return
				if(!found){
					if(show_message){
					}
					return false;
				}
			}
			//checking if there are no such items
			else if(condition.condition_parameter1==0){
				for(a=0;a<inventory[active_inventory].player_items.size();a++)
					if((mod.general_items[inventory[active_inventory].player_items[a].item].item_class==condition.condition_parameter0)&&(inventory[active_inventory].player_items[a].amount>0))
						found=true;
				if(found)return false;
			}
		}
	}

	return true;
}

void game_engine::draw_bars(void){

	unsigned int a;
	grim->System_SetState_Blending(true);

	//int anchor_x[4]={0,screen_width,0,screen_width};
	//int anchor_y[4]={0,0,screen_height,screen_height};
	int anchor_x[4]={0,1024,0,1024};
	int anchor_y[4]={0,0,768,768};
	int show_name=-1;



	for(a=0;a<maximum_bars;a++){
		if(a>=mod.general_bars.size())continue;
		if(mod.general_bars[a].dead)continue;

		if(!map_main->creature[0].bars[a].active)continue;
		if(!mod.general_bars[a].visible)continue;

		float left_side, right_side, y;


		//background picture
		if(mod.general_bars[a].background_picture>=0){
			resources.Texture_Set(mod.general_bars[a].background_picture);
			grim->Quads_SetSubset(0,0,1,1);
			grim->Quads_SetRotation(0);
			grim->Quads_SetColor(1,1,1,1.0f);
			grim->Quads_Begin();

			float x_point=mod.general_bars[a].location_x+anchor_x[mod.general_bars[a].anchor_point]+mod.general_bars[a].background_picture_x_offset;
			float y_point=mod.general_bars[a].location_y+anchor_y[mod.general_bars[a].anchor_point]+mod.general_bars[a].background_picture_y_offset;

			grim->Quads_Draw(x_point*x_multiplier, y_point*y_multiplier, mod.general_bars[a].background_picture_width*x_multiplier, mod.general_bars[a].background_picture_height*y_multiplier);

			grim->Quads_End();
		}



		//bar
		resources.Texture_Set(mod.general_bars[a].bar_picture);
		grim->Quads_Begin();

		float r_left;
		float g_left;
		float b_left;
		float r_right;
		float g_right;
		float b_right;


		//negative
		if(map_main->creature[0].bars[a].value<0){
			left_side=-map_main->creature[0].bars[a].value/map_main->creature[0].bars[a].minimum*mod.general_bars[a].length;
			if(map_main->creature[0].bars[a].minimum==0)left_side=map_main->creature[0].bars[a].value/map_main->creature[0].bars[a].maximum*mod.general_bars[a].length;
			right_side=0;

			//float r_middle=mod.general_bars[a].color_min_r+mod.general_bars[a].color_max_r;
			float position_left=(map_main->creature[0].bars[a].value-map_main->creature[0].bars[a].minimum)/(map_main->creature[0].bars[a].maximum-map_main->creature[0].bars[a].minimum);
			float position_right=(0-map_main->creature[0].bars[a].minimum)/(map_main->creature[0].bars[a].maximum-map_main->creature[0].bars[a].minimum);
			if(position_left<1)position_left=1;

			r_left=position_left*mod.general_bars[a].color_min_r+(1-position_left)*mod.general_bars[a].color_max_r;
			g_left=position_left*mod.general_bars[a].color_min_g+(1-position_left)*mod.general_bars[a].color_max_g;
			b_left=position_left*mod.general_bars[a].color_min_b+(1-position_left)*mod.general_bars[a].color_max_b;
			r_right=position_right*mod.general_bars[a].color_min_r+(1-position_right)*mod.general_bars[a].color_max_r;
			g_right=position_right*mod.general_bars[a].color_min_g+(1-position_right)*mod.general_bars[a].color_max_g;
			b_right=position_right*mod.general_bars[a].color_min_b+(1-position_right)*mod.general_bars[a].color_max_b;


		}

		//positive
		if(map_main->creature[0].bars[a].value>=0){
			left_side=0;
			right_side=map_main->creature[0].bars[a].value/map_main->creature[0].bars[a].maximum*mod.general_bars[a].length;
			if(map_main->creature[0].bars[a].maximum==0)right_side=map_main->creature[0].bars[a].value/map_main->creature[0].bars[a].minimum*mod.general_bars[a].length;

			float position_left=(0-map_main->creature[0].bars[a].minimum)/(map_main->creature[0].bars[a].maximum-map_main->creature[0].bars[a].minimum);
			float position_right=(map_main->creature[0].bars[a].value-map_main->creature[0].bars[a].minimum)/(map_main->creature[0].bars[a].maximum-map_main->creature[0].bars[a].minimum);

			if(position_right>1)position_right=1;


			r_left=(1-position_left)*mod.general_bars[a].color_min_r+(position_left)*mod.general_bars[a].color_max_r;
			g_left=(1-position_left)*mod.general_bars[a].color_min_g+(position_left)*mod.general_bars[a].color_max_g;
			b_left=(1-position_left)*mod.general_bars[a].color_min_b+(position_left)*mod.general_bars[a].color_max_b;
			r_right=(1-position_right)*mod.general_bars[a].color_min_r+(position_right)*mod.general_bars[a].color_max_r;
			g_right=(1-position_right)*mod.general_bars[a].color_min_g+(position_right)*mod.general_bars[a].color_max_g;
			b_right=(1-position_right)*mod.general_bars[a].color_min_b+(position_right)*mod.general_bars[a].color_max_b;


		}

		if(left_side>right_side){
			float temp=left_side;
			left_side=right_side;
			right_side=temp;

			float temp_r=r_left;
			float temp_g=g_left;
			float temp_b=b_left;
			r_left=r_right;
			g_left=g_right;
			b_left=b_right;
			r_right=temp_r;
			g_right=temp_g;
			b_right=temp_b;

		}





		if(left_side<-1*fabs(mod.general_bars[a].length))left_side=-1*fabs(mod.general_bars[a].length);
		if(right_side>1*fabs(mod.general_bars[a].length))right_side=1*fabs(mod.general_bars[a].length);
		y=mod.general_bars[a].location_y+anchor_y[mod.general_bars[a].anchor_point];
		left_side+=mod.general_bars[a].location_x+anchor_x[mod.general_bars[a].anchor_point];
		right_side+=mod.general_bars[a].location_x+anchor_x[mod.general_bars[a].anchor_point];

		//grim->Quads_SetColor(1,0,0,0.7f);
		left_side=left_side*x_multiplier;
		right_side=right_side*x_multiplier;
		y=y*y_multiplier;


		//middle part
		grim->Quads_SetColorVertex(1, r_right,g_right,b_right, 1);
		grim->Quads_SetColorVertex(0, r_left,g_left,b_left, 1);
		grim->Quads_SetColorVertex(3, r_left,g_left,b_left, 1);
		grim->Quads_SetColorVertex(2, r_right,g_right,b_right, 1);
		grim->Quads_SetSubset((7/16.0f),(0/16.0f),(9/16.0f),(16/16.0f));
		grim->Quads_Draw(left_side, y, right_side-left_side, 16*mod.general_bars[a].height*y_multiplier);
		//left side
		grim->Quads_SetColor(r_left,g_left,b_left,1.0f);
		grim->Quads_SetSubset((0/16.0f),(0/16.0f),(7/16.0f),(16/16.0f));
		grim->Quads_Draw(left_side-7, y, 7, 16*mod.general_bars[a].height*y_multiplier);
		//right side
		grim->Quads_SetColor(r_right,g_right,b_right,1.0f);
		grim->Quads_SetSubset((9/16.0f),(0/16.0f),(16/16.0f),(16/16.0f));
		grim->Quads_Draw(right_side, y, 7, 16*mod.general_bars[a].height*y_multiplier);

		grim->Quads_End();

		//draw number
		if(mod.general_bars[a].show_number){
			//itoa(map_main->creature[0].bars[a].value,temprivi,10)
			sprintf(temprivi,"%d",(int)(map_main->creature[0].bars[a].value));
			text_manager.write(font,temprivi,1.0f*x_multiplier,left_side, y ,0,0,false,1.0f,1.0f,1.0f,1);
		}

		//mouse on, show name
		if((mousex>left_side-7)&&(mousex<right_side+7)&&(mousey>y)&&(mousey<y+16*mod.general_bars[a].height*y_multiplier)){
			show_name=a;
			//text_manager.write(font,mod.general_bars[a].name,1.0f,mousex+20, mousey-10 ,screen_height,screen_width,false,1.0f,1.0f,1.0f,1);
		}

	}

	if(show_name>=0){
		float x=mousex+20;
		float y=mousey-8;
		if(x+mod.general_bars[show_name].name.length()*10>screen_width)x=screen_width-mod.general_bars[show_name].name.length()*10;
		text_manager.write(font,mod.general_bars[show_name].name,1.0f,x, y ,screen_width,screen_height,false,1.0f,1.0f,1.0f,1);
	}

}

void game_engine::count_bars(void){
	unsigned int a,b,c;

	for(a=0;a<maximum_bars;a++){

		if(a>=mod.general_bars.size())continue;
		if(mod.general_bars[a].dead)continue;
		if(!map_main->creature[0].bars[a].active)continue;

		//some bars show the values of other counters
		switch(mod.general_bars[a].bar_type){
			//1=show body temperature
			case 1:
				map_main->creature[0].bars[a].value=body_temperature*100;
				break;
			//2=show number of items parameter0
			case 2:
				map_main->creature[0].bars[a].value=0;
				for(b=0;b<inventory[active_inventory].player_items.size();b++){
					if((inventory[active_inventory].player_items[b].item==mod.general_bars[a].parameter0)&&(inventory[active_inventory].player_items[b].amount>0)){
						map_main->creature[0].bars[a].value=inventory[active_inventory].player_items[a].amount;
						break;
					}
				}
				break;
			//3=show wielded weapon ammo
			case 3:
				map_main->creature[0].bars[a].value=0;
				for(c=0;c<mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].fire_effects.size();c++){
					if(mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].fire_effects[c].effect_number==16){
						for(b=0;b<inventory[active_inventory].player_items.size();b++){
							if((inventory[active_inventory].player_items[b].item==mod.general_weapons[mod.general_creatures[map_main->creature[0].type].weapon].fire_effects[c].parameter1)&&(inventory[active_inventory].player_items[b].amount>0)){
								map_main->creature[0].bars[a].value=inventory[active_inventory].player_items[a].amount;
								break;
							}
						}
						break;
					}
				}



				break;
			//4=show armor
			case 4:
				map_main->creature[0].bars[a].value=armor;
				break;

			//5=show carry weight
			case 5:
				map_main->creature[0].bars[a].value=carry_weight;
				break;

			//6=show seconds from game start
			case 6:
				map_main->creature[0].bars[a].value=time_from_beginning;
				break;
		}

	}


	//bars reduced by player race specials
	for(a=0;a<mod.general_races[player_race].specialties.size();a++){
		if(!mod.general_races[player_race].specialties[a].difficulty[game_difficulty_level])continue;

		//6=if bar parameter0 < parameter1 percent, increase bar parameter2 by parameter3
		if(mod.general_races[player_race].specialties[a].number==6){
			//check condition
			if(!map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].active)continue;
			if(map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].value-map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].minimum<mod.general_races[player_race].specialties[a].parameter1*(map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].maximum-map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].minimum)){

				//increase the bar
				map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter2].value+=mod.general_races[player_race].specialties[a].parameter3*elapsed*game_speed*0.001f;

				//message
				if(mod.general_races[player_race].specialties[a].message!="none")
					text_manager.message(1000,1000,mod.general_races[player_race].specialties[a].message);

				//play sample
				/*
				if(game_speed>0)
					if((play_sound)&&(sound_initialized))
					if(!sample[cold_sound]->IsSoundPlaying())
						playsound(cold_sound,1,0,0,0,0);*/
			}

		}

		//7=if bar parameter0 > parameter1 percent, increase bar parameter2 by parameter3
		if(mod.general_races[player_race].specialties[a].number==7){
			//check condition
			if(!map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].active)continue;
			if(map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].value-map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].minimum>=mod.general_races[player_race].specialties[a].parameter1*(map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].maximum-map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter0].minimum)){

				//increase the bar
				map_main->creature[0].bars[(int)mod.general_races[player_race].specialties[a].parameter2].value+=mod.general_races[player_race].specialties[a].parameter3*elapsed*game_speed*0.001f;

				//message
				if(mod.general_races[player_race].specialties[a].message!="none")
					text_manager.message(1000,1000,mod.general_races[player_race].specialties[a].message);

				//play sample
				/*
				if(game_speed>0)
					if((play_sound)&&(sound_initialized))
					if(!sample[cold_sound]->IsSoundPlaying())
						playsound(cold_sound,1,0,0,0,0);*/
			}

		}
	}
}

void game_engine::calculate_scripts(void){

	int script_number;

	for(script_number=0;script_number<mod.scripts.size();script_number++){
		if(mod.scripts[script_number].dead) continue;
		if(script_info[script_number].dead) continue;
		if(!mod.scripts[script_number].run_without_calling) continue;
        run_script(script_number,true,true);
	}
}

void game_engine::run_script(int script_number, bool check_conditions, bool check_time){

		tempstring=mod.scripts[script_number].name;

		//calling creature
		int caller=-1;
		//0=player
		if(mod.scripts[script_number].calling_creature==0)
			caller=0;
		//1=creature nearest to the mouse
		if(mod.scripts[script_number].calling_creature==1)
			caller=creature_nearest_to_the_mouse;
		//2=player controlled creature
		if(mod.scripts[script_number].calling_creature==2)
			caller=player_controlled_creature;
		if(caller==-1)return;

		//call position
		//0=player location
		float call_x=player_middle_x;
		float call_y=player_middle_y;
		//1=mouse location
		if(mod.scripts[script_number].calling_position==1){
			call_x=camera_x+mousex;
			call_y=camera_y+mousey;
		}


		if(check_time){
			int time_increment=(time_from_beginning-script_info[script_number].script_calculated_on)*1000;

			if(time_increment<mod.scripts[script_number].interval)return;
			script_info[script_number].script_calculated_on=time_from_beginning;

			//if we're already counting down the execution, no need to check the conditions
			if((script_info[script_number].timer<=0)&&(check_conditions)){

				//go throught all conditions
				bool OK=true;
				for(unsigned int b=0;b<mod.scripts[script_number].conditions.size();b++){
					if(!check_condition(mod.scripts[script_number].conditions[b],&map_main->creature[caller],caller,call_x,call_y,false)){
						OK=false;
						break;
					}
				}
				if (!OK) return;
			}

			//we have a delay
			if(mod.scripts[script_number].delay>script_info[script_number].timer){
				script_info[script_number].timer+=time_increment*game_speed*0.001f;
				return;
			}

			//reset timer
			script_info[script_number].timer=0;
		}
		else{
			//go throught all conditions
			bool OK=true;
			for(unsigned int b=0;b<mod.scripts[script_number].conditions.size();b++){
				if(!check_condition(mod.scripts[script_number].conditions[b],&map_main->creature[caller],caller,call_x,call_y,false)){
					OK=false;
					break;
				}
			}
			if(!OK)return;
		}



		//let's run the effects
		for(unsigned int b=0;b<mod.scripts[script_number].effects.size();b++){
			run_effect(mod.scripts[script_number].effects[b],&map_main->creature[caller],caller,call_x,call_y,map_main->creature[caller].rotation,false);
		}

		//message
		if(mod.scripts[script_number].message!="none"){
			//show normal message
			if(mod.scripts[script_number].message_type==0){
				text_manager.message(5000,2000,mod.scripts[script_number].message);
			}
			//show journal entry
			else{
				record_message(4,script_number);
				if(show_journals)show_text_window(journal_records.size()-1);
			}
		}

		//play the sound
		playsound(mod.scripts[script_number].sound,1,0,0,0,0);

		//disable the script
		if(mod.scripts[script_number].disable_after_first_use)
			script_info[script_number].dead=true;

}

void game_engine::set_bar(creature_base *creature, unsigned int bar, float value){
	creature->bars[bar].value=value;

	//bar types, some bars may affect the values they show
	if(bar<mod.general_bars.size()){
		switch(mod.general_bars[bar].bar_type){
			case 1:
				body_temperature=creature->bars[bar].value*0.01f;
				break;
			case 4:
				armor=(int)creature->bars[bar].value;
				break;
			case 6:
				time_from_beginning=(int)creature->bars[bar].value;
				break;
		}
	}
}

void game_engine::carry_light(map *edit_map, creature_base *creature, int light){

	float size=mod.general_creatures[creature->type].size*creature->size*general_creature_size;

	switch(mod.general_lights[edit_map->lights[light].type].type){

	//flashlight
	case 0:
		//hands
		if(creature->light_attached_to==0)
			edit_map->lights[light].rotation=creature->rotation;
		//legs
		if(creature->light_attached_to==1)
			edit_map->lights[light].rotation=creature->rotation_legs;
		//head
		if(creature->light_attached_to==2)
			edit_map->lights[light].rotation=creature->rotation_head;

		edit_map->lights[light].x=creature->x-edit_map->lights[light].size*64+size*0.5f-sincos.table_cos(edit_map->lights[light].rotation+pi/2)*64*edit_map->lights[light].size;
		edit_map->lights[light].y=creature->y-edit_map->lights[light].size*64+size*0.5f-sincos.table_sin(edit_map->lights[light].rotation+pi/2)*64*edit_map->lights[light].size;
		edit_map->lights[light].x2=creature->x+size*0.5f;
		edit_map->lights[light].y2=creature->y+size*0.5f;
	break;

	//omni
	case 1:
		edit_map->lights[light].rotation=0;
		edit_map->lights[light].x=creature->x+size*0.5f-edit_map->lights[light].size*64;
		edit_map->lights[light].y=creature->y+size*0.5f-edit_map->lights[light].size*64;
		edit_map->lights[light].x2=creature->x+size*0.5f;
		edit_map->lights[light].y2=creature->y+size*0.5f;
	break;

	}
}


void game_engine::delete_light(map *edit_map, int light){

	int a;

	if(light<0)return;
	if(light>=edit_map->lights.size())return;

	edit_map->lights.erase(edit_map->lights.begin() + light);


	//correct all referrers
	for(a=0;a<edit_map->creature.size();a++){

		if(edit_map->creature[a].dead)continue;

		if(edit_map->creature[a].carry_light>light)
			edit_map->creature[a].carry_light--;

		else if(edit_map->creature[a].carry_light==light)
			edit_map->creature[a].carry_light=-1;
	}

}


void game_engine::delete_creature(map *edit_map, int creature){

	int a;

	if(creature<0)return;
	if(creature>=edit_map->creature.size())
		return;


	edit_map->creature.erase(edit_map->creature.begin() + creature);


	if(creature<player_controlled_creature)
		player_controlled_creature--;

	//correct all referrers
	for(a=0;a<edit_map->creature.size();a++){

		//if(edit_map->creature[a].dead)continue;

		if(edit_map->creature[a].carried_creature>creature)
			edit_map->creature[a].carried_creature--;

		else if(edit_map->creature[a].carried_creature==creature)
			edit_map->creature[a].carried_creature=-1;
	}

	edit_map->check_creatures();
}



bool game_engine::run_plot_object(int item){
	//for all effects
	bool must_delete=false;

	float temp_angle=map_main->creature[0].rotation;
	map_main->creature[0].rotation=map_main->items[item].rotation;

	int a,b;
	for(a=0;a<mod.general_plot_objects[map_main->items[item].item_type].effects.size();a++){
		Mod::effect_base effect=mod.general_plot_objects[map_main->items[item].item_type].effects[a];

		float size=map_main->items[item].size*general_object_size;

		//go throught all conditions
		bool OK=true;
		for(b=0;b<mod.general_plot_objects[map_main->items[item].item_type].effects[a].conditions.size();b++){
			if(!check_condition(mod.general_plot_objects[map_main->items[item].item_type].effects[a].conditions[b],&map_main->creature[0],0,map_main->items[item].x+size*0.5f,map_main->items[item].y+size*0.5f,mod.general_plot_objects[map_main->items[item].item_type].show_condition_help)){
				OK=false;
				break;
			}
		}
		if(!OK)continue;

		//run effects
		float temp_x2=map_main->creature[0].x2;
		float temp_y2=map_main->creature[0].y2;
		map_main->creature[0].x2=map_main->creature[0].x;
		map_main->creature[0].y2=map_main->creature[0].y;
		bool effect_ran=true;
		for(b=0;b<mod.general_plot_objects[map_main->items[item].item_type].effects[a].effects.size();b++){
			if(!run_effect(effect.effects[b],&map_main->creature[0],0,map_main->items[item].x+size*0.5f,map_main->items[item].y+size*0.5f,map_main->items[item].rotation,false)){
				//effect_ran=false;
				break;
			}
		}
		map_main->creature[0].x2=temp_x2;
		map_main->creature[0].y2=temp_y2;

		//all effects ran OK
		if(effect_ran){

			//play the sound
			playsound(effect.sound,1,map_main->items[item].x+size*0.5f,map_main->items[item].y+size*0.5f,player_middle_x,player_middle_y);

			//show text for first time use
			if(!seen_plot_object_text[map_main->items[item].item_type]){
				if(effect.event_text!="none"){
					record_message(1,map_main->items[item].item_type);
					seen_plot_object_text[map_main->items[item].item_type]=true;
					if(show_journals)show_text_window(journal_records.size()-1);
				}
			}
			//delete item if needed
			if((effect.vanish_after_used==0)||(effect.vanish_after_used==1)){//cannot be reused
				map_main->items[item].event_used=true;
				//only play sound when the event cannot be reused constantly
				//playsound(effect.sound,1,0,0,0,0);
			}
			if(effect.vanish_after_used==1){
				must_delete=true;
				//delete_plot_object(map_main,item, i, j, k);
				//k--;
			}
		}

		//some effect failed
		else{
			if(effect.event_failure_text!="none"){
				text_manager.message(5000,2000,effect.event_failure_text);
			}
		}
	}

	map_main->creature[0].rotation=temp_angle;
	return must_delete;
}


bool game_engine::creature_in_object(const creature_base *creature, const map_object *object){


	float bound_circle=14;
	float creature_size=(bound_circle)*(creature->size*mod.general_creatures[creature->type].size);
	float creature_x=creature->x+creature_size*0.5f/(bound_circle)*general_creature_size;
	float creature_y=creature->y+creature_size*0.5f/(bound_circle)*general_creature_size;

	switch(mod.general_objects[object->type].collision_type){

	//0=circle radius parameter0
	case 0:
		if(mod.general_objects[object->type].collision_parameter0>0){

			float object_size=mod.general_objects[object->type].collision_parameter0*object->size*general_object_size;
			float object_x=object->x+object_size*0.5f/mod.general_objects[object->type].collision_parameter0;
			float object_y=object->y+object_size*0.5f/mod.general_objects[object->type].collision_parameter0;

			if(sqr(creature_x-object_x)+sqr(creature_y-object_y)<sqr(creature_size*0.5f+object_size*0.5f)){
				return true;
			}
		}
		break;

	//1=polygon parameter0 from polygons.dat size parameter1
	case 1:
		float object_size=object->size*general_object_size;
		float object_x=object->x+object_size*0.5f;
		float object_y=object->y+object_size*0.5f;


		//all the lines
		int a;

		float sin=sincos.table_sin(object->rotation);
		float cos=sincos.table_cos(object->rotation);

		bool collision_found=false;
		bool c=false;
		for(a=0;a<mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()-1;a++){
			float xx1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].x*object_size*mod.general_objects[object->type].collision_parameter1;
			float yy1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].y*object_size*mod.general_objects[object->type].collision_parameter1;
			float xx2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].x*object_size*mod.general_objects[object->type].collision_parameter1;
			float yy2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].y*object_size*mod.general_objects[object->type].collision_parameter1;

			float x1=cos*xx1+sin*yy1+object_x;
			float y1=sin*xx1-cos*yy1+object_y;
			float x2=cos*xx2+sin*yy2+object_x;
			float y2=sin*xx2-cos*yy2+object_y;

			if ((((y1 <= creature_y) && (creature_y < y2)) ||
				 ((y2 <= creature_y) && (creature_y < y1))) &&
				(creature_x < (x2 - x1) * (creature_y - y1) / (y2 - y1) + x1))
			  c = !c;
		}
		return c;

		break;
	}

	return false;
}

bool game_engine::point_in_object(float x, float y, const map_object *object){

	int test_type=mod.general_objects[object->type].collision_type;
	float size=mod.general_objects[object->type].collision_parameter0;
	if(test_type<0){
		test_type=0;
		size=0.5f;
	}

	switch(test_type){

	//0=circle radius parameter0
	case 0:
		if(size>0){

			float object_size=object->size*general_object_size;
			float object_x=object->x+object_size*0.5f;
			float object_y=object->y+object_size*0.5f;

			if(sqr(x-object_x)+sqr(y-object_y)<sqr(size*object_size*0.5f)){
				return true;
			}
		}
		break;

	//1=polygon parameter0 from polygons.dat size parameter1
	case 1:
		float object_size=object->size*general_object_size;
		float object_x=object->x+object_size*0.5f;
		float object_y=object->y+object_size*0.5f;

		//all the lines
		int a;

		float sin=sincos.table_sin(object->rotation);
		float cos=sincos.table_cos(object->rotation);

		bool collision_found=false;
		bool c=false;
		for(a=0;a<mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()-1;a++){
			float xx1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].x*object_size*mod.general_objects[object->type].collision_parameter1;
			float yy1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].y*object_size*mod.general_objects[object->type].collision_parameter1;
			float xx2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].x*object_size*mod.general_objects[object->type].collision_parameter1;
			float yy2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].y*object_size*mod.general_objects[object->type].collision_parameter1;

			float x1=cos*xx1+sin*yy1+object_x;
			float y1=sin*xx1-cos*yy1+object_y;
			float x2=cos*xx2+sin*yy2+object_x;
			float y2=sin*xx2-cos*yy2+object_y;

			if ((((y1 <= y) && (y < y2)) ||
				 ((y2 <= y) && (y < y1))) &&
				(x < (x2 - x1) * (y - y1) / (y2 - y1) + x1))
			  c = !c;

		}
		return c;

		break;
	}

	return false;
}



bool game_engine::creature_collision_detection(creature_base *creature, const map_object *object, bool correct_place){

	if(mod.general_objects[object->type].collision_type==-1)return false;

	float bound_circle=14;
	float creature_size=(bound_circle)*(creature->size*mod.general_creatures[creature->type].size);
	float creature_x=creature->x+creature_size*0.5f/(bound_circle)*general_creature_size;
	float creature_y=creature->y+creature_size*0.5f/(bound_circle)*general_creature_size;
	float creature_x2=creature->x2+creature_size*0.5f/(bound_circle)*general_creature_size;
	float creature_y2=creature->y2+creature_size*0.5f/(bound_circle)*general_creature_size;

	switch(mod.general_objects[object->type].collision_type){

	//0=circle radius parameter0
	case 0:
		if(mod.general_objects[object->type].collision_parameter0>0){

			float object_size=mod.general_objects[object->type].collision_parameter0*object->size*general_object_size;
			float object_x=object->x+object_size*0.5f/mod.general_objects[object->type].collision_parameter0;
			float object_y=object->y+object_size*0.5f/mod.general_objects[object->type].collision_parameter0;

			if(sqr(creature_x-object_x)+sqr(creature_y-object_y)<sqr(creature_size*0.5f+object_size*0.5f)){

				//correct the place
				if(correct_place){
					float kerroin=sqrtf(sqr(creature_size*0.5f+object_size*0.5f*1.001f)/(sqr(creature_x-object_x)+sqr(creature_y-object_y)));
					creature->x+=(creature_x-object_x)*kerroin-(creature_x-object_x);
					creature->y+=(creature_y-object_y)*kerroin-(creature_y-object_y);
                    assert(!isnan(creature->x));
                    assert(!isnan(creature->y));

					//see if we can use the new point
					if(creature_will_collide(map_main,creature)){
						//must revert to old place
						creature->x=creature->x2;
						creature->y=creature->y2;
					}
				}
				return true;
			}
		}
		break;

	//1=polygon parameter0 from polygons.dat size parameter1
	case 1:
		bool collision=false;
		if(creature_in_object(creature,object)){
			collision=true;
		}
		if(!correct_place){
			return collision;
		}
		//we must find a spot outside the polygon
		else{
			if(!collision)return false;

			//find the line on which the new spot will be
			float object_size=object->size*general_object_size;
			float object_x=object->x+object_size*0.5f;
			float object_y=object->y+object_size*0.5f;

			float line2_x1=creature_x;//creature_x+(creature_x-object_x)*10;
			float line2_y1=creature_y;//creature_y+(creature_y-object_y)*10;
			float line2_x2=creature_x2+(creature_x2-creature_x)*20;//object_x
			float line2_y2=creature_y2+(creature_y2-creature_y)*20;//object_y

			//make_particle(7,1,1000,through_point_x,through_point_y,0,0);
			//make_particle(7,1,1000,object_x,object_y,0,0);

			//go through all lines
			point2d closest_hit;
			float wall_x1,wall_y1,wall_x2,wall_y2;
			int collision_line=-1;
			float sin=sincos.table_sin(object->rotation);
			float cos=sincos.table_cos(object->rotation);

			float closest_distance=0;
			for(unsigned int a=0;a<mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()-1;a++){
				//float bound_size=1.001f;
				float xx1=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a].x*object_size*mod.general_objects[object->type].collision_parameter1;
				float yy1=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a].y*object_size*mod.general_objects[object->type].collision_parameter1;
				float xx2=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a+1].x*object_size*mod.general_objects[object->type].collision_parameter1;
				float yy2=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a+1].y*object_size*mod.general_objects[object->type].collision_parameter1;

				float x1=cos*xx1+sin*yy1+object_x;
				float y1=sin*xx1-cos*yy1+object_y;
				float x2=cos*xx2+sin*yy2+object_x;
				float y2=sin*xx2-cos*yy2+object_y;

				//make_particle(7,1,1000,x1,y1,0,0);
				//make_particle(7,1,1000,x2,y2,0,0);

				//check if they intersect
				point2d hit;
				if(lines_intersect(x1,y1,x2,y2,line2_x1,line2_y1,line2_x2,line2_y2,&hit.x,&hit.y,0.001f)==1){

					//make_particle(7,1,1000,hit.x,hit.y,0,0);

					//see if this is the closest point
					float distance=sqr(hit.x-creature_x)+sqr(hit.y-creature_y);
					if((distance<closest_distance)||(collision_line==-1)){
						collision_line=a;
						closest_distance=distance;
						closest_hit=hit;
						wall_x1=x1;
						wall_y1=y1;
						wall_x2=x2;
						wall_y2=y2;
					}
				}
			}


			//find new point
			if(collision_line>=0){

				//project current position to line
				float new_x,new_y;
				{
					float kx=wall_x1-wall_x2;
					float ky=wall_y1-wall_y2;

					float xx1=creature_x+ky*1;
					float yy1=creature_y-kx*1;
					float xx2=creature_x-ky*1;
					float yy2=creature_y+kx*1;


					/*make_particle(7,1,1000,xx1,yy1,0,0);
					make_particle(7,1,1000,xx2,yy2,0,0);
					make_particle(7,1,1000,wall_x1,wall_y1,0,0);
					make_particle(7,1,1000,wall_x2,wall_y2,0,0);*/

					point2d hit;
					if(lines_intersect(xx1,yy1,xx2,yy2,wall_x1,wall_y1,wall_x2,wall_y2,&hit.x,&hit.y,0.0001f)==1){
						new_x=hit.x;
						new_y=hit.y;
					}
				}



				creature->x=new_x-creature_size*0.5f/(bound_circle)*general_creature_size;
				creature->y=new_y-creature_size*0.5f/(bound_circle)*general_creature_size;

				//see if we can use the new point
				if(creature_will_collide(map_main,creature)){
					//must revert to old place
					creature->x=creature->x2;
					creature->y=creature->y2;
				}
			}

			//we've been inside the object for two frames, we must locate a new place outside the object
			else{


				//try to revert to old place
				creature->x=creature->x2;
				creature->y=creature->y2;
				if(creature_will_collide(map_main,creature)){
					//old place is no good either, we must find a new one
					float jump_distance=10;
					vector <jump_point> jump_points;
					//find closest point on lines in polygon
					for(unsigned int a=0;a<mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()-1;a++){
						//float bound_size=1.001f;
						float xx1=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a].x*object_size*mod.general_objects[object->type].collision_parameter1;
						float yy1=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a].y*object_size*mod.general_objects[object->type].collision_parameter1;
						float xx2=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a+1].x*object_size*mod.general_objects[object->type].collision_parameter1;
						float yy2=mod.polygons[mod.general_objects[object->type].collision_parameter0].grown_points[a+1].y*object_size*mod.general_objects[object->type].collision_parameter1;

						float x1=cos*xx1+sin*yy1+object_x;
						float y1=sin*xx1-cos*yy1+object_y;
						float x2=cos*xx2+sin*yy2+object_x;
						float y2=sin*xx2-cos*yy2+object_y;

						for(int b=0;b<20;b++){
							line2_x2=line2_x1+randDouble(-100,100);
							line2_y2=line2_y1+randDouble(-100,100);


							point2d hit;
							if(lines_intersect(x1,y1,x2,y2,line2_x1,line2_y1,line2_x2,line2_y2,&hit.x,&hit.y,0.001f)==1){


								jump_point temp_point;
								temp_point.x=hit.x-creature_size*0.5f/(bound_circle)*general_creature_size;
								temp_point.y=hit.y-creature_size*0.5f/(bound_circle)*general_creature_size;
								temp_point.distance=sqr(creature_x-hit.x)+sqr(creature_y-hit.y);
								//make_particle(7,1,1000,hit.x,hit.y,0,0);
								jump_points.push_back(temp_point);
							}
						}
					}

					//order them by distance
					std::sort(jump_points.begin(),jump_points.end());

					//try the points out one by one
					for(unsigned int a=0;a<jump_points.size();a++){
						creature->x=jump_points[a].x;
						creature->y=jump_points[a].y;

						//see if we can use the new point
						if(creature_will_collide(map_main,creature)){
							//must revert to old place
							creature->x=creature->x2;
							creature->y=creature->y2;
						}
						//new place found
						else{
							break;
						}
					}
				}
			}
			return true;
		}
		break;
	}
	return false;
}


vector <point2d> game_engine::line_will_collide(float x1, float y1, float x2, float y2, bool return_on_first, bool avoid_terrain, bool only_ones_that_hinder_visibility, bool only_ones_that_stop_bullets, int check_area, bool check_props, bool check_plot_objects){
	vector <point2d> hits;
	int a,b,c;
	//float bound_circle=14;

	float temp_x1=x1;
	float temp_y1=y1;
	float temp_x2=x2;
	float temp_y2=y2;
	if(x1>x2){temp_x1=x2;temp_x2=x1;}
	if(y1>y2){temp_y1=y2;temp_y2=y1;}

	int alku_x=(int)(temp_x1/(general_object_size))-check_area;
	int alku_y=(int)(temp_y1/(general_object_size))-check_area;
	int loppu_x=(int)(temp_x2/(general_object_size))+check_area;
	int loppu_y=(int)(temp_y2/(general_object_size))+check_area;

	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

	for(a=alku_x;a<loppu_x;a++){
		for(b=alku_y;b<loppu_y;b++){
			vector <point2d> temp_hits;
			//props
			if(check_props)
			for(c=0;c<map_main->grid[a].grid[b].objects.size();c++){
				if(map_main->object[map_main->grid[a].grid[b].objects[c]].dead)continue;
				if(only_ones_that_hinder_visibility)
					if(!mod.general_objects[map_main->object[map_main->grid[a].grid[b].objects[c]].type].blocks_vision)continue;
				if(only_ones_that_stop_bullets)
					if(!mod.general_objects[map_main->object[map_main->grid[a].grid[b].objects[c]].type].stops_bullets)continue;
				temp_hits=line_collision_detection(x1,y1,x2,y2,&map_main->object[map_main->grid[a].grid[b].objects[c]],return_on_first);
				for(int d=0;d<temp_hits.size();d++){
					hits.push_back(temp_hits[d]);
					if(return_on_first)
						return hits;
				}

			}
			//plot objects
			if(check_plot_objects)
			for(c=0;c<map_main->grid[a].grid[b].items.size();c++){
				if(map_main->items[map_main->grid[a].grid[b].items[c]].dead)continue;
				if(map_main->items[map_main->grid[a].grid[b].items[c]].type >= mod.general_objects.size())continue;
				if(only_ones_that_hinder_visibility)
					if(!mod.general_objects[map_main->items[map_main->grid[a].grid[b].items[c]].type].blocks_vision)continue;
				if(only_ones_that_stop_bullets)
					if(!mod.general_objects[map_main->items[map_main->grid[a].grid[b].items[c]].type].stops_bullets)continue;
				if(map_main->items[map_main->grid[a].grid[b].items[c]].base_type!=0)continue;
				temp_hits=line_collision_detection(x1,y1,x2,y2,&map_main->items[map_main->grid[a].grid[b].items[c]],return_on_first);
				for(int d=0;d<temp_hits.size();d++){
					hits.push_back(temp_hits[d]);
					if(return_on_first)
						return hits;
				}

			}
			//AI_avoid terrain
			if(avoid_terrain){
				temp_hits=line_hazardous_terrain(x1,y1,x2,y2);
				for(int d=0;d<temp_hits.size();d++){
					hits.push_back(temp_hits[d]);
					if(return_on_first)
						return hits;
				}
			}

		}
	}

	return hits;
}

vector <point2d> game_engine::line_hazardous_terrain(float x1, float y1, float x2, float y2){

	vector <point2d> hits;
	int a,b;
	//float bound_circle=14;

	float temp_x1=x1;
	float temp_y1=y1;
	float temp_x2=x2;
	float temp_y2=y2;
	if(x1>x2){temp_x1=x2;temp_x2=x1;}
	if(y1>y2){temp_y1=y2;temp_y2=y1;}

	int alku_x=(int)(temp_x1/(general_object_size))-1;
	int alku_y=(int)(temp_y1/(general_object_size))-1;
	int loppu_x=(int)(temp_x2/(general_object_size))+1;
	int loppu_y=(int)(temp_y2/(general_object_size))+1;

	if(alku_x<0)alku_x=0;
	if(loppu_x>map_main->sizex-1)loppu_x=map_main->sizex-1;
	if(alku_y<0)alku_y=0;
	if(loppu_y>map_main->sizey-1)loppu_y=map_main->sizey-1;

	for(a=alku_x;a<loppu_x;a++){
		for(b=alku_y;b<loppu_y;b++){
			vector <point2d> temp_hits;
			if(mod.terrain_types[map_main->grid[a].grid[b].terrain_type].AI_avoid){
				point2d hit;
				float xx1=a*general_object_size;
				float yy1=b*general_object_size;
				float xx2=a*general_object_size+general_object_size;
				float yy2=b*general_object_size+general_object_size;

				/*make_particle(7,1,1000,xx1,yy1,0,0);
				make_particle(7,1,1000,xx2,yy1,0,0);
				make_particle(7,1,1000,xx2,yy2,0,0);
				make_particle(7,1,1000,xx1,yy2,0,0);*/

				if(lines_intersect(x1,y1,x2,y2,xx1,yy1,xx1,yy2,&hit.x,&hit.y,0.0001f)==1){
					//make_particle(6,1,1000,hit.x,hit.y,0,0);
					hits.push_back(hit);
				}
				if(lines_intersect(x1,y1,x2,y2,xx1,yy1,xx2,yy1,&hit.x,&hit.y,0.0001f)==1){
					//make_particle(6,1,1000,hit.x,hit.y,0,0);
					hits.push_back(hit);
				}
				if(lines_intersect(x1,y1,x2,y2,xx1,yy2,xx2,yy2,&hit.x,&hit.y,0.0001f)==1){
					//make_particle(6,1,1000,hit.x,hit.y,0,0);
					hits.push_back(hit);
				}
				if(lines_intersect(x1,y1,x2,y2,xx2,yy1,xx2,yy2,&hit.x,&hit.y,0.0001f)==1){
					//make_particle(6,1,1000,hit.x,hit.y,0,0);
					hits.push_back(hit);
				}

			}
		}
	}

	return hits;
}


vector <point2d> game_engine::line_collision_detection(float x1, float y1, float x2, float y2,map_object *object, bool return_on_first){
	vector <point2d> hits;

//	if(mod.general_objects[object->type].collision_type==-1)return false;
    if(mod.general_objects[object->type].collision_type==-1)
        return hits;

	switch(mod.general_objects[object->type].collision_type){

	//0=circle radius parameter0
	case 0:
		if(mod.general_objects[object->type].collision_parameter0>0){

			float object_size=mod.general_objects[object->type].collision_parameter0*object->size*general_object_size;
			float object_x=object->x+object_size*0.5f/mod.general_objects[object->type].collision_parameter0;
			float object_y=object->y+object_size*0.5f/mod.general_objects[object->type].collision_parameter0;

			hits=sphere_line_intersection (
				x1 , y1 ,
				x2 , y2 ,
				object_x , object_y ,
				object_size*0.5f*0.99f );

			return hits;
		}
		break;

	//1=polygon parameter0 from polygons.dat size parameter1
	case 1:
		float object_size=object->size*general_object_size;
		float object_x=object->x+object_size*0.5f;
		float object_y=object->y+object_size*0.5f;

		//all the lines
		int a;

		float sin=sincos.table_sin(object->rotation);
		float cos=sincos.table_cos(object->rotation);

		for(a=0;a<mod.polygons[mod.general_objects[object->type].collision_parameter0].points.size()-1;a++){
			float xx1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].x*object_size*mod.general_objects[object->type].collision_parameter1;
			float yy1=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a].y*object_size*mod.general_objects[object->type].collision_parameter1;
			float xx2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].x*object_size*mod.general_objects[object->type].collision_parameter1;
			float yy2=mod.polygons[mod.general_objects[object->type].collision_parameter0].points[a+1].y*object_size*mod.general_objects[object->type].collision_parameter1;

			float px1=cos*xx1+sin*yy1+object_x;
			float py1=sin*xx1-cos*yy1+object_y;
			float px2=cos*xx2+sin*yy2+object_x;
			float py2=sin*xx2-cos*yy2+object_y;

			point2d hit;
			if(lines_intersect(x1,y1,x2,y2,px1,py1,px2,py2,&hit.x,&hit.y,0.0001f)==1){
				hits.push_back(hit);
				if(return_on_first)
					return hits;
			}
		}

		break;
	}

	return hits;
}



void game_engine::start_map_editor(void){

	//load_game(0);////////////////////////////////////////////////testing!

	//load object infos + referred textures
	mod.mod_name=mod_names[selected_mod];
	load_mod(mod.mod_name);
	editor.start_editor(grim, &mod, &text_manager, &resources, screen_width, screen_height);

	game_state=9;
}

void game_engine::initialize_creature_specialties(creature_base *creature, map *map_to_edit, bool reset_bars){

	//make sure no bars exist
	if(reset_bars)
	for(unsigned int a=0;a<maximum_bars;a++){
		creature->bars[a].active=false;
	}
	//delete carried lights
	if(creature->carry_light>=0)
		delete_light(map_main,creature->carry_light);

	for(int specialty=0;specialty<mod.general_creatures[creature->type].specialties.size();specialty++){

		//add bars for creatures
		if(reset_bars)
		if(mod.general_creatures[creature->type].specialties[specialty].number==0){
			creature->bars[(int)mod.general_creatures[creature->type].specialties[specialty].parameter0].value=mod.general_creatures[creature->type].specialties[specialty].parameter3;
			creature->bars[(int)mod.general_creatures[creature->type].specialties[specialty].parameter0].minimum=mod.general_creatures[creature->type].specialties[specialty].parameter1;
			creature->bars[(int)mod.general_creatures[creature->type].specialties[specialty].parameter0].maximum=mod.general_creatures[creature->type].specialties[specialty].parameter2;
			creature->bars[(int)mod.general_creatures[creature->type].specialties[specialty].parameter0].active=true;
		}

		//add carried lights
		if(mod.general_creatures[creature->type].specialties[specialty].number==2){
			int type=mod.general_creatures[creature->type].specialties[specialty].parameter0;
			creature->carry_light=map_to_edit->create_light(creature->x,creature->y,type,mod.general_creatures[creature->type].specialties[specialty].parameter1,mod.general_lights[type].r,mod.general_lights[type].g,mod.general_lights[type].b,mod.general_lights[type].a,-1);
			creature->light_attached_to=mod.general_creatures[creature->type].specialties[specialty].parameter3;
			creature->carry_light_size=mod.general_creatures[creature->type].specialties[specialty].parameter1;
			creature->carry_light_type=type;
			carry_light(map_to_edit,creature,creature->carry_light);
		}

		//force AI
		if(mod.general_creatures[creature->type].specialties[specialty].number==3){
			creature->force_AI=true;
		}
	}
}


void game_engine::initialize_animation_frames(map *map_to_edit){

	//terrain
	debug.debug_output("Initialize terrain frames",1,0);
	for(int i=0;i<map_to_edit->sizex;i++){
		for(int j=0;j<map_to_edit->sizey;j++){

			//start with random frame
			map_to_edit->grid[i].grid[j].current_frame=randInt(0,mod.terrain_types[map_to_edit->grid[i].grid[j].terrain_type].terrain_frames.size());

			//set time to current plus random to next frame
			map_to_edit->grid[i].grid[j].frame_time=randDouble(0,mod.terrain_types[map_to_edit->grid[i].grid[j].terrain_type].terrain_frames[map_to_edit->grid[i].grid[j].current_frame].time);
		}
	}
	debug.debug_output("Initialize terrain frames",0,0);

	//props
	debug.debug_output("Initialize object frames",1,0);
	for(unsigned i=0;i<map_to_edit->object.size();i++){
		if(map_to_edit->object[i].dead)continue;

		/*itoa(map_to_edit->object[i].type,temprivi,10);
		debug.debug_output((string)"type="+temprivi,3,0);*/

		//start with random frame
		map_to_edit->object[i].current_animation_frame=randInt(0,mod.general_objects[map_to_edit->object[i].type].animation_frames.size());
		/*itoa(map_to_edit->object[i].current_animation_frame,temprivi,10);
		debug.debug_output((string)"frame="+temprivi,3,0);*/

		//set time to current plus random to next frame
		map_to_edit->object[i].animation_frame_time=randDouble(0,mod.general_objects[map_to_edit->object[i].type].animation_frames[map_to_edit->object[i].current_animation_frame].time);
		/*itoa((int)map_to_edit->object[i].animation_frame_time,temprivi,10);
		debug.debug_output((string)"time="+temprivi,3,0);*/

	}
	debug.debug_output("Initialize object frames",0,0);

	/*//plot_objects
	for(i=0;k<map_to_edit.items.size();k++){
		//it's a plot_object
		if(map_main->items[i].base_type==0){
		}
	}*/
}

void game_engine::find_suggested_camera_position(float *suggested_camera_x, float *suggested_camera_y){

	if(attach_camera_type==0){
		float camera_distance=attach_camera_parameter2;
		float size=mod.general_creatures[map_main->creature[attach_camera_parameter1].type].size*map_main->creature[attach_camera_parameter1].size*general_creature_size;
		*suggested_camera_x=map_main->creature[attach_camera_parameter1].x+size*0.5f-sincos.table_cos(map_main->creature[attach_camera_parameter1].rotation+pi/2)*camera_distance-screen_width/2.0f;
		*suggested_camera_y=map_main->creature[attach_camera_parameter1].y+size*0.5f-sincos.table_sin(map_main->creature[attach_camera_parameter1].rotation+pi/2)*camera_distance-screen_height/2.0f;
	}
	if(attach_camera_type==1){
		*suggested_camera_x=attach_camera_parameter1-screen_width/2.0f;
		*suggested_camera_y=attach_camera_parameter2-screen_height/2.0f;
	}
}

void game_engine::draw_map_grid_small(map *map_to_edit, int texture, int texture2){//renders map grid

	if(can_draw_map){
		if(grim->System_SetRenderTarget(texture)){

			grim->Quads_SetRotation(0);
			grim->Quads_SetSubset(0,0,1,1);
			const float darkness=1.0f;
			grim->Quads_SetColor(darkness,darkness,darkness,1);
			grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
			grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);

			float size_x=256.0f/(map_to_edit->sizex);
			float size_y=256.0f/(map_to_edit->sizey);

			grim->System_SetState_Blending(false);

			for(int i=0;i<map_to_edit->sizex;i++){
				for(int j=0;j<map_to_edit->sizey;j++){
					resources.Texture_Set(mod.terrain_types[map_to_edit->grid[i].grid[j].terrain_type].terrain_frames[0].texture);
					grim->Quads_Begin();
					grim->Quads_Draw(i*size_x, j*size_y, size_x, size_y);
					grim->Quads_End();
				}
			}

			//now copy map to second texture
			grim->System_SetRenderTarget(texture2);
			grim->Texture_Set(map_to_edit->map_texture);
			grim->Quads_Begin();
			grim->Quads_Draw(0, 0, 256.0f, 256.0f);
			grim->Quads_End();

			grim->System_SetRenderTarget(-1);
		}
	}


}


