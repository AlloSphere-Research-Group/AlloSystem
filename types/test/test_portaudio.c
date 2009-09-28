
#include "allo_types.h"
#include "portaudio.h"

PaError err;

AlloLatticeHeader latticetype;
AlloLattice inputlattice, outputlattice;

int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) 
{
	/* convert stream data to AlloLatticeTy */
	inputlattice.data.ptr = (void *)input;
	outputlattice.data.ptr = output;
	
	/* do any processing here: */

	return 0;
}

int main(int argc, char * argv) {

	PaStream* stream;
	
	err = Pa_Initialize();
	if (err != paNoError) goto out;
	err = Pa_OpenDefaultStream( &stream,
                              2,
                              2,
                              paFloat32,
                              44100.0,
                              64,
                              callback,
                              NULL );
	if (err != paNoError) goto out;
	
	/* configure lattice type for audio IO */
	latticetype.type = AlloFloat32Ty;
	/* note that portaudio by default gives us interleaved data */
	latticetype.components = 2; /* 2 channels */
	latticetype.dimcount = 1; /* 1 dim */
	latticetype.dim[0] = 64; /* block size */
	latticetype.stride[0] = latticetype.components * allo_type_size(latticetype.type); 
	
	allo_lattice_setheader(&inputlattice, &latticetype);
	allo_lattice_setheader(&outputlattice, &latticetype);
	
	err = Pa_StartStream(stream);
	if (err != paNoError) goto out;
	
	Pa_Sleep(1000);
	
	err = Pa_StopStream(stream);
	if (err != paNoError) goto out;
	err = Pa_CloseStream(stream);
	if (err != paNoError) goto out;
	err = Pa_Terminate();
	if (err != paNoError) goto out;
	return 0;
out:
	printf("PaError %s\n", Pa_GetErrorText(err));
	return -1;
}