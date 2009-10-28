#include "delta_lua.h"
#include "al_time.h"
#include "al_mainloop.h"

#include "portaudio.h"

#include "unistd.h"
#include "stdio.h"

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
		delta_audio_tick(D);
		
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

	printf("@%06.3f\n", t);
	
	/* 
		Resume any scheduled events in the main thread priority queue:
	*/
	delta_main_tick(D);
	
	// quit after N seconds:
	if (t > 3.) al_main_exit();
}


void script_preload(lua_State * L, const char * name, lua_CFunction func) {
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
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
	
	delta D = delta_main_init(44100.0, 0.03);

	lua_State * L = lua_open();
	luaL_openlibs(L);
	script_preload(L, "audio", luaopen_audio);
	script_preload(L, "delta", luaopen_delta);
	script_run(L, "test_delta.lua");
	
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

	al_main_enter(0.01, tick, D);
	
	
	err = Pa_StopStream(stream);
	if (err != paNoError) goto pa_out;
	err = Pa_CloseStream(stream);
	if (err != paNoError) goto pa_out;
	err = Pa_Terminate();
	if (err != paNoError) goto pa_out;

//	// non-realtime example:
//	while (delta_main_now() < 3) {
//		delta_main_tick();
//		printf(" %5.2f\n", delta_main_now());
//		
//		// simulate audio thread (non-realtime)
//		delta_audio_tick(441);
//		
//		// simulate realtime
//		//al_sleep(0.01);
//	}
	
	lua_close(L);
	delta_main_quit(&D);
	return 0;
	
pa_out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}