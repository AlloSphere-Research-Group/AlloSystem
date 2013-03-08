#include "allocore/system/al_Config.h"
#include "allocore/io/al_Serial.hpp"

//using namespace al;

#include "serial.cc"

#ifdef AL_WINDOWS
	#include "serial_win.cc"
#else
	#include "serial_unix.cc"
#endif
