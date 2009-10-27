#include "al_mainloop.h"
#include "al_time.h"
#include "delta.h"

#include "portaudio.h"
#include "stdio.h"

#define ddebug(...) 
//#define ddebug(...) printf(__VA_ARGS__)


/* 
	Simple demo functions to schedule:
*/
int printer(delta_sec t, char * args) {
	printf("tube msg: %s\n", args);
	return 0;
}

int audioprinter(delta_sec t, char * args) {
	printf("pq msg of %d: %s\n", delta_pq_used(delta_pq_main()), args);
	
	// test: forward this stuff to the audio system:
	char * buf = delta_tube_write_head_mem(delta_main_get()->inbox);
	if (buf) {
		sprintf(buf, "%s", args); // or memcpy(buf, args, DELTA_TUBE_ARGS_SIZE);
		delta_tube_write_send(delta_main_get()->inbox, t, printer);
	} else {
		return 1;
	}
	return 0;
}


PaError err;

/* audio thread entry point */
int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) 
{
	// TODO: convert audio inbufs and place into delta inbusses
	
	/*	
		Resume any scheduled events in the main thread priority queue,
		+ Execute any processes in the proclist:
	*/
	delta_audio_tick((delta_samplestamp)frameCount);
	
	// TODO: read delta outbusses into audio outbufs

	return 0;
}

/* main thread entry point */
void tick(al_nsec ns, void * u) {
	al_sec t = al_time_ns2s * ns;

	printf("@%06.3f\n", t);
	
	/* 
		Resume any scheduled events in the main thread priority queue:
	*/
	delta_main_tick();
	
	if (t > 3.) al_main_exit();
}

int main(int ac, char * av) {
	char * buf;
	int i;
	
	delta_main_init();
	
	/* queue up some messages: */
	for (i=0; i<10; i++) {
		buf = delta_pq_msg(delta_pq_main());
		if (buf) {
			sprintf(buf, "print %i\n", i);
			delta_pq_sched(delta_pq_main(), i, printer);
		}
		buf = delta_pq_msg(delta_pq_main());
		if (buf) {
			sprintf(buf, "audioprint %i\n", i);
			delta_pq_sched(delta_pq_main(), i, audioprinter);
		}
	}

	PaStream* stream;
	err = Pa_Initialize();
	if (err != paNoError) goto pa_out;
	err = Pa_OpenDefaultStream( &stream,
                              2,
                              2,
                              paFloat32,
                              44100.0,
                              64,
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
	
	delta_main_quit();
	return 0;
	
pa_out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}