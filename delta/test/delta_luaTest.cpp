#include "delta_lua.h"
#include "al_time.h"
#include "al_mainloop.h"

#include "portaudio.h"

#include <unistd.h>
#include <stdio.h>

PaError err;

/* audio thread entry point */
int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) 
{
	samplestamp offset;
	const float * inputs = (const float *)input;
	float * outputs = (float *)output;
	delta D = (delta)userData;
	const int inchans = delta_inchannels(D);
	const int outchans = delta_outchannels(D);
	const samplestamp blocksize = delta_blocksize(D);
	int ch, i;
	
	if ((samplestamp)frameCount % blocksize != 0) {
		printf("error: blocksize %d; expected a multiple of %d\n", frameCount, blocksize);
		return -1;
	}
	
	for (offset = 0; offset < frameCount; offset += blocksize) {
		/* 
			de-interleave audio inbufs and place into delta inbusses
		*/
		for (ch=0; ch<inchans; ch++) {
			sample * to = delta_audio_input(D, ch);
			for (i=0; i<blocksize; i++) {
				to[i] = inputs[offset + i*inchans];
			}
		}
		
		/*	
			Resume any scheduled events in the audio thread priority queue,
			+ Execute any processes in the proclist:
		*/
		delta_audio_tick();
		
		/*
			read delta outbusses into audio outbufs as interleaved
		*/
		for (ch=0; ch<outchans; ch++) {
			sample * from = delta_audio_output(D, ch);
			for (i=0; i<blocksize; i++) {
				outputs[offset + i*outchans] = from[i];
			}
		}
	}

	return 0;
}

/* main thread entry point */
void tick(al_nsec ns, void * u) {
	al_sec t = al_time_ns2s * ns;
	delta D = (delta)u;
	
	/* 
		Resume any scheduled events in the main thread priority queue:
			returns delta's main-thread logical time
	*/
	delta_main_tick();
	
	/*
		Handle latency difference here. 
		
		ratio of latency_actual / delta_latency() should be near to 1.
		it will gradually drift as audio/cpu clocks are not accurate;
			it may be necessary to have a gradual offset to account for this.
		if it starts growing excessively, the audio thread has probably stalled and we may want instead to switch to the cpu as the time source.
		
	*/
	//al_sec latency_actual = (t-td);
	//printf("@%06.3f	latency: %06.6f%\n", t, 100. * latency_actual / delta_latency(D));
	
	// quit after N seconds:
	//if (t > 3.) al_main_exit();
}


void script_preload(lua_State * L, const char * name, lua_CFunction func) {
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_settop(L, 0);
}
void script_load(lua_State * L, const char * name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_pushstring(L, name);
	if (lua_pcall(L, 1, 1, 0))
		printf("error: %s", lua_tostring(L, -1));
	lua_settop(L, 0);
}

void script_run(lua_State * L, char * file) {
	char path[256];
	getcwd(path, 255);
	char gocode [1024];
	sprintf(gocode, "require 'delta' \
		path = '%s/' \
		package.path = path .. '?.lua;' .. package.path \
		local f, err = loadfile(path .. '%s'); \
		if f then delta.go(f) else print(err) end ", path, file);
	if (luaL_dostring(L, gocode)) {
		printf("%s\n", lua_tostring(L, -1));
		return;
	}
}

int main(int ac, char * av) {

	lua_State * L = lua_open();
	luaL_openlibs(L);
	script_preload(L, "delta", luaopen_delta);
	//script_preload(L, "audio", luaopen_audio);
	script_run(L, "test_delta.lua");
	
	
	delta D = lua_getdelta(L);
	
	PaStream* stream;
	err = Pa_Initialize();
	if (err != paNoError) goto pa_out;
	err = Pa_OpenDefaultStream( &stream,
                              delta_inchannels(D),
                              delta_outchannels(D),
                              paFloat32,
                              delta_samplerate(D),
                              delta_blocksize(D),
                              callback,
                              D );
	if (err != paNoError) goto pa_out;
	err = Pa_StartStream(stream);
	if (err != paNoError) goto pa_out;

	al_main_enter(0.01, tick, D, NULL);
	
	
	err = Pa_StopStream(stream);
	if (err != paNoError) goto pa_out;
	err = Pa_CloseStream(stream);
	if (err != paNoError) goto pa_out;
	err = Pa_Terminate();
	if (err != paNoError) goto pa_out;
	
	lua_close(L);
	delta_close(&D);
	return 0;
	
pa_out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}