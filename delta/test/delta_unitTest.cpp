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
int printer(al_sec t, char * args) {
	printf("tube msg: %s\n", args);
	return 0;
}

int audioprinter(al_sec t, char * args) {
	printf("pq msg of %d: %s\n", al_pq_used(al_pq_main()), args);
	
	// test: forward this stuff to the audio system:
	char * buf = al_tube_write_head_mem(delta_inbox());
	if (buf) {
		sprintf(buf, "%s", args); // or memcpy(buf, args, DELTA_TUBE_ARGS_SIZE);
		al_tube_write_send(delta_inbox(), t, printer);
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
	delta_audio_tick();
	
	// TODO: read delta outbusses into audio outbufs

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
	
	if (t > 3.) al_main_exit();
}

int main(int ac, char * av) {
	char * buf;
	int i;
	
	delta D = delta_main_init(44100.0, 0.03);
	
	/* queue up some messages: */
	for (i=0; i<10; i++) {
		buf = al_pq_msg(al_pq_main());
		if (buf) {
			sprintf(buf, "print %i\n", i);
			al_pq_sched(al_pq_main(), i, printer);
		}
		buf = al_pq_msg(al_pq_main());
		if (buf) {
			sprintf(buf, "audioprint %i\n", i);
			al_pq_sched(al_pq_main(), i, audioprinter);
		}
	}

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
	
	delta_main_quit();
	return 0;
	
pa_out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}