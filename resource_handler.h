#ifndef __RESOURCE_HANDLER_H__
#define __RESOURCE_HANDLER_H__

#pragma warning(disable: 4786)//disable warnings about long symbols

#include "engine.h"
#include "func.h"
//#include "..\grim_api\grim.h"
#include <string>
#include <vector>
//#include "DSUtil.h"
#include "soundmanager.h"

#define maximum_samples 200//how many samples can be loaded

class resource_handler;

class resource_handler
{
protected:
	struct texture_handling_primitive_base{
		std::string texture_name;
		std::string mod_name;
		int texture_handle_in_grim;
		float last_used;
	};
	debugger *debug;
	//string texture_name[1000];
	//int textures;//how many textures to load
	Engine *grim;
	char temprivi[1000];
	int high_texture_count;
	int textures_count;

	std::vector <texture_handling_primitive_base> textures;
	int load_texture_in_grim(const char* name, const std::string& mod_name);
	SoundManager* g_pSoundManager;
	std::string sample_name[maximum_samples];
	std::string sample_name_mod[maximum_samples];
	bool play_sound;
	bool sound_initialized;


public:
	int samples_loaded;
	SoundSample*        sample[maximum_samples];
	float system_time;

	int load_texture(const std::string& name, const std::string& mod_name);
	int load_sample(const std::string& name, int samples, const std::string& mod_name);//loads the sample if it's unique
	void unload_unneeded_textures(bool unload_all);

	void Texture_Set(int number);
	void initialize_resource_handler(Engine * engine, debugger *debugger, SoundManager* g_pSoundManager, bool play_sound, bool sound_initialized);
	void uninitialize(void);
	resource_handler(void);
};


#endif
