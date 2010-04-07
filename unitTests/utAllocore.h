#ifndef INCLUDE_UT_ALLOCORE_H
#define INCLUDE_UT_ALLOCORE_H

#include <assert.h>
#include <stdio.h>

#include "io/al_AudioIO.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_OSC.hpp"
#include "protocol/al_OSCAPR.hpp"
#include "protocol/al_Serialize.hpp"
#include "system/al_Time.h"
#include "system/al_Time.hpp"
#include "system/al_Thread.hpp"
#include "types/al_types.h"
#include "types/al_types.hpp"
#include "types/al_Vec.hpp"

#ifdef AL_UT_NOPRINT	/* disable printing to stdout */
	#define UT_PRINTF	//
#else
	#define UT_PRINTF	printf
#endif

using namespace al;

int utIOAudioIO();
int utIOWindowGL();
int utProtocolOSC();
int utProtocolSerialize();
int utSystem();
int utTypes();

#endif
