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

//std::string type("_http._tcp"); 
std::string type("_osc._udp");
//std::string type("_ssh._tcp");

// a Service can publish a service on this machine:
mdns::Service zservice("allocore", 4110, type);

// a Client can browse and report available services for a given service type:
mdns::Client z(type);

void tick(al_sec t) {
	z.poll();
	zservice.poll();

	MainLoop::queue().send(0.5, tick);
}

int main(){
	tick(0);
	
	MainLoop::start();
	return 0;
}
