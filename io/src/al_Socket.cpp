#include "io/al_Socket.hpp"
#include "system/al_Config.h"

#ifdef AL_WIN32
	#include <Winsock2.h>
#else
	#include <unistd.h>
#endif

namespace al{

std::string getHostName(){
	const int len = 256;
	char name[len];						// host names are limited to 256 bytes
	if(gethostname(name, len) == 0){	// no error
		if(memchr(name, '\0', len)){	// name too long
			return name;			
		}
		
	}
	return "\0";						// there was an error...
}

} // al::
