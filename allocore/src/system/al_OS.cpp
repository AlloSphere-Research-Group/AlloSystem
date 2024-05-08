#include "allocore/system/al_Config.h"
#include "allocore/system/al_OS.hpp"

#ifdef AL_WINDOWS
#include <windows.h> // SetThreadExecutionState
#elif AL_OSX
#include <IOKit/pwr_mgt/IOPMLib.h>
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
	// Code valid for macOS 10.6+
	// https://developer.apple.com/library/archive/qa/qa1340/_index.html
	CFStringRef reason = CFSTR("Application requires display");
	static IOPMAssertionID ID = 0;
	if(whether && !ID){
		auto res = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &ID);
		//if(kIOReturnSuccess != res){}
		//printf("ID:%u\n", ID);
	} else if(ID){
		IOPMAssertionRelease(ID);
		ID = 0;
	}
#else

#endif
}

} // al::
