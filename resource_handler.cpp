#include "resource_handler.h"

using namespace std;
using namespace Debugger;

int resource_handler::load_texture(const string& name, const string& mod_name){//loads the texture if it's unique

	int i;

	if(name=="none")
		return -1;

	//find if we're already recorded this texture name
	for(i=0;i<textures.size();i++){
		if((textures[i].texture_name==name)&&(textures[i].mod_name==mod_name))
			return i;
	}


	std::string tempstring="Load Texture ";
	tempstring+=name;
	debug->debug_output(tempstring,Action::START,Logfile::STARTUP);

	//create new texture loading primitive
	texture_handling_primitive_base temp_load;
	temp_load.last_used=system_time;
	temp_load.mod_name=mod_name;
	temp_load.texture_handle_in_grim=-1;
	temp_load.texture_name=name;

	//check if such a file exists
	strcpy(temprivi,"textures/");
	strcat(temprivi,mod_name.c_str());
	strcat(temprivi,"/");
	strcat(temprivi,name.c_str());
	bool OK=grim->File_Exists(temprivi);

	//try default directory
	if(!OK){
		strcpy(temprivi,"textures/");
		strcat(temprivi,name.c_str());

		OK=grim->File_Exists(temprivi);
	}
	if(!OK){
		debug->debug_output("Loading Texture!",Action::FAIL_AND_END,Logfile::STARTUP);
		return -1;
	}



	debug->debug_output(tempstring, Action::END, Logfile::STARTUP);

	textures.push_back(temp_load);
	return textures.size()-1;



}


int resource_handler::load_sample(const string& name, int samples, const string& mod_name){//loads the sample if it's unique
    //TODO: use samples parameter if it is important

	if(name=="none.wav")return -1;
	if(name=="none")return -1;


	if(!play_sound)return -1;
	if(!sound_initialized)return -1;
	if(samples_loaded>=maximum_samples)return -1;

	int i;
	char temprivi[300];
	//find if the sample was already loaded
	for(i=0;i<samples_loaded;i++){
		//if(strcmpi(name,sample_name[i])==0){return i;}
		if((sample_name[i]==name)&&(sample_name_mod[i]==mod_name)){return i;}
	}

	//not loaded, load it
	{

		std::string tempstring="Load Sample ";
		tempstring+=name;
		debug->debug_output(tempstring,Action::START,Logfile::STARTUP);

		if(samples==-1)samples=2;

		bool res;
		//try mod directory
		strcpy(temprivi,"sound/");
		strcat(temprivi,mod_name.c_str());
		strcat(temprivi,"/");
		strcat(temprivi,name.c_str());
		res=g_pSoundManager->Create( &sample[samples_loaded], temprivi );



		//try default directory
		if(!res){
			SAFE_DELETE(sample[samples_loaded]);
			strcpy(temprivi,"sound/");
			strcat(temprivi,name.c_str());
			res=g_pSoundManager->Create( &sample[samples_loaded], temprivi );
		}

		if(res){
			//store name
			sample_name[samples_loaded]=name;
			sample_name_mod[samples_loaded]=mod_name;

			debug->debug_output(tempstring,Action::END, Logfile::STARTUP);

			samples_loaded++;
			return samples_loaded-1;
		}

		debug->debug_output("Loading Sample",Action::FAIL_AND_END,Logfile::STARTUP);
		return -1;
	}

}

void resource_handler::initialize_resource_handler(Engine *grim_engine, debugger *debugger, SoundManager* g_pSoundManager, bool play_sound, bool sound_initialized){
	debug=debugger;
	grim=grim_engine;
	this->g_pSoundManager=g_pSoundManager;
	this->play_sound=play_sound;
	this->sound_initialized=sound_initialized;
}

void resource_handler::uninitialize(void){
	for(int a=0;a<maximum_samples;a++){
		SAFE_DELETE(sample[a]);
	}
}

resource_handler::resource_handler(void){
	int a;

	textures.clear();
	textures_count=0;
	high_texture_count=0;

	//clear all samples
	for(a=0;a<maximum_samples;a++){
		sample[a]=NULL;
	}
	samples_loaded=0;
}

void resource_handler::Texture_Set(int number){


	if(number>textures.size())return;

	//first make sure the texture is loaded
	if(textures[number].texture_handle_in_grim<0){
		char temprivi2[1000];
		strcpy(temprivi2,textures[number].texture_name.c_str());
		textures[number].texture_handle_in_grim=load_texture_in_grim(temprivi2,textures[number].mod_name);
	}

	textures[number].last_used=system_time;
	grim->Texture_Set(textures[number].texture_handle_in_grim);
}

int resource_handler::load_texture_in_grim(const char *name, const string& mod_name){
	bool OK;

	textures_count++;


	//try mod directory
	strcpy(temprivi,"textures/");
	strcat(temprivi,mod_name.c_str());
	strcat(temprivi,"/");
	strcat(temprivi,name);
	OK=grim->Texture_Load(temprivi,temprivi );

	//try default directory
	if(!OK){
		strcpy(temprivi,"textures/");
		strcat(temprivi,name);

		OK=grim->Texture_Load(temprivi,temprivi );
	}

	int texture_number=grim->Texture_Get(temprivi);

	if(texture_number>high_texture_count)
		high_texture_count=texture_number;

	if(texture_number==-1){
		//we're still unable to load the texture, all slots must be full
		//release some slots
		unload_unneeded_textures(true);

		//int bug=1;
		//char* temp=grim->System_GetErrorMessage();
	}

	return texture_number;
}

void resource_handler::unload_unneeded_textures(bool unload_all){
	for(unsigned int a=0;a<textures.size();a++){
		texture_handling_primitive_base temp=textures[a];
		if(textures[a].texture_handle_in_grim>=0)
		if((system_time-textures[a].last_used>300)||(unload_all)){
			grim->Texture_Delete(textures[a].texture_handle_in_grim);
			textures[a].texture_handle_in_grim=-1;
		}
		//textures[a].texture_handle_in_grim=-1;
	}
	/*if(unload_all){
		textures.clear();
	}*/
}
