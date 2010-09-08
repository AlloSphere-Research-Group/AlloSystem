#include "utAllocore.h"

int main(){

	utMath();
	utTypes();
	utTypesConversion();
//	utSpatial();
	utSystem();
//	utProtocolOSC();
	utProtocolSerialize();

//	utIOAudioIO();
//	utIOWindowGL();
	utIOSocket();
	utFile();
//	utThread();

//	utProtocolGraphics();
	
	int size = 14;
	for (int z=-64; z<64; z++) {
		printf("%d %d %d\n", z, (z & size), !!(z & size));
	}

	return 0;
}


