#include "allocore/system/al_Config.h"

#if defined(AL_OSX)
	#include "hidapi/hid_mac.c"
#elif defined(AL_WINDOWS)
	#include "hidapi/hid_windows.c"
#elif defined(AL_LINUX)
	#include "hidapi/hid_linux.c"
#endif
