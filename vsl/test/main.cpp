#include "vsl/vsl.h"
#include "system/al_mainloop.h"

#include "portaudio.h"
#include "stdio.h"
#include "unistd.h"

/*

workflow:

create a vsl (specify loader paths)
read in an init script
attach to audio process
trigger state in callback


*/

static vsl v = NULL;

char path[1024];
PaError err;
PaStream* stream;
double cpu;
int cpucount;



/* main thread entry point */
void tick(al_nsec ns, void * u) {
	al_sec t = al_time_ns2s * ns;

	//printf("@%06.3f\n", t);
	
//	/* 
//		Resume any scheduled events in the main thread priority queue:
//	*/
//	delta_main_tick();
	
	//if (t > 30.) al_main_exit();
}

void script_run(lua_State * L, char * file) {
	char code[1024];
	sprintf(code, "local f, err = loadfile(path .. '%s'); \
		if f then f() else print(err) end ", file);
	if (luaL_dostring(L, code)) {
		printf("%s\n", lua_tostring(L, -1));
		return;
	}
}

/* audio thread entry point */
int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) 
{
	const float * in = (const float *)input;
	float * out = (float *)output;
	
	if (v == NULL) {
		v = vsl_create(path);
		script_run(vsl_lua(v), "test_vsl.lua");
	}
	
	lua_State * L = vsl_lua(v);
	
	lua_pushinteger(L, frameCount);
	lua_setglobal(L, "blocksize");

	lua_getglobal(L, "tick");
	lua_call(L, 0, 0);
	
	lua_gc(L, LUA_GCSTEP, 10);
	lua_gc(L, LUA_GCSTOP, 0);
	
	lua_getglobal(L, "out");
	for (int i=0; i<frameCount; i++) {
		lua_rawgeti(L, -1, i+1);
		out[i*2+1] = out[i*2] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	
	cpucount++;
	cpu += Pa_GetStreamCpuLoad(stream);
	if (cpucount >= 10) {
		printf("cpu %07.3f\n", 10. * cpu);
		cpucount = 0;
		cpu = 0;
	}
	
	return 0;
}

int main(int ac, char ** av) {
	getcwd(path, 1024);
	
	err = Pa_Initialize();
	if (err != paNoError) goto pa_out;
	err = Pa_OpenDefaultStream( &stream,
                              2,
                              2,
                              paFloat32,
                              44100.0, //delta_samplerate(),
                              64, //delta_blocksize(),
                              callback,
                              NULL );
	if (err != paNoError) goto pa_out;
	err = Pa_StartStream(stream);
	if (err != paNoError) goto pa_out;

	al_main_enter(0.01, tick, NULL);
	
	
	err = Pa_StopStream(stream);
	if (err != paNoError) goto pa_out;
	err = Pa_CloseStream(stream);
	if (err != paNoError) goto pa_out;
	err = Pa_Terminate();
	if (err != paNoError) goto pa_out;

	vsl_destroy(&v);
	return 0;
	
pa_out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}