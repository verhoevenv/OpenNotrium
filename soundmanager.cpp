#include "soundmanager.h"

#include <SDL.h>
#include "physfsrwops.h"

bool SoundManager::Create(SoundSample** dest, const char *filename)
{
    *dest = new SoundSample(filename, samplefreq, bytespersample);
    if(!(*dest)->initOkay()){
        return false;
    }
    return true;
}

bool SoundManager::Initialize(int freq, int channels)
{
    if ( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 ) {
		return false;
	}
    //TODO: change parameters?
    //TODO: close audio
    //check http://www.koders.com/cpp/fidCCF9C0D295448E549DBADA9E72FF9A974D8ACBCD.aspx
    //for examples
    if ( Mix_OpenAudio(freq, AUDIO_S16SYS, channels, 1024) < 0 ) {
        return false;
    }

    Mix_AllocateChannels(numchannels);

    samplefreq = freq;
    //bytespersample = 2;
    bytespersample = 4; //for some reason

    return true;
}

void SoundManager::playMusic(const char* filename){
    currentmusic = Mix_LoadMUS_RW(PHYSFSRWOPS_openRead(filename), 1);
    Mix_PlayMusic(currentmusic, -1);
}

void SoundManager::setMusicVolume(float volume){
    Mix_VolumeMusic(volume*MIX_MAX_VOLUME);
}

SoundSample::SoundSample(const char *filename,int freq,int bytes){
    //TODO: the delay seems solvable by using VC++ compiler/libs and DirectSound
    chunk = Mix_LoadWAV_RW(PHYSFSRWOPS_openRead(filename), 1);
    samplefreq = freq;
    bytespersample = bytes;
}

bool SoundSample::initOkay(void){
    return chunk;
}

float SoundSample::GetSoundLength(void) {
    return (float)(chunk->alen) / bytespersample / samplefreq * 1000;
}

void SoundSample::Play(float volume, float pan){
    Mix_VolumeChunk(chunk,volume*MIX_MAX_VOLUME);
    int channel = Mix_PlayChannel(-1, chunk,0);
    if(channel == -1){
        Mix_AllocateChannels(8 + Mix_AllocateChannels(-1));
        channel = Mix_PlayChannel(-1, chunk,0);
    }
    if(channel != -1)
        Mix_SetPanning(channel, 127 - (pan * 127) , 127 + (pan * 127));
}
