#ifndef INCLUDE_UT_ALLOCORE_H
#define INCLUDE_UT_ALLOCORE_H

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

int utIOAudioIO();
int utIOSocket();
int utIOWindowGL();
int utMath();
int utMathSpherical();
int utGraphicsDraw();
int utGraphicsMesh();
int utProtocolOSC();
int utProtocolSerialize();
int utSpatial();
int utSystem();
int utTypes();
int utTypesConversion();
int utThread();
int utFile();
int utAsset();

SearchPaths& getSearchPaths();

#endif
