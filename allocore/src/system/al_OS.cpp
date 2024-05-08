#include "allocore/system/al_Config.h"
#include "allocore/system/al_OS.hpp"

#ifdef AL_WINDOWS
#include <windows.h> // SetThreadExecutionState
#elif AL_OSX
#else
#endif

namespace al{

void requiresDisplay(bool whether, bool continuous){
#ifdef AL_WINDOWS
	if(whether){
		auto flags = ES_DISPLAY_REQUIRED;
		if(continuous) flags |= ES_CONTINUOUS;
		SetThreadExecutionState(flags);
	} else if(continuous){
		SetThreadExecutionState(ES_CONTINUOUS);
	}
#elif AL_OSX
    
#else

#endif
}

} // al::
