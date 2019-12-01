#ifndef INCLUDE_UT_ALLOCORE_H
#define INCLUDE_UT_ALLOCORE_H

#undef NDEBUG
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "allocore/al_Allocore.hpp"

#ifdef AL_UT_NOPRINT	/* disable printing to stdout */
	#define UT_PRINTF	//
#else
	#define UT_PRINTF	printf
#endif

using namespace al;

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

SearchPaths& getSearchPaths();

bool almostEqual(float v1, float v2);

#endif
