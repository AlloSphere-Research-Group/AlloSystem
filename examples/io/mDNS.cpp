/*
Allocore Example: Freenect

Description:
Demonstrating mDNS

Author:
Graham Wakefield, 2012
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_mDNS.hpp"

using namespace al;

int main(){
	printf("starting\n");
	
	// browse
	//launch_service_search(service_name ex "_osc._udp", callback);
	
	// stop browsing

	// discovery callback
	// receives list of services containing names / ip / ports
	
	// disconnect callback
	// receives ip / port / name of disconnected service
	
	// register
	// broadcast a service name / ip / port
	
	// stop broadcasting service
	

	MainLoop::start();
	return 0;
}
