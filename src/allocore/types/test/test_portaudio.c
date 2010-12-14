
#include "allocore/types/al_array.h"
#include "portaudio.h"

PaError err;

AlloArrayHeader arraytype;
AlloArray inputarray, outputarray;

int phase = 0;
double pincr = 0.06;

int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) 
{
	/* convert stream data to AlloArrayTy */
	inputarray.data.ptr = (void *)input;
	outputarray.data.ptr = output;
	
	/* do any processing here: */
	assert(frameCount == outputarray.header.dim[1]);
	
	char * outp = outputarray.data.ptr;
	int frame_stride = outputarray.header.stride[1];
	for (int i=0; i<outputarray.header.dim[1]; i++) {
		float * out = (float *)(outp + frame_stride * i);
		out[1] = sin(pincr * (phase+i));
		phase += 0.01;
	}
	
	phase += frameCount;

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
	
	/* configure array type for audio IO */
	arraytype.type = AlloFloat32Ty;
	/* note that portaudio by default gives us interleaved data */
	arraytype.components = 1; /* single sample units */
	arraytype.dimcount = 2; /* 1 dim */
	arraytype.dim[0] = 2; /* 2 channels */
	arraytype.dim[1] = 64; /* block size */
	arraytype.stride[0] = arraytype.components * allo_type_size(arraytype.type); 
	arraytype.stride[1] = arraytype.stride[0] * arraytype.dim[0]; 
	
	allo_array_setheader(&inputarray, &arraytype);
	allo_array_setheader(&outputarray, &arraytype);
	
	err = Pa_StartStream(stream);
	if (err != paNoError) goto out;
	
	Pa_Sleep(2000);
	
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