#include "soundmanager.h"

bool SoundManager::Create(SoundSample** dest, char *filename)
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
    if ( Mix_OpenAudio(freq, AUDIO_S16SYS, channels, 1024) < 0 ) {
        return false;
    }

    Mix_AllocateChannels(numchannels);

    samplefreq = freq;
    //bytespersample = 2;
    bytespersample = 4; //for some reason

    return true;
}

void SoundManager::playMusic(char* filename){
    currentmusic = Mix_LoadMUS(filename);
    Mix_PlayMusic(currentmusic, -1);
}

void SoundManager::setMusicVolume(float volume){
    Mix_VolumeMusic(volume*MIX_MAX_VOLUME);
}

SoundSample::SoundSample(char *filename,int freq,int bytes){
    //TODO: the delay seems solvable by using VC++ compiler/libs and DirectSound
    chunk = Mix_LoadWAV(filename);
    samplefreq = freq;
    bytespersample = bytes;
}

bool SoundSample::initOkay(){
    return chunk;
}

float SoundSample::GetSoundLength() {
    return (float)(chunk->alen) / bytespersample / samplefreq * 1000;
}

void SoundSample::Play(float volume, float pan){
    Mix_VolumeChunk(chunk,volume*MIX_MAX_VOLUME);
    int channel = Mix_PlayChannel(-1, chunk,0);
    if(channel != -1){
        Mix_SetPanning(channel, 127 - (pan * 127) , 127 + (pan * 127));
    }

}
