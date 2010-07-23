#ifndef INCLUDE_UT_ALLOCORE_H
#define INCLUDE_UT_ALLOCORE_H

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "io/al_AudioIO.hpp"
#include "io/al_Socket.hpp"
#include "io/al_WindowGL.hpp"
#include "math/al_Complex.hpp"
#include "math/al_Functions.hpp"
#include "math/al_Generators.hpp"
#include "math/al_Interpolation.hpp"
#include "math/al_Plane.hpp"
#include "math/al_Random.hpp"
#include "math/al_Quat.hpp"
#include "math/al_Vec.hpp"
#include "protocol/al_Graphics.hpp"
#include "protocol/al_GraphicsBackendOpenGL.hpp"
#include "protocol/al_OSC.hpp"
#include "protocol/al_OSCAPR.hpp"
#include "protocol/al_Serialize.hpp"
#include "spatial/al_Camera.hpp"
#include "spatial/al_CoordinateFrame.hpp"
#include "system/al_Time.h"
#include "system/al_Time.hpp"
#include "system/al_Thread.hpp"
#include "types/al_Buffer.hpp"
#include "types/al_Conversion.hpp"
#include "types/al_types.h"
#include "types/al_types.hpp"


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
int utProtocolGraphics();
int utProtocolOSC();
int utProtocolSerialize();
int utSpatial();
int utSystem();
int utTypes();
int utTypesConversion();
int utThreadAPR();
int utFileAPR();

#endif
