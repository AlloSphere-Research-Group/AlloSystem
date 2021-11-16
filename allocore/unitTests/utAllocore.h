#ifndef INCLUDE_UT_ALLOCORE_H
#define INCLUDE_UT_ALLOCORE_H

#undef NDEBUG
#include <assert.h>
#include <cmath>

#ifdef AL_UT_NOPRINT	/* disable printing to stdout */
	#define UT_PRINTF	//
#else
	#define UT_PRINTF	printf
#endif

inline bool almostEqual(float v1, float v2) {
	return std::abs(v1 -v2) < 0.00001;
}

int utAudioScene();
int utIOAudioIO();
int utIOSocket();
int utIOWindowGL();
int utMath();
int utMathSpherical();
int utGraphicsDraw();
int utGraphicsMesh();
int utProtocolOSC();
int utSpatial();
int utSystem();
int utTypes();
int utTypesConversion();
int utThread();
int utFile();
int utAsset();
int utAmbisonics();

#endif
