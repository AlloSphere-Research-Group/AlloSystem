#ifndef INCLUDE_AL_MAIN_H
#define INCLUDE_AL_MAIN_H 1

#include <assert.h>
#include <stdio.h>

/* allocore */
#include "io/al_AudioIO.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_OSC.hpp"
#include "protocol/al_OSCAPR.hpp"
#include "protocol/al_Serialize.hpp"
#include "system/al_Time.h"
#include "system/al_Time.hpp"
#include "system/al_Thread.hpp"
#include "system/al_MainLoop.h"
#include "types/al_types.h"
#include "types/al_types.hpp"
#include "types/al_Vec.hpp"

#ifdef AL_UT_NOPRINT	/* disable printing to stdout */
	#define UT_PRINTF	//
#else
	#define UT_PRINTF	printf
#endif

/* Apache Portable Runtime */
#include "apr_general.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_thread_proc.h"
#include "apr_thread_mutex.h"

#endif /* include guard */
